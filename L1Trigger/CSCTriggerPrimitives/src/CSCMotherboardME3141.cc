#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboardME3141.h>
#include <FWCore/MessageLogger/interface/MessageLogger.h>
#include <DataFormats/MuonDetId/interface/CSCTriggerNumbering.h>
#include <L1Trigger/CSCCommonTrigger/interface/CSCTriggerGeometry.h>
#include <Geometry/RPCGeometry/interface/RPCGeometry.h>

CSCMotherboardME3141::CSCMotherboardME3141(unsigned endcap, unsigned station,
                               unsigned sector, unsigned subsector,
                               unsigned chamber,
                               const edm::ParameterSet& conf) :
  CSCMotherboard(endcap, station, sector, subsector, chamber, conf)
{
  edm::ParameterSet commonParams = conf.getParameter<edm::ParameterSet>("commonParam");
  
  if (!isSLHC) edm::LogError("L1CSCTPEmulatorConfigError")
    << "+++ Upgrade CSCMotherboardME3141 constructed while isSLHC is not set! +++\n";
  
  edm::ParameterSet alctParams = conf.getParameter<edm::ParameterSet>("alctSLHC");
  edm::ParameterSet clctParams = conf.getParameter<edm::ParameterSet>("clctSLHC");
  edm::ParameterSet tmbParams = conf.getParameter<edm::ParameterSet>("tmbSLHC");

  // central bx for LCT is 6 for simulation
  lct_central_bx = tmbParams.getUntrackedParameter<int>("lctCentralBX", 6);

  // upgrade algorithm
  runUpgradeME3141_ = tmbParams.getUntrackedParameter<bool>("runUpgradeME3141",false);
}

CSCMotherboardME3141::~CSCMotherboardME3141() 
{
}


void
CSCMotherboardME3141::run(const CSCWireDigiCollection* wiredc,
			  const CSCComparatorDigiCollection* compdc,
			  const RPCDigiCollection* rpcDigis) 
{
  clear();

  if (!( alct and clct and runUpgradeME3141_))
  {
    if (infoV >= 0) edm::LogError("L1CSCTPEmulatorSetupError")
      << "+++ run() called for non-existing ALCT/CLCT processor! +++ \n";
    return;
  }

  alct->run(wiredc); // run anodeLCT
  clct->run(compdc); // run cathodeLCT

  bool rpcGeometryAvailable(false);
  if (rpc_g != nullptr) {
    if (infoV >= 0) edm::LogInfo("L1CSCTPEmulatorSetupInfo")
      << "+++ run() called for RPC-CSC integrated trigger! +++ \n";
    rpcGeometryAvailable = true;
  }

  if (runUpgradeME3141_ and not rpcGeometryAvailable) {
    if (infoV >= 0) edm::LogError("L1CSCTPEmulatorSetupError")
      << "+++ run() called for RPC-CSC integrated trigger without valid RPC geometry! +++ \n";
    return;
  }

  // retrieve CSCChamber geometry                                                                                                                                       
  //CSCTriggerGeomManager* geo_manager(CSCTriggerGeometry::get());
  //CSCChamber* cscChamber(geo_manager->chamber(theEndcap, theStation, theSector, theSubsector, theTrigChamber));
  //CSCDetId csc_id(cscChamber->id());
  //const CSCLayer* keyLayer(cscChamber->layer(3));
  //const CSCLayerGeometry* keyLayerGeometry(keyLayer->geometry());

  int used_alct_mask[20];
  for (int a=0;a<20;++a) used_alct_mask[a]=0;
  
  int bx_alct_matched = 0; // bx of last matched ALCT
  for (int bx_clct = 0; bx_clct < CSCCathodeLCTProcessor::MAX_CLCT_BINS;
       bx_clct++) {
    // There should be at least one valid ALCT or CLCT for a
    // correlated LCT to be formed.  Decision on whether to reject
    // non-complete LCTs (and if yes of which type) is made further
    // upstream.
    if (clct->bestCLCT[bx_clct].isValid()) {
      // Look for ALCTs within the match-time window.  The window is
      // centered at the CLCT bx; therefore, we make an assumption
      // that anode and cathode hits are perfectly synchronized.  This
      // is always true for MC, but only an approximation when the
      // data is analyzed (which works fairly good as long as wide
      // windows are used).  To get rid of this assumption, one would
      // need to access "full BX" words, which are not readily
      // available.
      bool is_matched = false;
      int bx_alct_start = bx_clct - match_trig_window_size/2;
      int bx_alct_stop  = bx_clct + match_trig_window_size/2;
      // Empirical correction to match 2009 collision data (firmware change?)
      // (but don't do it for SLHC case, assume it would not be there)
      if (!isSLHC) bx_alct_stop += match_trig_window_size%2;
      
      for (int bx_alct = bx_alct_start; bx_alct <= bx_alct_stop; bx_alct++) {
	if (bx_alct < 0 || bx_alct >= CSCAnodeLCTProcessor::MAX_ALCT_BINS)
	  continue;
	// default: do not reuse ALCTs that were used with previous CLCTs
	if (drop_used_alcts && used_alct_mask[bx_alct]) continue;
	if (alct->bestALCT[bx_alct].isValid()) {
	  if (infoV > 1) LogTrace("CSCMotherboard")
	    << "Successful ALCT-CLCT match: bx_clct = " << bx_clct
	    << "; match window: [" << bx_alct_start << "; " << bx_alct_stop
	    << "]; bx_alct = " << bx_alct;
	  correlateLCTs(alct->bestALCT[bx_alct], alct->secondALCT[bx_alct],
			clct->bestCLCT[bx_clct], clct->secondCLCT[bx_clct]);
	  used_alct_mask[bx_alct] += 1;
	  is_matched = true;
	  bx_alct_matched = bx_alct;
	  break;
	}
      }
      // No ALCT within the match time interval found: report CLCT-only LCT
      // (use dummy ALCTs).
      if (!is_matched) {
	if (infoV > 1) LogTrace("CSCMotherboard")
	  << "Unsuccessful ALCT-CLCT match (CLCT only): bx_clct = "
	  << bx_clct << "; match window: [" << bx_alct_start
	  << "; " << bx_alct_stop << "]";
	correlateLCTs(alct->bestALCT[bx_clct], alct->secondALCT[bx_clct],
		      clct->bestCLCT[bx_clct], clct->secondCLCT[bx_clct]);
      }
    }
    // No valid CLCTs; attempt to make ALCT-only LCT.  Use only ALCTs
    // which have zeroth chance to be matched at later cathode times.
    // (I am not entirely sure this perfectly matches the firmware logic.)
    // Use dummy CLCTs.
    else {
      int bx_alct = bx_clct - match_trig_window_size/2;
      if (bx_alct >= 0 && bx_alct > bx_alct_matched) {
	if (alct->bestALCT[bx_alct].isValid()) {
	  if (infoV > 1) LogTrace("CSCMotherboard")
	    << "Unsuccessful ALCT-CLCT match (ALCT only): bx_alct = "
	    << bx_alct;
	  correlateLCTs(alct->bestALCT[bx_alct], alct->secondALCT[bx_alct],
			clct->bestCLCT[bx_clct], clct->secondCLCT[bx_clct]);
	}
      }
    }
  }
  
  if (infoV > 0) {
    for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
      if (firstLCT[bx].isValid())
	LogDebug("CSCMotherboard") << firstLCT[bx];
      if (secondLCT[bx].isValid())
	LogDebug("CSCMotherboard") << secondLCT[bx];
    }
  }
}
