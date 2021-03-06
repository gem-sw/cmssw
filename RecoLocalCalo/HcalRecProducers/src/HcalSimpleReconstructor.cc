#include "HcalSimpleReconstructor.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/Common/interface/EDCollection.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>
    
HcalSimpleReconstructor::HcalSimpleReconstructor(edm::ParameterSet const& conf):
  reco_(conf.getParameter<bool>("correctForTimeslew"),
	conf.getParameter<bool>("correctForPhaseContainment"),conf.getParameter<double>("correctionPhaseNS")),
  det_(DetId::Hcal),
  inputLabel_(conf.getParameter<edm::InputTag>("digiLabel")),
  dropZSmarkedPassed_(conf.getParameter<bool>("dropZSmarkedPassed")),
  firstSample_(conf.getParameter<int>("firstSample")),
  samplesToAdd_(conf.getParameter<int>("samplesToAdd")),
  tsFromDB_(conf.getParameter<bool>("tsFromDB")),
  firstDepthWeight_(1.),
  upgradeHBHE_(false),
  upgradeHF_(false),
  paramTS(0),
  theTopology(0), 
  puCorrMethod_(conf.existsAs<int>("puCorrMethod") ? conf.getParameter<int>("puCorrMethod") : 0)
{
  
  

  if ( conf.exists("firstDepthWeight") ) {
    firstDepthWeight_ = conf.getParameter<double>("firstDepthWeight");
  }
  reco_.setD1W(firstDepthWeight_); // set depth1=layer0 weight in any case...

  std::string subd=conf.getParameter<std::string>("Subdetector");
  if(!strcasecmp(subd.c_str(),"upgradeHBHE")) {
     upgradeHBHE_ = true;
     produces<HBHERecHitCollection>();
  }
  else if (!strcasecmp(subd.c_str(),"upgradeHF")) {
     upgradeHF_ = true;
     produces<HFRecHitCollection>();
  }
  else if (!strcasecmp(subd.c_str(),"HO")) {
    subdet_=HcalOuter;
    produces<HORecHitCollection>();
  }  
  else if (!strcasecmp(subd.c_str(),"HBHE")) {
    if( !upgradeHBHE_) {
      subdet_=HcalBarrel;
      produces<HBHERecHitCollection>();
    }
  } 
  else if (!strcasecmp(subd.c_str(),"HF")) {
    if( !upgradeHF_) {
    subdet_=HcalForward;
    produces<HFRecHitCollection>();
    }
  } 
  else {
    std::cout << "HcalSimpleReconstructor is not associated with a specific subdetector!" << std::endl;
  } 
  
  reco_.setpuCorrMethod(puCorrMethod_);
  if(puCorrMethod_ == 2) { 
    reco_.setpuCorrParams(
              conf.getParameter<bool>  ("applyPedConstraint"),
              conf.getParameter<bool>  ("applyTimeConstraint"),
              conf.getParameter<bool>  ("applyPulseJitter"),
              conf.getParameter<bool>  ("applyUnconstrainedFit"),
              conf.getParameter<bool>  ("applyTimeSlew"),
              conf.getParameter<double>("ts4Min"),
              conf.getParameter<double>("ts4Max"),
              conf.getParameter<double>("pulseJitter"),
              conf.getParameter<double>("meanTime"),
              conf.getParameter<double>("timeSigma"),
              conf.getParameter<double>("meanPed"),
              conf.getParameter<double>("pedSigma"),
              conf.getParameter<double>("noise"),
              conf.getParameter<double>("timeMin"),
              conf.getParameter<double>("timeMax"),
              conf.getParameter<double>("ts3chi2"),
              conf.getParameter<double>("ts4chi2"),
              conf.getParameter<double>("ts345chi2"),
              conf.getParameter<double>("chargeMax"), //For the unconstrained Fit
              conf.getParameter<int>   ("fitTimes")
              );
  }
  
}

