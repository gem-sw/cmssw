#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCEEDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCHEDetId.h"
#include "SimCalorimetry/HGCSimProducers/interface/HGCDigitizer.h"
#include "SimGeneral/MixingModule/interface/PileUpEventPrincipal.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CrossingFrame/interface/CrossingFrame.h"
#include "SimDataFormats/CrossingFrame/interface/MixCollection.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"

#include <algorithm>
#include <boost/foreach.hpp>

//
HGCDigitizer::HGCDigitizer(const edm::ParameterSet& ps) :
  checkValidDetIds_(true),
  simHitAccumulator_( new HGCSimHitDataAccumulator ),
  mySubDet_(ForwardSubdetector::ForwardEmpty),
  refSpeed_(0.1*CLHEP::c_light) //[CLHEP::c_light]=mm/ns convert to cm/ns
{
  //configure from cfg
  hitCollection_     = ps.getParameter< std::string >("hitCollection");
  digiCollection_    = ps.getParameter< std::string >("digiCollection");
  maxSimHitsAccTime_ = ps.getParameter< uint32_t >("maxSimHitsAccTime");
  bxTime_            = ps.getParameter< double >("bxTime");
  digitizationType_  = ps.getParameter< uint32_t >("digitizationType");
  useAllChannels_    = ps.getParameter< bool >("useAllChannels");
  verbosity_         = ps.getUntrackedParameter< uint32_t >("verbosity",0);
  tofDelay_          = ps.getParameter< double >("tofDelay");  

  //get the random number generator
  edm::Service<edm::RandomNumberGenerator> rng;
  if ( ! rng.isAvailable()) 
    {
      throw cms::Exception("Configuration") << "HGCDigitizer requires the RandomNumberGeneratorService - please add this service or remove the modules that require it";
    }
  
  CLHEP::HepRandomEngine& engine = rng->getEngine();
  if(hitCollection_.find("HitsEE")!=std::string::npos) { 
    mySubDet_=ForwardSubdetector::HGCEE;  
    theHGCEEDigitizer_=std::unique_ptr<HGCEEDigitizer>(new HGCEEDigitizer(ps) ); 
    theHGCEEDigitizer_->setRandomNumberEngine(engine);
  }
  if(hitCollection_.find("HitsHEfront")!=std::string::npos)  
    { 
      mySubDet_=ForwardSubdetector::HGCHEF;
      theHGCHEfrontDigitizer_=std::unique_ptr<HGCHEfrontDigitizer>(new HGCHEfrontDigitizer(ps) );
      theHGCHEfrontDigitizer_->setRandomNumberEngine(engine);
    }
  if(hitCollection_.find("HitsHEback")!=std::string::npos)
    { 
      mySubDet_=ForwardSubdetector::HGCHEB;
      theHGCHEbackDigitizer_=std::unique_ptr<HGCHEbackDigitizer>(new HGCHEbackDigitizer(ps) );
      theHGCHEbackDigitizer_->setRandomNumberEngine(engine);
    }
}

//
void HGCDigitizer::initializeEvent(edm::Event const& e, edm::EventSetup const& es) 
{
  resetSimHitDataAccumulator(); 
}

//
void HGCDigitizer::finalizeEvent(edm::Event& e, edm::EventSetup const& es)
{
  if( producesEEDigis() ) 
    {
      std::auto_ptr<HGCEEDigiCollection> digiResult(new HGCEEDigiCollection() );
      theHGCEEDigitizer_->run(digiResult,*simHitAccumulator_,digitizationType_);
      edm::LogInfo("HGCDigitizer") << " @ finalize event - produced " << digiResult->size() <<  " EE hits";
      e.put(digiResult,digiCollection());
    }
  if( producesHEfrontDigis())
    {
      std::auto_ptr<HGCHEDigiCollection> digiResult(new HGCHEDigiCollection() );
      theHGCHEfrontDigitizer_->run(digiResult,*simHitAccumulator_,digitizationType_);
      edm::LogInfo("HGCDigitizer") << " @ finalize event - produced " << digiResult->size() <<  " HE front hits";
      e.put(digiResult,digiCollection());
    }
  if( producesHEbackDigis() )
    {
      std::auto_ptr<HGCHEDigiCollection> digiResult(new HGCHEDigiCollection() );
      theHGCHEbackDigitizer_->run(digiResult,*simHitAccumulator_,digitizationType_);
      edm::LogInfo("HGCDigitizer") << " @ finalize event - produced " << digiResult->size() <<  " HE back hits";
      e.put(digiResult,digiCollection());
    }
}

