// -*- C++ -*-
//
// Package:    TrackAssociator
// Class:      TrackDetectorAssociator
// 
/*

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Dmytro Kovalskyi
//         Created:  Fri Apr 21 10:59:41 PDT 2006
//
//

#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "TrackingTools/TrackAssociator/interface/TrackDetectorAssociator.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/OrphanHandle.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackExtra.h"
#include "DataFormats/CaloTowers/interface/CaloTower.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/DTRecHit/interface/DTRecHitCollection.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/DetId/interface/DetId.h"

// calorimeter info
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCChamberSpecs.h"

#include "DataFormats/GeometrySurface/interface/Cylinder.h"
#include "DataFormats/GeometrySurface/interface/Plane.h"

#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"


#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"
#include <stack>
#include <set>

#include "DataFormats/Math/interface/LorentzVector.h"
#include "Math/VectorUtil.h"
#include <algorithm>

#include "TrackingTools/TrackAssociator/interface/DetIdAssociator.h"
#include "TrackingTools/TrackAssociator/interface/DetIdInfo.h"
// #include "TrackingTools/TrackAssociator/interface/CaloDetIdAssociator.h"
// #include "TrackingTools/TrackAssociator/interface/EcalDetIdAssociator.h"
// #include "TrackingTools/TrackAssociator/interface/PreshowerDetIdAssociator.h"

#include "DataFormats/DTRecHit/interface/DTRecSegment4DCollection.h"
#include "DataFormats/DTRecHit/interface/DTRecSegment2D.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/ErrorFrameTransformer.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"

#include "TrackingTools/Records/interface/DetIdAssociatorRecord.h"

//temp
#include <Geometry/CommonTopologies/interface/TrapezoidalStripTopology.h>
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "TrackingTools/AnalyticalJacobians/interface/JacobianCartesianToLocal.h"

using namespace reco;

TrackDetectorAssociator::TrackDetectorAssociator() 
{
   ivProp_ = 0;
   defProp_ = 0;
   useDefaultPropagator_ = false;
}

TrackDetectorAssociator::~TrackDetectorAssociator()
{
   if (defProp_) delete defProp_;
}

void TrackDetectorAssociator::setPropagator( const Propagator* ptr)
{
   ivProp_ = ptr;
   cachedTrajectory_.setPropagator(ivProp_);
}

void TrackDetectorAssociator::useDefaultPropagator()
{
   useDefaultPropagator_ = true;
}


void TrackDetectorAssociator::init( const edm::EventSetup& iSetup )
{
   // access the calorimeter geometry
   iSetup.get<CaloGeometryRecord>().get(theCaloGeometry_);
   if (!theCaloGeometry_.isValid()) 
     throw cms::Exception("FatalError") << "Unable to find CaloGeometryRecord in event!\n";
   
   // get the tracking Geometry
   iSetup.get<GlobalTrackingGeometryRecord>().get(theTrackingGeometry_);
   if (!theTrackingGeometry_.isValid()) 
     throw cms::Exception("FatalError") << "Unable to find GlobalTrackingGeometryRecord in event!\n";
   
   if (useDefaultPropagator_ && (! defProp_ || theMagneticFeildWatcher_.check(iSetup) ) ) {
      // setup propagator
      edm::ESHandle<MagneticField> bField;
      iSetup.get<IdealMagneticFieldRecord>().get(bField);
      
      SteppingHelixPropagator* prop  = new SteppingHelixPropagator(&*bField,anyDirection);
      prop->setMaterialMode(false);
      prop->applyRadX0Correction(true);
      // prop->setDebug(true); // tmp
      defProp_ = prop;
      setPropagator(defProp_);
   }

   // access the GEM geometry
   iSetup.get<MuonGeometryRecord>().get(gemGeom);

   iSetup.get<DetIdAssociatorRecord>().get("EcalDetIdAssociator", ecalDetIdAssociator_);
   iSetup.get<DetIdAssociatorRecord>().get("HcalDetIdAssociator", hcalDetIdAssociator_);
   iSetup.get<DetIdAssociatorRecord>().get("HODetIdAssociator", hoDetIdAssociator_);
   iSetup.get<DetIdAssociatorRecord>().get("CaloDetIdAssociator", caloDetIdAssociator_);
   iSetup.get<DetIdAssociatorRecord>().get("MuonDetIdAssociator", muonDetIdAssociator_);
   iSetup.get<DetIdAssociatorRecord>().get("PreshowerDetIdAssociator", preshowerDetIdAssociator_);
}

TrackDetMatchInfo TrackDetectorAssociator::associate( const edm::Event& iEvent,
					      const edm::EventSetup& iSetup,
					      const FreeTrajectoryState& fts,
					      const AssociatorParameters& parameters )
{
   return associate(iEvent,iSetup,parameters,&fts);
}

TrackDetMatchInfo TrackDetectorAssociator::associate( const edm::Event& iEvent,
						      const edm::EventSetup& iSetup,
						      const AssociatorParameters& parameters,
						      const FreeTrajectoryState* innerState,
						      const FreeTrajectoryState* outerState)
{
   TrackDetMatchInfo info;
   if (! parameters.useEcal && ! parameters.useCalo && ! parameters.useHcal &&
       ! parameters.useHO && ! parameters.useMuon && !parameters.usePreshower)
     throw cms::Exception("ConfigurationError") << 
     "Configuration error! No subdetector was selected for the track association.";
   
   SteppingHelixStateInfo trackOrigin(*innerState);
   info.stateAtIP = *innerState;
   cachedTrajectory_.setStateAtIP(trackOrigin);
   
   init( iSetup );
   // get track trajectory
   // ECAL points (EB+EE)
   // If the phi angle between a track entrance and exit points is more
   // than 2 crystals, it is possible that the track will cross 3 crystals
   // and therefore one has to check at least 3 points along the track
   // trajectory inside ECAL. In order to have a chance to cross 4 crystalls
   // in the barrel, a track should have P_t as low as 3 GeV or smaller
   // If it's necessary, number of points along trajectory can be increased
   
   info.setCaloGeometry(theCaloGeometry_);
   
   cachedTrajectory_.reset_trajectory();
   // estimate propagation outer boundaries based on 
   // requested sub-detector information. For now limit
   // propagation region only if muon matching is not 
   // requested.
   double HOmaxR = hoDetIdAssociator_->volume().maxR();
   double HOmaxZ = hoDetIdAssociator_->volume().maxZ();
   double minR = ecalDetIdAssociator_->volume().minR();
   double minZ = preshowerDetIdAssociator_->volume().minZ();
   cachedTrajectory_.setMaxHORadius(HOmaxR);
   cachedTrajectory_.setMaxHOLength(HOmaxZ*2.);
   cachedTrajectory_.setMinDetectorRadius(minR);
   cachedTrajectory_.setMinDetectorLength(minZ*2.);

   if (parameters.useMuon) {
     double maxR = muonDetIdAssociator_->volume().maxR();
     double maxZ = muonDetIdAssociator_->volume().maxZ();
     cachedTrajectory_.setMaxDetectorRadius(maxR);
     cachedTrajectory_.setMaxDetectorLength(maxZ*2.);
   }
   else {
     cachedTrajectory_.setMaxDetectorRadius(HOmaxR);
     cachedTrajectory_.setMaxDetectorLength(HOmaxZ*2.);
   }
   
   // If track extras exist and outerState is before HO maximum, then use outerState
   if (outerState) {
     if (outerState->position().perp()<HOmaxR && fabs(outerState->position().z())<HOmaxZ) {
       LogTrace("TrackAssociator") << "Using outerState as trackOrigin at Rho=" << outerState->position().perp()
             << "  Z=" << outerState->position().z() << "\n";
       trackOrigin = SteppingHelixStateInfo(*outerState);
     }
     else if(innerState) {
       LogTrace("TrackAssociator") << "Using innerState as trackOrigin at Rho=" << innerState->position().perp()
             << "  Z=" << innerState->position().z() << "\n";
       trackOrigin = SteppingHelixStateInfo(*innerState);
     }
   }

   if ( trackOrigin.momentum().mag() == 0 ) return info;
   if ( std::isnan(trackOrigin.momentum().x()) or std::isnan(trackOrigin.momentum().y()) or std::isnan(trackOrigin.momentum().z()) ) return info;
   if ( ! cachedTrajectory_.propagateAll(trackOrigin) ) return info;
   
   // get trajectory in calorimeters
   cachedTrajectory_.findEcalTrajectory( ecalDetIdAssociator_->volume() );
   cachedTrajectory_.findHcalTrajectory( hcalDetIdAssociator_->volume() );
   cachedTrajectory_.findHOTrajectory( hoDetIdAssociator_->volume() );
   cachedTrajectory_.findPreshowerTrajectory( preshowerDetIdAssociator_->volume() );

   info.trkGlobPosAtEcal = getPoint( cachedTrajectory_.getStateAtEcal().position() );
   info.trkGlobPosAtHcal = getPoint( cachedTrajectory_.getStateAtHcal().position() );
   info.trkGlobPosAtHO  = getPoint( cachedTrajectory_.getStateAtHO().position() );
   
   info.trkMomAtEcal = cachedTrajectory_.getStateAtEcal().momentum();
   info.trkMomAtHcal = cachedTrajectory_.getStateAtHcal().momentum();
   info.trkMomAtHO   = cachedTrajectory_.getStateAtHO().momentum();
   
   if (parameters.useEcal) fillEcal( iEvent, info, parameters);
   if (parameters.useCalo) fillCaloTowers( iEvent, info, parameters);
   if (parameters.useHcal) fillHcal( iEvent, info, parameters);
   if (parameters.useHO)   fillHO( iEvent, info, parameters);
   if (parameters.usePreshower) fillPreshower( iEvent, info, parameters);
   if (parameters.useMuon) fillMuon( iEvent, info, parameters);
   if (parameters.truthMatch) fillCaloTruth( iEvent, info, parameters);
   
   return info;
}

void TrackDetectorAssociator::fillEcal( const edm::Event& iEvent,
				TrackDetMatchInfo& info,
				const AssociatorParameters& parameters)
{
   const std::vector<SteppingHelixStateInfo>& trajectoryStates = cachedTrajectory_.getEcalTrajectory();
   
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = trajectoryStates.begin();
       itr != trajectoryStates.end(); itr++) 
     LogTrace("TrackAssociator") << "ECAL trajectory point (rho, z, phi): " << itr->position().perp() <<
     ", " << itr->position().z() << ", " << itr->position().phi();

   std::vector<GlobalPoint> coreTrajectory;
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = trajectoryStates.begin();
       itr != trajectoryStates.end(); itr++) coreTrajectory.push_back(itr->position());
   
   if(coreTrajectory.empty()) {
      LogTrace("TrackAssociator") << "ECAL track trajectory is empty; moving on\n";
      info.isGoodEcal = 0;
      return;
   }
   info.isGoodEcal = 1;

   // Find ECAL crystals
   edm::Handle<EBRecHitCollection> EBRecHits;
   iEvent.getByToken(parameters.EBRecHitsToken, EBRecHits);
   if (!EBRecHits.isValid()) throw cms::Exception("FatalError") << "Unable to find EBRecHitCollection in the event!\n";

   edm::Handle<EERecHitCollection> EERecHits;
   iEvent.getByToken(parameters.EERecHitsToken, EERecHits);
   if (!EERecHits.isValid()) throw cms::Exception("FatalError") << "Unable to find EERecHitCollection in event!\n";

   std::set<DetId> ecalIdsInRegion;
   if (parameters.accountForTrajectoryChangeCalo){
      // get trajectory change with respect to initial state
      DetIdAssociator::MapRange mapRange = getMapRange(cachedTrajectory_.trajectoryDelta(CachedTrajectory::IpToEcal),
						       parameters.dREcalPreselection);
      ecalIdsInRegion = ecalDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0],mapRange);
   } else ecalIdsInRegion = ecalDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0], parameters.dREcalPreselection);
   LogTrace("TrackAssociator") << "ECAL hits in the region: " << ecalIdsInRegion.size();
   if (parameters.dREcalPreselection > parameters.dREcal)
     ecalIdsInRegion =  ecalDetIdAssociator_->getDetIdsInACone(ecalIdsInRegion, coreTrajectory, parameters.dREcal);
   LogTrace("TrackAssociator") << "ECAL hits in the cone: " << ecalIdsInRegion.size();
   info.crossedEcalIds  = ecalDetIdAssociator_->getCrossedDetIds(ecalIdsInRegion, coreTrajectory);
   const std::vector<DetId>& crossedEcalIds =  info.crossedEcalIds;
   LogTrace("TrackAssociator") << "ECAL crossed hits " << crossedEcalIds.size();
   

   // add EcalRecHits
   for(std::vector<DetId>::const_iterator itr=crossedEcalIds.begin(); itr!=crossedEcalIds.end();itr++)
   {
      std::vector<EcalRecHit>::const_iterator ebHit = (*EBRecHits).find(*itr);
      std::vector<EcalRecHit>::const_iterator eeHit = (*EERecHits).find(*itr);
      if(ebHit != (*EBRecHits).end()) 
         info.crossedEcalRecHits.push_back(&*ebHit);
      else if(eeHit != (*EERecHits).end()) 
         info.crossedEcalRecHits.push_back(&*eeHit);
      else  
         LogTrace("TrackAssociator") << "Crossed EcalRecHit is not found for DetId: " << itr->rawId();
   }
   for(std::set<DetId>::const_iterator itr=ecalIdsInRegion.begin(); itr!=ecalIdsInRegion.end();itr++)
   {
      std::vector<EcalRecHit>::const_iterator ebHit = (*EBRecHits).find(*itr);
      std::vector<EcalRecHit>::const_iterator eeHit = (*EERecHits).find(*itr);
      if(ebHit != (*EBRecHits).end()) 
         info.ecalRecHits.push_back(&*ebHit);
      else if(eeHit != (*EERecHits).end()) 
         info.ecalRecHits.push_back(&*eeHit);
      else 
         LogTrace("TrackAssociator") << "EcalRecHit from the cone is not found for DetId: " << itr->rawId();
   }
}

void TrackDetectorAssociator::fillCaloTowers( const edm::Event& iEvent,
				      TrackDetMatchInfo& info,
				      const AssociatorParameters& parameters)
{
   // use ECAL and HCAL trajectories to match a tower. (HO isn't used for matching).
   std::vector<GlobalPoint> trajectory;
   const std::vector<SteppingHelixStateInfo>& ecalTrajectoryStates = cachedTrajectory_.getEcalTrajectory();
   const std::vector<SteppingHelixStateInfo>& hcalTrajectoryStates = cachedTrajectory_.getHcalTrajectory();
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = ecalTrajectoryStates.begin();
       itr != ecalTrajectoryStates.end(); itr++) trajectory.push_back(itr->position());
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = hcalTrajectoryStates.begin();
       itr != hcalTrajectoryStates.end(); itr++) trajectory.push_back(itr->position());
   
   if(trajectory.empty()) {
      LogTrace("TrackAssociator") << "HCAL trajectory is empty; moving on\n";
      info.isGoodCalo = 0;
      return;
   }
   info.isGoodCalo = 1;
   
   // find crossed CaloTowers
   edm::Handle<CaloTowerCollection> caloTowers;
   iEvent.getByToken(parameters.caloTowersToken, caloTowers);
   if (!caloTowers.isValid())  throw cms::Exception("FatalError") << "Unable to find CaloTowers in event!\n";
   
   std::set<DetId> caloTowerIdsInRegion;
   if (parameters.accountForTrajectoryChangeCalo){
      // get trajectory change with respect to initial state
      DetIdAssociator::MapRange mapRange = getMapRange(cachedTrajectory_.trajectoryDelta(CachedTrajectory::IpToHcal),
						       parameters.dRHcalPreselection);
      caloTowerIdsInRegion = caloDetIdAssociator_->getDetIdsCloseToAPoint(trajectory[0],mapRange);
   } else caloTowerIdsInRegion = caloDetIdAssociator_->getDetIdsCloseToAPoint(trajectory[0], parameters.dRHcalPreselection);
   
   LogTrace("TrackAssociator") << "Towers in the region: " << caloTowerIdsInRegion.size();

   auto caloTowerIdsInAConeBegin = caloTowerIdsInRegion.begin();
   auto caloTowerIdsInAConeEnd = caloTowerIdsInRegion.end();
   std::set<DetId> caloTowerIdsInAConeTmp;
   if (!caloDetIdAssociator_->selectAllInACone(parameters.dRHcal)){
     caloTowerIdsInAConeTmp = caloDetIdAssociator_->getDetIdsInACone(caloTowerIdsInRegion, trajectory, parameters.dRHcal);
     caloTowerIdsInAConeBegin = caloTowerIdsInAConeTmp.begin();
     caloTowerIdsInAConeEnd = caloTowerIdsInAConeTmp.end();
   }
   LogTrace("TrackAssociator") << "Towers in the cone: " << std::distance(caloTowerIdsInAConeBegin, caloTowerIdsInAConeEnd);

   info.crossedTowerIds = caloDetIdAssociator_->getCrossedDetIds(caloTowerIdsInRegion, trajectory);
   const std::vector<DetId>& crossedCaloTowerIds = info.crossedTowerIds;
   LogTrace("TrackAssociator") << "Towers crossed: " << crossedCaloTowerIds.size();
   
   // add CaloTowers
   for(std::vector<DetId>::const_iterator itr=crossedCaloTowerIds.begin(); itr!=crossedCaloTowerIds.end();itr++)
     {
	CaloTowerCollection::const_iterator tower = (*caloTowers).find(*itr);
	if(tower != (*caloTowers).end()) 
	  info.crossedTowers.push_back(&*tower);
	else
	  LogTrace("TrackAssociator") << "Crossed CaloTower is not found for DetId: " << (*itr).rawId();
     }
   
   for(std::set<DetId>::const_iterator itr=caloTowerIdsInAConeBegin; itr!=caloTowerIdsInAConeEnd;itr++)
     {
	CaloTowerCollection::const_iterator tower = (*caloTowers).find(*itr);
	if(tower != (*caloTowers).end()) 
	  info.towers.push_back(&*tower);
	else 
	  LogTrace("TrackAssociator") << "CaloTower from the cone is not found for DetId: " << (*itr).rawId();
     }
   
}

void TrackDetectorAssociator::fillPreshower( const edm::Event& iEvent,
					     TrackDetMatchInfo& info,
					     const AssociatorParameters& parameters)
{
   std::vector<GlobalPoint> trajectory;
   const std::vector<SteppingHelixStateInfo>& trajectoryStates = cachedTrajectory_.getPreshowerTrajectory();
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = trajectoryStates.begin();
       itr != trajectoryStates.end(); itr++) trajectory.push_back(itr->position());
   
   if(trajectory.empty()) {
      LogTrace("TrackAssociator") << "Preshower trajectory is empty; moving on\n";
      return;
   }
   
   std::set<DetId> idsInRegion = 
     preshowerDetIdAssociator_->getDetIdsCloseToAPoint(trajectory[0], 
						       parameters.dRPreshowerPreselection);
   
   LogTrace("TrackAssociator") << "Number of Preshower Ids in the region: " << idsInRegion.size();
   info.crossedPreshowerIds = preshowerDetIdAssociator_->getCrossedDetIds(idsInRegion, trajectory);
   LogTrace("TrackAssociator") << "Number of Preshower Ids in crossed: " << info.crossedPreshowerIds.size();
}


void TrackDetectorAssociator::fillHcal( const edm::Event& iEvent,
				TrackDetMatchInfo& info,
				const AssociatorParameters& parameters)
{
   const std::vector<SteppingHelixStateInfo>& trajectoryStates = cachedTrajectory_.getHcalTrajectory();

   std::vector<GlobalPoint> coreTrajectory;
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = trajectoryStates.begin();
       itr != trajectoryStates.end(); itr++) coreTrajectory.push_back(itr->position());   
   
   if(coreTrajectory.empty()) {
      LogTrace("TrackAssociator") << "HCAL trajectory is empty; moving on\n";
      info.isGoodHcal = 0;
      return;
   }
   info.isGoodHcal = 1;
   
   // find crossed Hcals
   edm::Handle<HBHERecHitCollection> collection;
   iEvent.getByToken(parameters.HBHEcollToken, collection);
   if ( ! collection.isValid() ) throw cms::Exception("FatalError") << "Unable to find HBHERecHits in event!\n";
   
   std::set<DetId> idsInRegion;
   if (parameters.accountForTrajectoryChangeCalo){
      // get trajectory change with respect to initial state
      DetIdAssociator::MapRange mapRange = getMapRange(cachedTrajectory_.trajectoryDelta(CachedTrajectory::IpToHcal),
						       parameters.dRHcalPreselection);
      idsInRegion = hcalDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0], mapRange);
   } else idsInRegion = hcalDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0], parameters.dRHcalPreselection);
   
   LogTrace("TrackAssociator") << "HCAL hits in the region: " << idsInRegion.size() << "\n" << DetIdInfo::info(idsInRegion,0);

   auto idsInAConeBegin = idsInRegion.begin();
   auto idsInAConeEnd = idsInRegion.end();
   std::set<DetId> idsInAConeTmp;
   if (! hcalDetIdAssociator_->selectAllInACone(parameters.dRHcal)){
     idsInAConeTmp = hcalDetIdAssociator_->getDetIdsInACone(idsInRegion, coreTrajectory, parameters.dRHcal);
     idsInAConeBegin = idsInAConeTmp.begin();
     idsInAConeEnd = idsInAConeTmp.end();
   }
   LogTrace("TrackAssociator") << "HCAL hits in the cone: " << std::distance(idsInAConeBegin, idsInAConeEnd) << "\n" 
			       << DetIdInfo::info(std::set<DetId>(idsInAConeBegin, idsInAConeEnd), 0);
   info.crossedHcalIds = hcalDetIdAssociator_->getCrossedDetIds(idsInRegion, coreTrajectory);
   const std::vector<DetId>& crossedIds = info.crossedHcalIds;
   LogTrace("TrackAssociator") << "HCAL hits crossed: " << crossedIds.size() << "\n" << DetIdInfo::info(crossedIds,0);
   
   // add Hcal
   for(std::vector<DetId>::const_iterator itr=crossedIds.begin(); itr!=crossedIds.end();itr++)
     {
	HBHERecHitCollection::const_iterator hit = (*collection).find(*itr);
	if( hit != (*collection).end() ) 
	  info.crossedHcalRecHits.push_back(&*hit);
	else
	  LogTrace("TrackAssociator") << "Crossed HBHERecHit is not found for DetId: " << itr->rawId();
     }
   for(std::set<DetId>::const_iterator itr=idsInAConeBegin; itr!=idsInAConeEnd;itr++)
     {
	HBHERecHitCollection::const_iterator hit = (*collection).find(*itr);
	if( hit != (*collection).end() ) 
	  info.hcalRecHits.push_back(&*hit);
	else 
	  LogTrace("TrackAssociator") << "HBHERecHit from the cone is not found for DetId: " << itr->rawId();
     }
}

void TrackDetectorAssociator::fillHO( const edm::Event& iEvent,
			      TrackDetMatchInfo& info,
			      const AssociatorParameters& parameters)
{
   const std::vector<SteppingHelixStateInfo>& trajectoryStates = cachedTrajectory_.getHOTrajectory();

   std::vector<GlobalPoint> coreTrajectory;
   for(std::vector<SteppingHelixStateInfo>::const_iterator itr = trajectoryStates.begin();
       itr != trajectoryStates.end(); itr++) coreTrajectory.push_back(itr->position());

   if(coreTrajectory.empty()) {
      LogTrace("TrackAssociator") << "HO trajectory is empty; moving on\n";
      info.isGoodHO = 0;
      return;
   }
   info.isGoodHO = 1;
   
   // find crossed HOs
   edm::Handle<HORecHitCollection> collection;
   iEvent.getByToken(parameters.HOcollToken, collection);
   if ( ! collection.isValid() ) throw cms::Exception("FatalError") << "Unable to find HORecHits in event!\n";
   
   std::set<DetId> idsInRegion;
   if (parameters.accountForTrajectoryChangeCalo){
      // get trajectory change with respect to initial state
      DetIdAssociator::MapRange mapRange = getMapRange(cachedTrajectory_.trajectoryDelta(CachedTrajectory::IpToHO),
						       parameters.dRHcalPreselection);
      idsInRegion = hoDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0], mapRange);
   } else idsInRegion = hoDetIdAssociator_->getDetIdsCloseToAPoint(coreTrajectory[0], parameters.dRHcalPreselection);
   
   LogTrace("TrackAssociator") << "idsInRegion.size(): " << idsInRegion.size();

   auto idsInAConeBegin = idsInRegion.begin();
   auto idsInAConeEnd = idsInRegion.end();
   std::set<DetId> idsInAConeTmp;
   if (! hoDetIdAssociator_->selectAllInACone(parameters.dRHcal)){
     idsInAConeTmp = hoDetIdAssociator_->getDetIdsInACone(idsInRegion, coreTrajectory, parameters.dRHcal);
     idsInAConeBegin = idsInAConeTmp.begin();
     idsInAConeEnd = idsInAConeTmp.end();
   }
   LogTrace("TrackAssociator") << "idsInACone.size(): " << std::distance(idsInAConeBegin, idsInAConeEnd);
   info.crossedHOIds = hoDetIdAssociator_->getCrossedDetIds(idsInRegion, coreTrajectory);
   const std::vector<DetId>& crossedIds = info.crossedHOIds;
   
   // add HO
   for(std::vector<DetId>::const_iterator itr=crossedIds.begin(); itr!=crossedIds.end();itr++)
     {
	HORecHitCollection::const_iterator hit = (*collection).find(*itr);
	if( hit != (*collection).end() ) 
	  info.crossedHORecHits.push_back(&*hit);
	else
	  LogTrace("TrackAssociator") << "Crossed HORecHit is not found for DetId: " << itr->rawId();
     }

   for(std::set<DetId>::const_iterator itr=idsInAConeBegin; itr!=idsInAConeEnd;itr++)
     {
	HORecHitCollection::const_iterator hit = (*collection).find(*itr);
	if( hit != (*collection).end() ) 
	  info.hoRecHits.push_back(&*hit);
	else 
	  LogTrace("TrackAssociator") << "HORecHit from the cone is not found for DetId: " << itr->rawId();
     }
}

FreeTrajectoryState TrackDetectorAssociator::getFreeTrajectoryState( const edm::EventSetup& iSetup, 
									    const SimTrack& track, 
									    const SimVertex& vertex )
{
   GlobalVector vector( track.momentum().x(), track.momentum().y(), track.momentum().z() );
   GlobalPoint point( vertex.position().x(), vertex.position().y(), vertex.position().z() );

   int charge = track.type( )> 0 ? -1 : 1; // lepton convention
   if ( abs(track.type( )) == 211 || // pion
	abs(track.type( )) == 321 || // kaon
	abs(track.type( )) == 2212 )
     charge = track.type( )< 0 ? -1 : 1;
   return getFreeTrajectoryState(iSetup, vector, point, charge);
}

FreeTrajectoryState TrackDetectorAssociator::getFreeTrajectoryState( const edm::EventSetup& iSetup,
									    const GlobalVector& momentum, 
									    const GlobalPoint& vertex,
									    const int charge)
{
   edm::ESHandle<MagneticField> bField;
   iSetup.get<IdealMagneticFieldRecord>().get(bField);
   
   GlobalTrajectoryParameters tPars(vertex, momentum, charge, &*bField);
   
   ROOT::Math::SMatrixIdentity id;
   AlgebraicSymMatrix66 covT(id); covT *= 1e-6; // initialize to sigma=1e-3
   CartesianTrajectoryError tCov(covT);
   
   return FreeTrajectoryState(tPars, tCov);
}


FreeTrajectoryState TrackDetectorAssociator::getFreeTrajectoryState( const edm::EventSetup& iSetup,
									    const reco::Track& track )
{
   edm::ESHandle<MagneticField> bField;
   iSetup.get<IdealMagneticFieldRecord>().get(bField);
   
   GlobalVector vector( track.momentum().x(), track.momentum().y(), track.momentum().z() );

   GlobalPoint point( track.vertex().x(), track.vertex().y(),  track.vertex().z() );

   GlobalTrajectoryParameters tPars(point, vector, track.charge(), &*bField);
   
   // FIX THIS !!!
   // need to convert from perigee to global or helix (curvilinear) frame
   // for now just an arbitrary matrix.
   ROOT::Math::SMatrixIdentity id;
   AlgebraicSymMatrix66 covT(id); covT *= 1e-6; // initialize to sigma=1e-3
   CartesianTrajectoryError tCov(covT);
   
   return FreeTrajectoryState(tPars, tCov);
}

DetIdAssociator::MapRange TrackDetectorAssociator::getMapRange( const std::pair<float,float>& delta,
								const float dR )
{
   DetIdAssociator::MapRange mapRange;
   mapRange.dThetaPlus = dR;
   mapRange.dThetaMinus = dR;
   mapRange.dPhiPlus = dR;
   mapRange.dPhiMinus = dR;
   if ( delta.first > 0 ) 
     mapRange.dThetaPlus += delta.first;
   else
     mapRange.dThetaMinus += fabs(delta.first);
   if ( delta.second > 0 ) 
     mapRange.dPhiPlus += delta.second;
   else
     mapRange.dPhiMinus += fabs(delta.second);
   LogTrace("TrackAssociator") << "Selection range: (dThetaPlus, dThetaMinus, dPhiPlus, dPhiMinus, dRPreselection): " <<
     mapRange.dThetaPlus << ", " << mapRange.dThetaMinus << ", " << 
     mapRange.dPhiPlus << ", " << mapRange.dPhiMinus << ", " << dR;
   return mapRange;
}

void TrackDetectorAssociator::getTAMuonChamberMatches(std::vector<TAMuonChamberMatch>& matches,
						    const AssociatorParameters& parameters)
{
   // Strategy:
   //    Propagate through the whole detector, estimate change in eta and phi 
   //    along the trajectory, add this to dRMuon and find DetIds around this 
   //    direction using the map. Then propagate fast to each surface and apply 
   //    final matching criteria.

   // get the direction first
   SteppingHelixStateInfo trajectoryPoint = cachedTrajectory_.getStateAtHcal();
   // If trajectory point at HCAL is not valid, try to use the outer most state of the
   // trajectory instead.
   if(! trajectoryPoint.isValid() ) trajectoryPoint = cachedTrajectory_.getOuterState();
   if(! trajectoryPoint.isValid() ) {
      LogTrace("TrackAssociator") << 
	"trajectory position at HCAL is not valid. Assume the track cannot reach muon detectors and skip it";
      return;
   }

   GlobalVector direction = trajectoryPoint.momentum().unit();
   LogTrace("TrackAssociator") << "muon direction: " << direction << "\n\t and corresponding point: " <<
     trajectoryPoint.position() <<"\n";
   
   DetIdAssociator::MapRange mapRange = getMapRange(cachedTrajectory_.trajectoryDelta(CachedTrajectory::FullTrajectory),
						    parameters.dRMuonPreselection);
     
   // and find chamber DetIds

   std::set<DetId> muonIdsInRegion = 
     muonDetIdAssociator_->getDetIdsCloseToAPoint(trajectoryPoint.position(), mapRange);
   LogTrace("TrackAssociator") << "Number of chambers to check: " << muonIdsInRegion.size();
   for(std::set<DetId>::const_iterator detId = muonIdsInRegion.begin(); detId != muonIdsInRegion.end(); detId++)
   {
      const GeomDet* geomDet = muonDetIdAssociator_->getGeomDet(*detId);
      TrajectoryStateOnSurface stateOnSurface = cachedTrajectory_.propagate( &geomDet->surface() );
      if (! stateOnSurface.isValid()) {
         LogTrace("TrackAssociator") << "Failed to propagate the track; moving on\n\t"<<
	   "Element is not crosssed: " << DetIdInfo::info(*detId,0) << "\n";
         continue;
      }
      LocalPoint localPoint = geomDet->surface().toLocal(stateOnSurface.freeState()->position());
      LocalError localError = stateOnSurface.localError().positionError();
      float distanceX = 0;
      float distanceY = 0;
      float sigmaX = 0.0;
      float sigmaY = 0.0;
      if(const CSCChamber* cscChamber = dynamic_cast<const CSCChamber*>(geomDet) ) {
         const CSCChamberSpecs* chamberSpecs = cscChamber->specs();
         if(! chamberSpecs) {
            LogTrace("TrackAssociator") << "Failed to get CSCChamberSpecs from CSCChamber; moving on\n";
            continue;
         }
         const CSCLayerGeometry* layerGeometry = chamberSpecs->oddLayerGeometry(1);
         if(! layerGeometry) {
            LogTrace("TrackAssociator") << "Failed to get CSCLayerGeometry from CSCChamberSpecs; moving on\n";
            continue;
         }
         const CSCWireTopology* wireTopology = layerGeometry->wireTopology();
         if(! wireTopology) {
            LogTrace("TrackAssociator") << "Failed to get CSCWireTopology from CSCLayerGeometry; moving on\n";
            continue;
         }

         float wideWidth      = wireTopology->wideWidthOfPlane();
         float narrowWidth    = wireTopology->narrowWidthOfPlane();
         float length         = wireTopology->lengthOfPlane();
         // If slanted, there is no y offset between local origin and symmetry center of wire plane
         float yOfFirstWire   = fabs(wireTopology->wireAngle())>1.E-06 ? -0.5*length : wireTopology->yOfWire(1);
         // y offset between local origin and symmetry center of wire plane
         float yCOWPOffset    = yOfFirstWire+0.5*length;

         // tangent of the incline angle from inside the trapezoid
         float tangent = (wideWidth-narrowWidth)/(2.*length);
         // y position wrt bottom of trapezoid
         float yPrime  = localPoint.y()+fabs(yOfFirstWire);
         // half trapezoid width at y' is 0.5 * narrowWidth + x side of triangle with the above tangent and side y'
         float halfWidthAtYPrime = 0.5*narrowWidth+yPrime*tangent;
         distanceX = fabs(localPoint.x()) - halfWidthAtYPrime;
         distanceY = fabs(localPoint.y()-yCOWPOffset) - 0.5*length;
	       sigmaX = distanceX/sqrt(localError.xx());
         sigmaY = distanceY/sqrt(localError.yy());
      }
      else {
         distanceX = fabs(localPoint.x()) - geomDet->surface().bounds().width()/2.;
         distanceY = fabs(localPoint.y()) - geomDet->surface().bounds().length()/2.;
	       sigmaX = distanceX/sqrt(localError.xx());
         sigmaY = distanceY/sqrt(localError.yy());

	       // if(detId->subdetId() == 3) {
	       //    RPCDetId Rsid = RPCDetId(detId->rawId());
	       //    std::cout<< Rsid <<std::endl;
	       //    std::cout<<"RPCChamber width="<< geomDet->surface().bounds().width() <<", length="<< geomDet->surface().bounds().length() <<std::endl;
	       //  }
	       // if(const GEMSuperChamber* gemChamber = dynamic_cast<const GEMSuperChamber*>(geomDet) ) {
	       //   if(gemChamber) {
	       if(detId->subdetId() == 4) {
	          std::cout << "here1" << std::endl; 
	          // gem width and length are interchanged - need to fix
	          //distanceX = fabs(localPoint.x()) - geomDet->surface().bounds().width();
	          const GEMChamber* gemChamber = dynamic_cast<const GEMChamber*>(geomDet);
	          int nEtaPartitions = gemChamber->nEtaPartitions(); // FIXME temp fix for chambersize
	          distanceY = fabs(localPoint.y()) - geomDet->surface().bounds().length()*nEtaPartitions; // FIXME temp fix for chambersize
	          sigmaX = distanceX/sqrt(localError.xx());
	          sigmaY = distanceY/sqrt(localError.yy());
	          // std::cout<<"getTAMuonChamberMatches::GEM distanceX="<< distanceX <<", distanceY="<< distanceY <<std::endl;
	          // GEMDetId Rsid = GEMDetId(detId->rawId());
	          // std::cout<< Rsid <<std::endl;
	           std::cout<<"GEMSuperChamber width="<< geomDet->surface().bounds().width() <<", length="<< geomDet->surface().bounds().length() <<std::endl;
	          // auto& rolls(gemChamber->etaPartitions());
	          // for (auto roll : rolls){
	          //   //const TrapezoidalStripTopology* top_(dynamic_cast<const TrapezoidalStripTopology*>(&(roll->topology())));
	          //   auto& parameters(roll->specs()->parameters());
	          //   double bottomLength(parameters[0]); bottomLength = 2*bottomLength; // bottom is largest length, so furtest away from beamline
	          //   double topLength(parameters[1]);    topLength    = 2*topLength;    // top is shortest length, so closest to beamline
	          //   double height(parameters[2]);       height       = 2*height;
	          //   std::cout<<"GEM roll bottomLength="<< bottomLength <<", topLength="<< topLength <<", height="<< height <<std::endl;
	      
	          //   std::cout<<"GEM roll width="<< roll->surface().bounds().width() <<", length="<< roll->surface().bounds().length()<<std::endl;
	          // }
	          // }
	       }
	 
      }
      if ( (distanceX < parameters.muonMaxDistanceX && distanceY < parameters.muonMaxDistanceY) ||
	   (sigmaX < parameters.muonMaxDistanceSigmaX && sigmaY < parameters.muonMaxDistanceSigmaY) ) {
	LogTrace("TrackAssociator") << "found a match: " << DetIdInfo::info(*detId,0) << "\n";
         TAMuonChamberMatch match;
         match.tState = stateOnSurface;
         match.localDistanceX = distanceX;
         match.localDistanceY = distanceY;
         match.id = *detId;
         matches.push_back(match);
      } else {
	LogTrace("TrackAssociator") << "chamber is too far: " << 
	  DetIdInfo::info(*detId,0) << "\n\tdistanceX: " << distanceX << "\t distanceY: " << distanceY <<
	  "\t sigmaX: " << sigmaX << "\t sigmaY: " << sigmaY << "\n";
      }	
   }
   
}

void TrackDetectorAssociator::fillMuon( const edm::Event& iEvent,
					TrackDetMatchInfo& info,
					const AssociatorParameters& parameters)
{
   // Get the segments from the event
   edm::Handle<DTRecSegment4DCollection> dtSegments;
   iEvent.getByToken(parameters.dtSegmentsToken, dtSegments);
   if (! dtSegments.isValid()) 
     throw cms::Exception("FatalError") << "Unable to find DTRecSegment4DCollection in event!\n";
   
   edm::Handle<CSCSegmentCollection> cscSegments;
   iEvent.getByToken(parameters.cscSegmentsToken, cscSegments);
   if (! cscSegments.isValid()) 
     throw cms::Exception("FatalError") << "Unable to find CSCSegmentCollection in event!\n";

   edm::Handle<GEMSegmentCollection> gemSegments;
   iEvent.getByToken(parameters.gemSegmentsToken, gemSegments );
   if (! gemSegments.isValid())
     throw cms::Exception("FatalError") << "Unable to find GEMSegmentCollection in event!\n";

   ///// get a set of DetId's in a given direction
   
   // check the map of available segments
   // if there is no segments in a given direction at all,
   // then there is no point to fly there.
   // 
   // MISSING
   // Possible solution: quick search for presence of segments 
   // for the set of DetIds

   // get a set of matches corresponding to muon chambers
   std::vector<TAMuonChamberMatch> matchedChambers;
   LogTrace("TrackAssociator") << "Trying to Get ChamberMatches" << std::endl;
   getTAMuonChamberMatches(matchedChambers, parameters);
   LogTrace("TrackAssociator") << "Chambers matched: " << matchedChambers.size() << "\n";
   
   // Iterate over all chamber matches and fill segment matching 
   // info if it's available
   for(std::vector<TAMuonChamberMatch>::iterator matchedChamber = matchedChambers.begin(); 
       matchedChamber != matchedChambers.end(); matchedChamber++){
	   const GeomDet* geomDet = muonDetIdAssociator_->getGeomDet((*matchedChamber).id);
	   // DT chamber
	   if(const DTChamber* chamber = dynamic_cast<const DTChamber*>(geomDet) ) {
	     // Get the range for the corresponding segments
	     DTRecSegment4DCollection::range  range = dtSegments->get(chamber->id());
	     // Loop over the segments of this chamber
	     for (DTRecSegment4DCollection::const_iterator segment = range.first; segment!=range.second; segment++) {
	       if (addTAMuonSegmentMatch(*matchedChamber, &(*segment), parameters)) {
           matchedChamber->segments.back().dtSegmentRef = DTRecSegment4DRef(dtSegments, segment - dtSegments->begin());
         }
       }
	   }
     // CSC Chamber
     else if(const CSCChamber* chamber = dynamic_cast<const CSCChamber*>(geomDet) ) {
	     // Get the range for the corresponding segments
	     CSCSegmentCollection::range  range = cscSegments->get(chamber->id());
	     // Loop over the segments
	     for (CSCSegmentCollection::const_iterator segment = range.first; segment!=range.second; segment++) {
		     if (addTAMuonSegmentMatch(*matchedChamber, &(*segment), parameters)) {
           matchedChamber->segments.back().cscSegmentRef = CSCSegmentRef(cscSegments, segment - cscSegments->begin());
         }
	     }
     }
     // GEM Chamber
     //else if(const GEMSuperChamber* chamber = dynamic_cast<const GEMSuperChamber*>(geomDet) ) {
     else if(const GEMChamber* chamber = dynamic_cast<const GEMChamber*>(geomDet) ) {
	     // Get the range for the corresponding segments
	     GEMSegmentCollection::range  range = gemSegments->get(chamber->id());
	     // Loop over the segments
	     std::cout<<"TrackDetectorAssociator::GEM::found gem chamber"<<std::endl;
	     for (GEMSegmentCollection::const_iterator segment = range.first; segment!=range.second; segment++) {
	       if (addTAMuonSegmentMatch(*matchedChamber, &(*segment), parameters)) {
	         std::cout<<"TrackDetectorAssociator::GEM::matched to gem seg"<<std::endl;
	         matchedChamber->segments.back().gemSegmentRef = GEMSegmentRef(gemSegments, segment - gemSegments->begin());
	       }
	     }
     }
   
     info.chambers.push_back(*matchedChamber);

   }
}


bool TrackDetectorAssociator::addTAMuonSegmentMatch(TAMuonChamberMatch& matchedChamber,
					  const RecSegment* segment,
					  const AssociatorParameters& parameters)
{
   LogTrace("TrackAssociator")
     << "Segment local position: " << segment->localPosition() << "\n"
     << std::hex << segment->geographicalId().rawId() << "\n";
   
   const GeomDet* chamber = muonDetIdAssociator_->getGeomDet(matchedChamber.id);
   TrajectoryStateOnSurface trajectoryStateOnSurface = matchedChamber.tState;
   GlobalPoint segmentGlobalPosition = chamber->toGlobal(segment->localPosition());

   LogTrace("TrackAssociator")
     << "Segment global position: " << segmentGlobalPosition << " \t (R_xy,eta,phi): "
     << segmentGlobalPosition.perp() << "," << segmentGlobalPosition.eta() << "," << segmentGlobalPosition.phi() << "\n";

   LogTrace("TrackAssociator")
     << "\teta hit: " << segmentGlobalPosition.eta() << " \tpropagator: " << trajectoryStateOnSurface.freeState()->position().eta() << "\n"
     << "\tphi hit: " << segmentGlobalPosition.phi() << " \tpropagator: " << trajectoryStateOnSurface.freeState()->position().phi() << std::endl;
   
   bool isGood = false;
   bool isDTWithoutY = false;
   const DTRecSegment4D* dtseg = dynamic_cast<const DTRecSegment4D*>(segment);
   if ( dtseg && (! dtseg->hasZed()) )
      isDTWithoutY = true;

   double deltaPhi(fabs(segmentGlobalPosition.phi()-trajectoryStateOnSurface.freeState()->position().phi()));
   if(deltaPhi>M_PI) deltaPhi = fabs(deltaPhi-M_PI*2.);

   if( isDTWithoutY ){
	   isGood = deltaPhi < parameters.dRMuon;
	   // Be in chamber
	   isGood &= fabs(segmentGlobalPosition.eta()-trajectoryStateOnSurface.freeState()->position().eta()) < .3;
   }
   else if(const GEMChamber* gemchamber = dynamic_cast<const GEMChamber*>(chamber) ) {
     const GEMSegment* gemsegment = dynamic_cast<const GEMSegment*>(segment);
     int station = gemsegment->specificRecHits()[0].gemId().station();
     int chargeReco = trajectoryStateOnSurface.charge();
     // segment hit
     LocalPoint thisPosition(gemsegment->localPosition());
     LocalVector thisDirection(gemsegment->localDirection());
     GlobalPoint SegPos(gemchamber->toGlobal(thisPosition));
     GlobalVector SegDir(gemchamber->toGlobal(thisDirection));
/*
     std::cout
     << "[addTAMuonSegmentMatch]" << std::endl
     << "chamber station = " << gemchamber->id().station() << std::endl
     << "segment station = " << station << std::endl
     << "nEtaPartitions  = " << gemchamber->nEtaPartitions() << std::endl
     << "z position = " << SegPos.z() << std::endl << std::endl;
*/
     // track
     GlobalPoint r3FinalReco_glob(trajectoryStateOnSurface.freeState()->position().x(), 
                                  trajectoryStateOnSurface.freeState()->position().y(), 
                                  trajectoryStateOnSurface.freeState()->position().z()   );
     GlobalVector p3FinalReco_glob(trajectoryStateOnSurface.freeState()->momentum().x(), 
                                   trajectoryStateOnSurface.freeState()->momentum().y(),
                                   trajectoryStateOnSurface.freeState()->momentum().z()  );

     LocalPoint r3FinalReco = gemchamber->toLocal(r3FinalReco_glob);
     LocalVector p3FinalReco= gemchamber->toLocal(p3FinalReco_glob);
     // track error 
     LocalTrajectoryParameters ltp(r3FinalReco,p3FinalReco,chargeReco);
     JacobianCartesianToLocal jctl(gemchamber->surface(),ltp);
     AlgebraicMatrix56 jacobGlbToLoc = jctl.jacobian();
     AlgebraicSymMatrix66 covFinalReco = trajectoryStateOnSurface.hasError() ? trajectoryStateOnSurface.cartesianError().matrix() : AlgebraicSymMatrix66();;
 
     AlgebraicMatrix55 Ctmp =  (jacobGlbToLoc * covFinalReco) * ROOT::Math::Transpose(jacobGlbToLoc);
     AlgebraicSymMatrix55 C;  // I couldn't find any other way, so I resort to the brute force
     for(int i=0; i<5; ++i) {
       for(int j=0; j<5; ++j) {
         C[i][j] = Ctmp[i][j];
       }
     } 

     Double_t sigmax = sqrt(C[3][3]+gemsegment->localPositionError().xx() ),
              sigmay = sqrt(C[4][4]+gemsegment->localPositionError().yy() );

     std::cout
     << std::endl
     << "station = " << station << std::endl
     << "chamber = " << gemsegment->specificRecHits()[0].gemId().chamber() << std::endl
     << "roll = " << gemsegment->specificRecHits()[0].gemId().roll() << std::endl
     << "track r3 global : (" << r3FinalReco_glob.x() << ", " << r3FinalReco_glob.y() << ", " << r3FinalReco_glob.z() << ")" << std::endl
     << "track p3 global : (" << p3FinalReco_glob.x() << ", " << p3FinalReco_glob.y() << ", " << p3FinalReco_glob.z() << ")" << std::endl
     << "track r3 local : (" << r3FinalReco.x() << ", " << r3FinalReco.y() << ", " << r3FinalReco.z() << ")" << std::endl
     << "track p3 local : (" << p3FinalReco.x() << ", " << p3FinalReco.y() << ", " << p3FinalReco.z() << ")" << std::endl
     << "hit r3 global : (" << SegPos.x() << ", " << SegPos.y() << ", " << SegPos.z() << ")" << std::endl
     << "hit p3 global : (" << SegDir.x() << ", " << SegDir.y() << ", " << SegDir.z() << ")" << std::endl
     << "hit r3 local : (" << thisPosition.x() << ", " << thisPosition.y() << ", " << thisPosition.z() << ")" << std::endl
     << "hit p3 local : (" << thisDirection.x() << ", " << thisDirection.y() << ", " << thisDirection.z() << ")" << std::endl
     << "sigmax2 = " << C[3][3] << ", " << gemsegment->localPositionError().xx() << std::endl
     << "sigmay2 = " << C[4][4] << ", " << gemsegment->localPositionError().yy() << std::endl;

     bool X_MatchFound = false, Y_MatchFound = false, Dir_MatchFound = false;
    
     if (station == 1){
       if ( (std::abs(thisPosition.x()-r3FinalReco.x()) < (parameters.maxPullXGE11_ * sigmax)) &&
      (std::abs(thisPosition.x()-r3FinalReco.x()) < parameters.maxDiffXGE11_ ) ) X_MatchFound = true;
       if ( (std::abs(thisPosition.y()-r3FinalReco.y()) < (parameters.maxPullYGE11_ * sigmay)) &&
      (std::abs(thisPosition.y()-r3FinalReco.y()) < parameters.maxDiffYGE11_ ) ) Y_MatchFound = true;
     }
     if (station == 3){
       if ( (std::abs(thisPosition.x()-r3FinalReco.x()) < (parameters.maxPullXGE21_ * sigmax)) &&
      (std::abs(thisPosition.x()-r3FinalReco.x()) < parameters.maxDiffXGE21_ ) ) X_MatchFound = true;
       if ( (std::abs(thisPosition.y()-r3FinalReco.y()) < (parameters.maxPullYGE21_ * sigmay)) &&
      (std::abs(thisPosition.y()-r3FinalReco.y()) < parameters.maxDiffYGE21_ ) ) Y_MatchFound = true;
     }
     double segLocalPhi = thisDirection.phi();
     if (std::abs(reco::deltaPhi(p3FinalReco.phi(),segLocalPhi)) < parameters.maxDiffPhiDirection_) Dir_MatchFound = true;
     std::cout << "=============> X : " << X_MatchFound << ", Y : " << Y_MatchFound << ", Phi : " << Dir_MatchFound << std::endl;

     isGood = X_MatchFound && Y_MatchFound && Dir_MatchFound;
     if(isGood) std::cout << "+++++++++++++++> pass" << std::endl;
   }
   else isGood = sqrt( pow(segmentGlobalPosition.eta()-trajectoryStateOnSurface.freeState()->position().eta(),2) + 
			   deltaPhi*deltaPhi) < parameters.dRMuon;

   if(isGood) {
      TAMuonSegmentMatch muonSegment;
      muonSegment.segmentGlobalPosition = getPoint(segmentGlobalPosition);
      muonSegment.segmentLocalPosition = getPoint( segment->localPosition() );
      muonSegment.segmentLocalDirection = getVector( segment->localDirection() );
      muonSegment.segmentLocalErrorXX = segment->localPositionError().xx();
      muonSegment.segmentLocalErrorYY = segment->localPositionError().yy();
      muonSegment.segmentLocalErrorXY = segment->localPositionError().xy();
      muonSegment.segmentLocalErrorDxDz = segment->localDirectionError().xx();
      muonSegment.segmentLocalErrorDyDz = segment->localDirectionError().yy();
      
      // DANGEROUS - compiler cannot guaranty parameters ordering
      // AlgebraicSymMatrix segmentCovMatrix = segment->parametersError();
      // muonSegment.segmentLocalErrorXDxDz = segmentCovMatrix[2][0];
      // muonSegment.segmentLocalErrorYDyDz = segmentCovMatrix[3][1];
      muonSegment.segmentLocalErrorXDxDz = -999;
      muonSegment.segmentLocalErrorYDyDz = -999;
      muonSegment.hasZed = true;
      muonSegment.hasPhi = true;
      
      // timing information
      muonSegment.t0 = 0;
      if ( dtseg ) {
        if ( (dtseg->hasPhi()) && (! isDTWithoutY) ) {
  	  int phiHits = dtseg->phiSegment()->specificRecHits().size();
	  //	  int zHits = dtseg->zSegment()->specificRecHits().size();
	  int hits=0;
	  double t0=0.;
	  // TODO: cuts on hit numbers not optimized in any way yet...
	  if (phiHits>5 && dtseg->phiSegment()->ist0Valid()) {
	    t0+=dtseg->phiSegment()->t0()*phiHits;
	    hits+=phiHits;
	    LogTrace("TrackAssociator") << " Phi t0: " << dtseg->phiSegment()->t0() << " hits: " << phiHits;
	  }
	  // the z segments seem to contain little useful information...
//	  if (zHits>3) {
//	    t0+=s->zSegment()->t0()*zHits;
//	    hits+=zHits;
//	    std::cout << "   Z t0: " << s->zSegment()->t0() << " hits: " << zHits << std::endl;
//	  }
	  if (hits) muonSegment.t0 = t0/hits;
//	  std::cout << " --- t0: " << muonSegment.t0 << std::endl;
        } else {
           // check and set dimensionality
           if (isDTWithoutY) muonSegment.hasZed = false;
           if (! dtseg->hasPhi()) muonSegment.hasPhi = false;
        }
      }
      matchedChamber.segments.push_back(muonSegment);
   }

   return isGood;
}