HcalSimpleReconstructor::~HcalSimpleReconstructor() { 
  delete paramTS;
  delete theTopology;
}

void HcalSimpleReconstructor::beginRun(edm::Run const&r, edm::EventSetup const & es){
  if(tsFromDB_) {
    edm::ESHandle<HcalRecoParams> p;
    es.get<HcalRecoParamsRcd>().get(p);
    paramTS = new HcalRecoParams(*p.product());

    edm::ESHandle<HcalTopology> htopo;
    es.get<HcalRecNumberingRecord>().get(htopo);
    theTopology=new HcalTopology(*htopo);
    paramTS->setTopo(theTopology);

  }
  reco_.beginRun(es);
}

void HcalSimpleReconstructor::endRun(edm::Run const&r, edm::EventSetup const & es){
  if(tsFromDB_ && paramTS) {
    delete paramTS;
    paramTS = 0;
    reco_.endRun();
  }
}


template<class DIGICOLL, class RECHITCOLL> 
void HcalSimpleReconstructor::process(edm::Event& e, const edm::EventSetup& eventSetup)
{

  // get conditions
  edm::ESHandle<HcalDbService> conditions;
  eventSetup.get<HcalDbRecord>().get(conditions);

  edm::Handle<DIGICOLL> digi;
  e.getByLabel(inputLabel_,digi);

  // create empty output
  std::auto_ptr<RECHITCOLL> rec(new RECHITCOLL);
  rec->reserve(digi->size());
  // run the algorithm
  int first = firstSample_;
  int toadd = samplesToAdd_;
  typename DIGICOLL::const_iterator i;
  for (i=digi->begin(); i!=digi->end(); i++) {
    HcalDetId cell = i->id();
    DetId detcell=(DetId)cell;
    // rof 27.03.09: drop ZS marked and passed digis:
    if (dropZSmarkedPassed_)
      if (i->zsMarkAndPass()) continue;

    const HcalCalibrations& calibrations=conditions->getHcalCalibrations(cell);
    const HcalQIECoder* channelCoder = conditions->getHcalCoder (cell);
    const HcalQIEShape* shape = conditions->getHcalShape (channelCoder); 
    HcalCoderDb coder (*channelCoder, *shape);

    //>>> firstSample & samplesToAdd
    if(tsFromDB_) {
      const HcalRecoParam* param_ts = paramTS->getValues(detcell.rawId());
      first = param_ts->firstSample();
      toadd = param_ts->samplesToAdd();
    }
    rec->push_back(reco_.reconstruct(*i,first,toadd,coder,calibrations));   
  }
  // return result
  e.put(rec);
}