//
void HGCDigitizer::accumulate(edm::Event const& e, edm::EventSetup const& eventSetup) {

  //get inputs
  edm::Handle<edm::PCaloHitContainer> hits;
  e.getByLabel(edm::InputTag("g4SimHits",hitCollection_),hits); 
  if( !hits.isValid() ){
    edm::LogError("HGCDigitizer") << " @ accumulate : can't find " << hitCollection_ << " collection of g4SimHits";
    return;
  }

  //get geometry
  edm::ESHandle<HGCalGeometry> geom;
  if( producesEEDigis() )      eventSetup.get<IdealGeometryRecord>().get("HGCalEESensitive"            , geom);
  if( producesHEfrontDigis() ) eventSetup.get<IdealGeometryRecord>().get("HGCalHESiliconSensitive"     , geom);
  if( producesHEbackDigis() )  eventSetup.get<IdealGeometryRecord>().get("HGCalHEScintillatorSensitive", geom);

  //accumulate in-time the main event
  accumulate(hits, 0, geom);
}

//
void HGCDigitizer::accumulate(PileUpEventPrincipal const& e, edm::EventSetup const& eventSetup) {

  //get inputs
  edm::Handle<edm::PCaloHitContainer> hits;
  e.getByLabel(edm::InputTag("g4SimHits",hitCollection_),hits); 
  if( !hits.isValid() ){
    edm::LogError("HGCDigitizer") << " @ accumulate : can't find " << hitCollection_ << " collection of g4SimHits";
    return;
  }

  //get geometry
  edm::ESHandle<HGCalGeometry> geom;
  if( producesEEDigis() )      eventSetup.get<IdealGeometryRecord>().get("HGCalEESensitive"            , geom);
  if( producesHEfrontDigis() ) eventSetup.get<IdealGeometryRecord>().get("HGCalHESiliconSensitive"     , geom);
  if( producesHEbackDigis() )  eventSetup.get<IdealGeometryRecord>().get("HGCalHEScintillatorSensitive", geom);

  //accumulate for the simulated bunch crossing  
  accumulate(hits, e.bunchCrossing(), geom);
}