//********************** NON-CORE CODE ******************************//

void TrackDetectorAssociator::fillCaloTruth( const edm::Event& iEvent,
					     TrackDetMatchInfo& info,
					     const AssociatorParameters& parameters)
{
   // get list of simulated tracks and their vertices
   using namespace edm;
   Handle<SimTrackContainer> simTracks;
   iEvent.getByToken(parameters.simTracksToken, simTracks);
   if (! simTracks.isValid() ) throw cms::Exception("FatalError") << "No simulated tracks found\n";
   
   Handle<SimVertexContainer> simVertices;
   iEvent.getByToken(parameters.simVerticesToken, simVertices);
   if (! simVertices.isValid() ) throw cms::Exception("FatalError") << "No simulated vertices found\n";
   
   // get sim calo hits
   Handle<PCaloHitContainer> simEcalHitsEB;
   iEvent.getByToken(parameters.simEcalHitsEBToken, simEcalHitsEB);
   if (! simEcalHitsEB.isValid() ) throw cms::Exception("FatalError") << "No simulated ECAL EB hits found\n";

   Handle<PCaloHitContainer> simEcalHitsEE;
   iEvent.getByToken(parameters.simEcalHitsEEToken, simEcalHitsEE);
   if (! simEcalHitsEE.isValid() ) throw cms::Exception("FatalError") << "No simulated ECAL EE hits found\n";

   Handle<PCaloHitContainer> simHcalHits;
   iEvent.getByToken(parameters.simHcalHitsToken, simHcalHits);
   if (! simHcalHits.isValid() ) throw cms::Exception("FatalError") << "No simulated HCAL hits found\n";

   // find truth partner
   SimTrackContainer::const_iterator simTrack = simTracks->begin();
   for( ; simTrack != simTracks->end(); ++simTrack){
      math::XYZVector simP3( simTrack->momentum().x(), simTrack->momentum().y(), simTrack->momentum().z() );
      math::XYZVector recoP3( info.stateAtIP.momentum().x(), info.stateAtIP.momentum().y(), info.stateAtIP.momentum().z() );
      if ( ROOT::Math::VectorUtil::DeltaR(recoP3, simP3) < 0.1 ) break;
   }
   if ( simTrack != simTracks->end() ) {
      info.simTrack = &(*simTrack);
      double ecalTrueEnergy(0);
      double hcalTrueEnergy(0);
      
      // loop over calo hits
      for( PCaloHitContainer::const_iterator hit = simEcalHitsEB->begin(); hit != simEcalHitsEB->end(); ++hit )
	if ( hit->geantTrackId() == info.simTrack->genpartIndex() ) ecalTrueEnergy += hit->energy();
      
      for( PCaloHitContainer::const_iterator hit = simEcalHitsEE->begin(); hit != simEcalHitsEE->end(); ++hit )
	if ( hit->geantTrackId() == info.simTrack->genpartIndex() ) ecalTrueEnergy += hit->energy();
      
      for( PCaloHitContainer::const_iterator hit = simHcalHits->begin(); hit != simHcalHits->end(); ++hit )
	if ( hit->geantTrackId() == info.simTrack->genpartIndex() ) hcalTrueEnergy += hit->energy();
      
      info.ecalTrueEnergy = ecalTrueEnergy;
      info.hcalTrueEnergy = hcalTrueEnergy;
      info.hcalTrueEnergyCorrected = hcalTrueEnergy;
      if ( fabs(info.trkGlobPosAtHcal.eta()) < 1.3 )
	info.hcalTrueEnergyCorrected = hcalTrueEnergy*113.2;
      else 
	if ( fabs(info.trkGlobPosAtHcal.eta()) < 3.0 )
	  info.hcalTrueEnergyCorrected = hcalTrueEnergy*167.2;
   }
}

