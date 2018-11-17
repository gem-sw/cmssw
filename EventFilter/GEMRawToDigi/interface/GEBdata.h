#ifndef EventFilter_GEMRawToDigi_GEBdata_h
#define EventFilter_GEMRawToDigi_GEBdata_h
#include "VFATdata.h"
#include <vector>

namespace gem {
  // Input status 1 bit for each
  // BX mismatch GLIB OH / BX mismatch GLIB VFAT / OOS GLIB OH / OOS GLIB VFAT / No VFAT marker
  // Event size warn / L1AFIFO near full / InFIFO near full / EvtFIFO near full / Event size overflow
  // L1AFIFO full / InFIFO full / EvtFIFO full
  union GEBchamberHeader {      
    uint64_t word;
    struct {
      uint64_t inputStatus     : 23; // Input status (critical) only first 13 bits used
      uint64_t vfatWordCnt     : 12; // Size of VFAT payload in 64bit words expected to send to AMC13
      uint64_t inputID         : 5 ; // GLIB input ID (starting at 0)
      uint64_t zeroSupWordsCnt : 24; // Bitmask indicating if certain VFAT blocks have been zero suppressed
    };
  };
  union GEBchamberTrailer {      
    uint64_t word;
    struct {
      uint64_t ecOH         : 20; // OH event counter
      uint64_t bcOH         : 14; // OH bunch crossing
      uint64_t stuckData    : 1;  // Input status (warning): There was data in InFIFO or EvtFIFO when L1A FIFO was empty
      uint64_t inFIFOund    : 1;  // Input status (critical): Input FIFO underflow occurred while sending this event
      uint64_t vfatWordCntT : 12; // Size of VFAT payload in 64bit words sent to AMC13 
      uint64_t ohcrc        : 16; // CRC of OH data (currently not available – filled with 0)
    };
  };

  class GEBdata
  {
  public:
    
    GEBdata() {};
    ~GEBdata(){vfatd_.clear();}

    //!Read chamberHeader from the block.
    void setChamberHeader(uint64_t word) { ch_.word = word;}
    uint64_t getChamberHeader() const { return ch_.word;}

    //!Read chamberTrailer from the block.
    void setChamberTrailer(uint64_t word) { ct_.word = word;}
    uint64_t getChamberTrailer() const { return ct_.word;}

    uint32_t inputStatus()     const {return ch_.inputStatus;}
    uint16_t vfatWordCnt()     const {return ch_.vfatWordCnt;}
    uint8_t  inputID()         const {return ch_.inputID;}
    uint16_t zeroSupWordsCnt() const {return ch_.zeroSupWordsCnt;}

    uint32_t ecOH()            const {return ct_.ecOH;}
    uint16_t bcOH()            const {return ct_.bcOH;}
    uint8_t  stuckData()       const {return ct_.stuckData;}
    uint8_t  inFIFOund()       const {return ct_.inFIFOund;}
    uint16_t vfatWordCntT()    const {return ct_.vfatWordCntT;}
    uint16_t ohcrc()           const {return ct_.ohcrc;}

    void setVfatWordCnt(uint16_t n){ch_.vfatWordCnt = n; ct_.vfatWordCntT = n;}
    void setInputID(uint8_t n){ch_.inputID = n;}

    //!Adds VFAT data to the vector
    void addVFAT(VFATdata v){vfatd_.push_back(v);}
    //!Returns the vector of FVAT data
    const std::vector<VFATdata> * vFATs() const {return &vfatd_;}  

    static const int sizeGebID = 5;
    
  private:
    std::vector<VFATdata> vfatd_;     ///<Vector of VFAT data
    GEBchamberHeader ch_;
    GEBchamberTrailer ct_;
  };
}
#endif
