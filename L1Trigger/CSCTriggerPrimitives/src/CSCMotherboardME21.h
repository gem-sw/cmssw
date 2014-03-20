#ifndef CSCTriggerPrimitives_CSCMotherboardME21_h
#define CSCTriggerPrimitives_CSCMotherboardME21_h

/** \class CSCMotherboardME11
 *
 * Extended CSCMotherboardME21 for ME21 TMB upgrade
 *
 * \author Sven Dildick March 2014
 *
 * Based on CSCMotherboardME21 code
 *
 */

#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboard.h>
#include <DataFormats/GEMDigi/interface/GEMCSCPadDigiCollection.h>

class CSCGeometry;
class CSCChamber;
class GEMGeometry;
class GEMSuperChamber;

class CSCMotherboardME21 : public CSCMotherboard
{
  typedef std::map<int, std::vector<std::pair<unsigned int, const GEMCSCPadDigi*> > > GEMPads;
  typedef std::pair<unsigned int, const GEMCSCPadDigi*> GEMPadBX;
  typedef std::vector<GEMPadBX> GEMPadsBX;
  // roll, pad, isCopad?
  typedef std::vector<std::tuple<unsigned int, const GEMCSCPadDigi*, bool> > GEMPadsBXGeneral;

 public:
  /** Normal constructor. */
  CSCMotherboardME21(unsigned endcap, unsigned station, unsigned sector, 
		 unsigned subsector, unsigned chamber,
		 const edm::ParameterSet& conf);

  /** Default destructor. */
  ~CSCMotherboardME21();

  /** Run function for normal usage.  Runs cathode and anode LCT processors,
      takes results and correlates into CorrelatedLCT. */
  void run(const CSCWireDigiCollection* wiredc, 
           const CSCComparatorDigiCollection* compdc, 
           const GEMCSCPadDigiCollection* gemPads);

  /// set CSC and GEM geometries for the matching needs
  void setCSCGeometry(const CSCGeometry *g) { csc_g = g; }
  void setGEMGeometry(const GEMGeometry *g) { gem_g = g; }

  void buildCoincidencePads(const GEMCSCPadDigiCollection* out_pads, 
                            GEMCSCPadDigiCollection& out_co_pads);

  void retrieveGEMPads(const GEMCSCPadDigiCollection* pads, unsigned id, bool iscopad = false);

  std::map<int,std::pair<double,double> > createGEMPadLUT(bool isEven, bool isLong);

  int assignGEMRoll(double eta);
  int deltaRoll(int wg, int roll);
  int deltaPad(int hs, int pad);

  void printGEMTriggerPads(int minBX, int maxBx, bool iscopad = false);

  GEMPadsBX matchingGEMPads(const CSCCLCTDigi& cLCT, const GEMPadsBX& pads = GEMPadsBX(), 
                            bool isCopad = false, bool first = true);  
  GEMPadsBX matchingGEMPads(const CSCALCTDigi& aLCT, const GEMPadsBX& pads = GEMPadsBX(), 
                            bool isCopad = false, bool first = true);  
  GEMPadsBX matchingGEMPads(const CSCCLCTDigi& cLCT, const CSCALCTDigi& aLCT, const GEMPadsBX& pads = GEMPadsBX(), 
                            bool isCopad = false, bool first = true);  

 private: 

  const CSCGeometry* csc_g;
  const GEMGeometry* gem_g;

  std::vector<CSCALCTDigi> alctV;
  std::vector<CSCCLCTDigi> clctV;

  // central LCT bx number
  int lct_central_bx;

  bool runUpgradeME21_;

  /** whether to not reuse CLCTs that were used by previous matching ALCTs
      in ALCT-to-CLCT algorithm */
  bool drop_used_clcts;

  //  deltas used to construct GEM coincidence pads
  int maxDeltaBXInCoPad_;
  int maxDeltaRollInCoPad_;
  int maxDeltaPadInCoPad_;

  //  deltas used to match to GEM pads
  int maxDeltaBXPad_;
  int maxDeltaRollPad_;
  int maxDeltaPadPad_;

  //  deltas used to match to GEM coincidence pads
  int maxDeltaBXCoPad_;
  int maxDeltaRollCoPad_;
  int maxDeltaPadCoPad_;

  std::map<int,std::pair<double,double> > gemPadToEtaLimitsShort_;
  std::map<int,std::pair<double,double> > gemPadToEtaLimitsLong_;
  

/*   void correlateLCTs(CSCALCTDigi bestALCT, CSCALCTDigi secondALCT, */
/*                      CSCCLCTDigi bestCLCT, CSCCLCTDigi secondCLCT); */

/*   CSCCorrelatedLCTDigi constructLCTs(const CSCALCTDigi& aLCT, */
/*                                      const CSCCLCTDigi& cLCT); */

/*   unsigned int encodePattern(const int ptn, const int highPt); */

/*   unsigned int findQuality(const CSCALCTDigi& aLCT, const CSCCLCTDigi& cLCT); */

  // map< bx , vector<gemid, pad> >
  GEMPads pads_;
  GEMPads coPads_;
};
#endif