TrackDetMatchInfo TrackDetectorAssociator::associate( const edm::Event& iEvent,
						      const edm::EventSetup& iSetup,
						      const reco::Track& track,
						      const AssociatorParameters& parameters,
						      Direction direction /*= Any*/ )
{
   double currentStepSize = cachedTrajectory_.getPropagationStep();
   
   edm::ESHandle<MagneticField> bField;
   iSetup.get<IdealMagneticFieldRecord>().get(bField);

   if(track.extra().isNull()) {
      if ( direction != InsideOut ) 
	throw cms::Exception("FatalError") << 
	"No TrackExtra information is available and association is done with something else than InsideOut track.\n" <<
	"Either change the parameter or provide needed data!\n";
     LogTrace("TrackAssociator") << "Track Extras not found\n";
     FreeTrajectoryState initialState = trajectoryStateTransform::initialFreeState(track,&*bField);
     return associate(iEvent, iSetup, parameters, &initialState); // 5th argument is null pointer
   }
   
   LogTrace("TrackAssociator") << "Track Extras found\n";
   FreeTrajectoryState innerState = trajectoryStateTransform::innerFreeState(track,&*bField);
   FreeTrajectoryState outerState = trajectoryStateTransform::outerFreeState(track,&*bField);
   FreeTrajectoryState referenceState = trajectoryStateTransform::initialFreeState(track,&*bField);
   
   LogTrace("TrackAssociator") << "inner track state (rho, z, phi):" << 
     track.innerPosition().Rho() << ", " << track.innerPosition().z() <<
     ", " << track.innerPosition().phi() << "\n";
   LogTrace("TrackAssociator") << "innerFreeState (rho, z, phi):" << 
     innerState.position().perp() << ", " << innerState.position().z() <<
     ", " << innerState.position().phi() << "\n";
   
   LogTrace("TrackAssociator") << "outer track state (rho, z, phi):" << 
     track.outerPosition().Rho() << ", " << track.outerPosition().z() <<
     ", " << track.outerPosition().phi() << "\n";
   LogTrace("TrackAssociator") << "outerFreeState (rho, z, phi):" << 
     outerState.position().perp() << ", " << outerState.position().z() <<
     ", " << outerState.position().phi() << "\n";
   
   // InsideOut first
   if ( crossedIP( track ) ) {
      switch ( direction ) {
       case InsideOut:
       case Any:
	 return associate(iEvent, iSetup, parameters, &referenceState, &outerState);
	 break;
       case OutsideIn:
	   {
	      cachedTrajectory_.setPropagationStep( -fabs(currentStepSize) );
	      TrackDetMatchInfo result = associate(iEvent, iSetup, parameters, &innerState, &referenceState);
	      cachedTrajectory_.setPropagationStep( currentStepSize );
	      return result;
	      break;
	   }
      }
   } else {
      switch ( direction ) {
       case InsideOut:
	 return associate(iEvent, iSetup, parameters, &innerState, &outerState);
	 break;
       case OutsideIn:
	   {
	      cachedTrajectory_.setPropagationStep( -fabs(currentStepSize) );
	      TrackDetMatchInfo result = associate(iEvent, iSetup, parameters, &outerState, &innerState);
	      cachedTrajectory_.setPropagationStep( currentStepSize );
	      return result;
	      break;
	   }
       case Any:
	   {
	      // check if we deal with clear outside-in case
	      if ( track.innerPosition().Dot( track.innerMomentum() ) < 0 &&
		   track.outerPosition().Dot( track.outerMomentum() ) < 0 )
		{
		   cachedTrajectory_.setPropagationStep( -fabs(currentStepSize) );
		   TrackDetMatchInfo result;
		   if ( track.innerPosition().R() < track.outerPosition().R() )
		     result = associate(iEvent, iSetup, parameters, &innerState, &outerState);
		   else
		     result = associate(iEvent, iSetup, parameters, &outerState, &innerState);
		   cachedTrajectory_.setPropagationStep( currentStepSize );
		   return result;
		}
	   }
      }
   }
	
   // all other cases  
   return associate(iEvent, iSetup, parameters, &innerState, &outerState);
}