void HcalSimpleReconstructor::processUpgrade(edm::Event& e, const edm::EventSetup& eventSetup)
{
  // get conditions
  edm::ESHandle<HcalDbService> conditions;
  eventSetup.get<HcalDbRecord>().get(conditions);

  if(upgradeHBHE_){

    edm::Handle<HBHEUpgradeDigiCollection> digi;
    e.getByLabel(inputLabel_, digi);

    // create empty output
    std::auto_ptr<HBHERecHitCollection> rec(new HBHERecHitCollection);
    rec->reserve(digi->size()); 

    // run the algorithm
    int first = firstSample_;
    int toadd = samplesToAdd_;
    HBHEUpgradeDigiCollection::const_iterator i;
    for (i=digi->begin(); i!=digi->end(); i++) {
      HcalDetId cell = i->id();
      DetId detcell=(DetId)cell;
      // rof 27.03.09: drop ZS marked and passed digis:
      if (dropZSmarkedPassed_)
      if (i->zsMarkAndPass()) continue;
      
      const HcalCalibrations& calibrations=conditions->getHcalCalibrations(cell);
      const HcalQIECoder* channelCoder = conditions->getHcalCoder (cell);
      const HcalQIEShape* shape = conditions->getHcalShape (channelCoder); 
      HcalCoderDb coder (*channelCoder, *shape);

      //>>> firstSample & samplesToAdd
      if(tsFromDB_) {
	const HcalRecoParam* param_ts = paramTS->getValues(detcell.rawId());
	first = param_ts->firstSample();
	toadd = param_ts->samplesToAdd();
      }
      rec->push_back(reco_.reconstructHBHEUpgrade(*i,first,toadd,coder,calibrations));

    }

    e.put(rec); // put results
  }// End of upgradeHBHE

  if(upgradeHF_){

    edm::Handle<HFUpgradeDigiCollection> digi;
    e.getByLabel(inputLabel_, digi);

    // create empty output
    std::auto_ptr<HFRecHitCollection> rec(new HFRecHitCollection);
    rec->reserve(digi->size()); 

    // run the algorithm
    int first = firstSample_;
    int toadd = samplesToAdd_;
    HFUpgradeDigiCollection::const_iterator i;
    for (i=digi->begin(); i!=digi->end(); i++) {
      HcalDetId cell = i->id();
      DetId detcell=(DetId)cell;
      // rof 27.03.09: drop ZS marked and passed digis:
      if (dropZSmarkedPassed_)
      if (i->zsMarkAndPass()) continue;
      
      const HcalCalibrations& calibrations=conditions->getHcalCalibrations(cell);
      const HcalQIECoder* channelCoder = conditions->getHcalCoder (cell);
      const HcalQIEShape* shape = conditions->getHcalShape (channelCoder); 
      HcalCoderDb coder (*channelCoder, *shape);

      //>>> firstSample & samplesToAdd
      if(tsFromDB_) {
	const HcalRecoParam* param_ts = paramTS->getValues(detcell.rawId());
	first = param_ts->firstSample();
	toadd = param_ts->samplesToAdd();
      }
      rec->push_back(reco_.reconstructHFUpgrade(*i,first,toadd,coder,calibrations));

    }  
    e.put(rec); // put results
  }// End of upgradeHF

}



void HcalSimpleReconstructor::produce(edm::Event& e, const edm::EventSetup& eventSetup)
{
  // HACK related to HB- corrections
  if(e.isRealData()) reco_.setForData();
 
  // What to produce, better to avoid the same subdet Upgrade and regular 
  // rechits "clashes"
  if(upgradeHBHE_ || upgradeHF_) {
      processUpgrade(e, eventSetup);
  } else if (det_==DetId::Hcal) {
    if ((subdet_==HcalBarrel || subdet_==HcalEndcap) && !upgradeHBHE_) {
      process<HBHEDigiCollection, HBHERecHitCollection>(e, eventSetup);
    } else if (subdet_==HcalForward && !upgradeHF_) {
      process<HFDigiCollection, HFRecHitCollection>(e, eventSetup);
    } else if (subdet_==HcalOuter) {
      process<HODigiCollection, HORecHitCollection>(e, eventSetup);
    } else if (subdet_==HcalOther && subdetOther_==HcalCalibration) {
      process<HcalCalibDigiCollection, HcalCalibRecHitCollection>(e, eventSetup);
    }
  }

  //
  // Print a message stating how many fit errors occurred (rather than printing each error individually)
  //
  auto errorFrequency=reco_.fitErrorCodeFrequency();
  if( !errorFrequency.empty() )
  {
    std::string message="Fit errors produced:\n";
    for( const auto& codeFrequencyPair : errorFrequency )
    {
      if( codeFrequencyPair.first==0 ) message+="\t"+std::to_string(codeFrequencyPair.second)+ " successful fits.\n";
      else message+="\tError "+std::to_string(codeFrequencyPair.first)+" occurred "+std::to_string(codeFrequencyPair.second)+ " times.\n";
    }
    edm::LogWarning( "HcalSimpleReconstructor" ) << message;
  }
  reco_.resetFitErrorFrequency(); // Now that data has been output, reset ready for the next event.
}