//
void HGCDigitizer::accumulate(edm::Handle<edm::PCaloHitContainer> const &hits, 
			      int bxCrossing,
			      const edm::ESHandle<HGCalGeometry> &geom)
{
  if(!geom.isValid()) return;
  const HGCalTopology &topo=geom->topology();
  const HGCalDDDConstants &dddConst=topo.dddConstants();

  //base time samples for each DetId, initialized to 0
  std::array<HGCSimHitData,2> baseData;
  baseData[0].fill(0.); //accumulated energy
  baseData[1].fill(0.); //time-of-flight
  
  //configuration to apply for the computation of time-of-flight
  bool weightToAbyEnergy(false);
  float tdcOnset(0),keV2fC(0.0);
  if( mySubDet_==ForwardSubdetector::HGCEE )
    {
      weightToAbyEnergy = theHGCEEDigitizer_->toaModeByEnergy();
      tdcOnset          = theHGCEEDigitizer_->tdcOnset();
      keV2fC            = theHGCEEDigitizer_->keV2fC();
    }
  else if( mySubDet_==ForwardSubdetector::HGCHEF )
    {
      weightToAbyEnergy = theHGCHEfrontDigitizer_->toaModeByEnergy();
      tdcOnset          = theHGCHEfrontDigitizer_->tdcOnset();
      keV2fC            = theHGCHEfrontDigitizer_->keV2fC();
    }

  //create list of tuples (pos in container, RECO DetId, time) to be sorted first
  int nchits=(int)hits->size();  
  std::vector< HGCCaloHitTuple_t > hitRefs(nchits);
  for(int i=0; i<nchits; i++)
    {
      HGCalDetId simId( hits->at(i).id() );

      //skip this hit if after ganging it is not valid
      int layer(simId.layer()), cell(simId.cell());
      std::pair<int,int> recoLayerCell=dddConst.simToReco(cell,layer,topo.detectorType());
      cell  = recoLayerCell.first;
      layer = recoLayerCell.second;
      if(layer<0 || cell<0) 
	{
	  hitRefs[i]=std::make_tuple( i, 0, 0.);
	  continue;
	}

      //assign the RECO DetId
      DetId id( producesEEDigis() ?
		(uint32_t)HGCEEDetId(mySubDet_,simId.zside(),layer,simId.sector(),simId.subsector(),cell):
		(uint32_t)HGCHEDetId(mySubDet_,simId.zside(),layer,simId.sector(),simId.subsector(),cell) );

      if (verbosity_>0) {
	if (producesEEDigis())
	  std::cout << "HGCDigitizer: i/p " << simId << " o/p " << HGCEEDetId(id) << std::endl;
	else
	  std::cout << "HGCDigitizer: i/p " << simId << " o/p " << HGCHEDetId(id) << std::endl;
      }

      hitRefs[i]=std::make_tuple( i, 
				  id.rawId(), 
				  (float)hits->at(i).time() );
    }
  std::sort(hitRefs.begin(),hitRefs.end(),this->orderByDetIdThenTime);
  
  //loop over sorted hits
  for(int i=0; i<nchits; i++)
    {
      int hitidx   = std::get<0>(hitRefs[i]);
      uint32_t id  = std::get<1>(hitRefs[i]);
      if(id==0) continue; // to be ignored at RECO level

      float toa    = std::get<2>(hitRefs[i]);
      const PCaloHit &hit=hits->at( hitidx );     
      float charge = hit.energy()*1e6*keV2fC;


      //distance to the center of the detector
      float dist2center( geom->getPosition(id).mag() );
      
      //hit time: [time()]=ns  [centerDist]=cm [refSpeed_]=cm/ns + delay by 1ns
      //accumulate in 15 buckets of 25ns (9 pre-samples, 1 in-time, 5 post-samples)
      float tof(toa-dist2center/refSpeed_+tofDelay_);
      int itime=floor( tof/bxTime_ ) ;
      
      //no need to add bx crossing - tof comes already corrected from the mixing module
      //itime += bxCrossing;
      itime += 9;
      
      if(itime<0 || itime>14) continue; 
            
      //check if already existing (perhaps could remove this in the future - 2nd event should have all defined)
      HGCSimHitDataAccumulator::iterator simHitIt=simHitAccumulator_->find(id);
      if(simHitIt==simHitAccumulator_->end())
	{
	  simHitAccumulator_->insert( std::make_pair(id,baseData) );
	  simHitIt=simHitAccumulator_->find(id);
	}
      
      //check if time index is ok and store energy
      if(itime >= (int)simHitIt->second[0].size() ) continue;

      (simHitIt->second)[0][itime] += charge;
      float accCharge=(simHitIt->second)[0][itime];

      /*
      std::cout << id 
		<< " " << accCharge
		<< " " << charge
		<< " " << itime 
		<< " " << tof;
      */

      //time-of-arrival (check how to be used)
      if(weightToAbyEnergy) (simHitIt->second)[1][itime] += charge*tof;
      else if((simHitIt->second)[1][itime]==0)
	{	
	  if( accCharge>tdcOnset)
	    {
	      //extrapolate linear using previous simhit if it concerns to the same DetId
	      float fireTDC=tof;
	      if(i>0)
		{
		  uint32_t prev_id  = std::get<1>(hitRefs[i-1]);
		  if(prev_id==id)
		    {
		      float prev_toa    = std::get<2>(hitRefs[i-1]);
		      float prev_tof(prev_toa-dist2center/refSpeed_+tofDelay_);
		      //float prev_charge = std::get<3>(hitRefs[i-1]);
		      float deltaQ2TDCOnset = tdcOnset-((simHitIt->second)[0][itime]-charge);
		      float deltaQ          = charge;
		      float deltaT          = (tof-prev_tof);
		      fireTDC               = deltaT*(deltaQ2TDCOnset/deltaQ)+prev_tof;
		    }		  
		}

	      (simHitIt->second)[1][itime]=fireTDC;
	    }
	}

      //std::cout << " " << (simHitIt->second)[1][itime] << std::endl;
    }
  hitRefs.clear();
  
  //add base data for noise simulation
  if(!checkValidDetIds_) return;
  if(!geom.isValid()) return;
  const std::vector<DetId> &validIds=geom->getValidDetIds();   
  int nadded(0);
  if (useAllChannels_) {
    for(std::vector<DetId>::const_iterator it=validIds.begin(); it!=validIds.end(); it++) {
      uint32_t id(it->rawId());
      if(simHitAccumulator_->find(id)!=simHitAccumulator_->end()) continue;
      simHitAccumulator_->insert( std::make_pair(id,baseData) );
      nadded++;
    }
  }
  
  if (verbosity_ > 0) 
    std::cout << "HGCDigitizer:Added " << nadded << ":" << validIds.size() 
	      << " detIds without " << hitCollection_ 
	      << " in first event processed" << std::endl;
  checkValidDetIds_=false;
}

//
void HGCDigitizer::beginRun(const edm::EventSetup & es)
{
}

//
void HGCDigitizer::endRun()
{
}

//
void HGCDigitizer::resetSimHitDataAccumulator()
{
  for( HGCSimHitDataAccumulator::iterator it = simHitAccumulator_->begin(); it!=simHitAccumulator_->end(); it++)
    {
      it->second[0].fill(0.);
      it->second[1].fill(0.);
    }
}