TrackDetMatchInfo TrackDetectorAssociator::associate( const edm::Event& iEvent,
						      const edm::EventSetup& iSetup,
						      const SimTrack& track,
						      const SimVertex& vertex,
						      const AssociatorParameters& parameters)
{
   return associate(iEvent, iSetup, getFreeTrajectoryState(iSetup, track, vertex), parameters);
}

TrackDetMatchInfo TrackDetectorAssociator::associate( const edm::Event& iEvent,
						      const edm::EventSetup& iSetup,
						      const GlobalVector& momentum,
						      const GlobalPoint& vertex,
						      const int charge,
						      const AssociatorParameters& parameters)
{
   return associate(iEvent, iSetup, getFreeTrajectoryState(iSetup, momentum, vertex, charge), parameters);
}

bool TrackDetectorAssociator::crossedIP( const reco::Track& track )
{
   bool crossed = true;
   crossed &= (track.innerPosition().rho() > 3 ); // something close to active volume
   crossed &= (track.outerPosition().rho() > 3 ); // something close to active volume
   crossed &=  ( ( track.innerPosition().x()*track.innerMomentum().x() +
		   track.innerPosition().y()*track.innerMomentum().y() < 0 ) !=
		 ( track.outerPosition().x()*track.outerMomentum().x() + 
		   track.outerPosition().y()*track.outerMomentum().y() < 0 ) );
   return crossed;
}
