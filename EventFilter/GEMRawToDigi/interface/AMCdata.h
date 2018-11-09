#ifndef EventFilter_GEMRawToDigi_AMCdata_h
#define EventFilter_GEMRawToDigi_AMCdata_h
#include "GEBdata.h"
#include <vector>

namespace gem {
  
  union AMCheader1 {
    uint64_t word;
    struct {
      uint64_t dataLength : 20; // Overall size of this FED event fragment 
      uint64_t bxID       : 12; // Bunch crossing ID
      uint64_t l1AID      : 24; // L1A number – basically this is like event number, but it’s reset by resync
      uint64_t AMCnum     : 4;  // Slot number of the AMC
      uint64_t reserved   : 4;  // not used
    };
  };
  union AMCheader2 {
    uint64_t word;
    struct {
      uint64_t boardID    : 16; // 8bit long GLIB serial number 
      uint64_t orbitNum   : 16;
      uint64_t param3     : 8;
      uint64_t param2     : 8;
      uint64_t param1     : 8;
      uint64_t runType    : 4;  // run types like physics, cosmics, threshold scan, latency scan, etc..
      uint64_t formatVer  : 4;  // Current format version = 0x0
    };
  };
  union AMCTrailer {
    uint64_t word;
    struct {
      uint64_t dataLengthT: 20; // Overall size of this FED event fragment 
      uint64_t l1AIDT     : 12; // 8bit long GLIB serial number (first 8 bits)
      uint64_t crc        : 32;
    };
  };
  union EventHeader {
    uint64_t word;
    struct {
      uint64_t ttsState   : 11; // GLIB TTS state at the moment when this event was built.
      uint64_t davCnt     : 5;  // Number of chamber blocks
      uint64_t buffState  : 24; // buffer error 
      uint64_t davList    : 24; // Bitmask indicating which inputs/chambers have data
    };
  };
  union EventTrailer {
    uint64_t word;
    struct {
      uint64_t oosGlib    : 40; // GLIB is out‐of‐sync (critical): L1A ID is different for different chambers in this event (1 bit)
      uint64_t chTimeOut  : 24; // GLIB did not receive data from a particular input for this L1A 
    };
  };

  class AMCdata
  {
    
  public:
  AMCdata() {};
    ~AMCdata(){gebd_.clear();}

    void setAMCheader1(uint64_t word) { amch1_.word = word;}
    uint64_t getAMCheader1() const { return amch1_.word;}

    void setAMCheader2(uint64_t word) { amch2_.word = word;}
    uint64_t getAMCheader2() const { return amch2_.word;}

    void setAMCTrailer(uint64_t word) { amct_.word = word;}
    uint64_t getAMCTrailer() const { return amct_.word;}

    void setGEMeventHeader(uint64_t word) { eh_.word = word;}
    uint64_t getGEMeventHeader() const { return eh_.word;}

    void setGEMeventTrailer(uint64_t word) { et_.word = word;}
    uint64_t getGEMeventTrailer() const { return et_.word;}

    uint32_t dataLength() const {return amch1_.dataLength;}
    uint16_t bx()         const {return amch1_.bxID;}
    uint32_t l1A()        const {return amch1_.l1AID;}
    uint8_t  amcNum()     const {return amch1_.AMCnum;}

    uint16_t boardId()    const {return amch2_.boardID;}
    uint16_t orbitNum()   const {return amch2_.orbitNum;}
    uint8_t  param3()     const {return amch2_.param3;}
    uint8_t  param2()     const {return amch2_.param2;}
    uint8_t  param1()     const {return amch2_.param1;}
    uint8_t  runType()    const {return amch2_.runType;}
    uint8_t  formatVer()  const {return amch2_.formatVer;}
    
    uint16_t ttsState()   const {return eh_.ttsState;}
    uint8_t  davCnt()     const {return eh_.davCnt;}
    uint32_t buffState()  const {return eh_.buffState;}
    uint32_t davList()    const {return eh_.davList;}

    uint8_t  oosGlib()    const {return et_.oosGlib;}
    uint32_t chTimeOut()  const {return et_.chTimeOut;}


    void setdataLength(uint64_t n) {amch1_.dataLength = n;}
    void setbx(uint64_t n) {amch1_.bxID = n;}
    void setl1A(uint64_t n) {amch1_.l1AID = n;}
    void setamcNum(uint64_t n) {amch1_.AMCnum = n;}
    
    void setboardId(uint64_t n) {amch2_.boardID = n;}
    void setorbitNum(uint64_t n) {amch2_.orbitNum = n;}
    void setrunType(uint64_t n) {amch2_.runType = n;}
    
    void setdavCnt(uint64_t n) {eh_.davCnt = n;}
    void setdavList(uint64_t n) {eh_.davList = n;}
    
    //!Adds GEB data to vector
    void addGEB(GEBdata g){gebd_.push_back(g);}
    //!Returns a vector of GEB data
    const std::vector<GEBdata> * gebs() const {return &gebd_;}
  
  private:
    std::vector<GEBdata> gebd_; ///<Vector of GEB data
    AMCheader1 amch1_;
    AMCheader2 amch2_;
    AMCTrailer amct_;
    EventHeader eh_;
    EventTrailer et_;
  };
}
#endif
