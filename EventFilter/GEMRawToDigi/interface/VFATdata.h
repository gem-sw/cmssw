#ifndef EventFilter_GEMRawToDigi_VFATdata_h
#define EventFilter_GEMRawToDigi_VFATdata_h

#include <cstdint>

namespace gem {
  /// VFAT data structure - 3 words of 64 bits each
  union VFATfirst{
    uint64_t word;
    // v3 dataformat
    struct {
      uint64_t msData1  : 16; ///<channels from 65to128
      uint64_t bc       : 16; ///<Bunch Crossing number, 16 bits
      uint64_t ec       : 8;  ///<Event Counter, 8 bits
      uint64_t header   : 8;  ///<normally 0x1E. 0x5E indicates that the VFAT3 internal buffer is half-full, so it's like a warning
      uint64_t crcCheck : 8;  ///<bits 183:177 are not used, should be 0, bit 176 is 1 if CTP7 detected a CRC mismatch
      uint64_t pos      : 8;  ///<an 8bit value indicating the VFAT position on this GEB (it can be 0 to 23)
    };
    // v2 dataformat
    struct {
      uint64_t msData1v2: 16; ///<channels from 65to128 - placeholder since msData1 reads same info
      uint64_t chipID   : 12; ///<Chip ID, 12 bits
      uint64_t b1110    : 4;  ///<1110:4 Control bits, shoud be 1110
      uint64_t flag     : 4;  ///<Control Flags: 4 bits, Hamming Error/AFULL/SEUlogic/SUEI2C
      uint64_t ecV2     : 8;  ///<Event Counter, 8 bits
      uint64_t b1100    : 4;  ///<1100:4, Control bits, shoud be 1100
      uint64_t bcV2     : 12; ///<Bunch Crossing number, 12 bits
      uint64_t b1010    : 4;  ///<1010:4 Control bits, shoud be 1010
    };
  };
  union VFATsecond{
    uint64_t word;
    struct {
      uint64_t lsData1  : 16; ///<channels from 1to64 
      uint64_t msData2  : 48; ///<channels from 65to128
    };
  };
  union VFATthird{
    uint64_t word;
    struct {
      uint64_t crc      : 16; ///<Check Sum value, 16 bits
      uint64_t lsData2  : 48; ///<channels from 1to64 
    };
  };
      
  class VFATdata
  {
  public:
    
    VFATdata();
    // this constructor only used for packing sim digis
    VFATdata(const uint16_t BC,
	     const uint8_t EC,
	     const uint8_t Pos,
	     const uint64_t lsDatas,
	     const uint64_t msDatas);
    ~VFATdata(){}

    //!Read first word from the block.
    void read_fw(uint64_t word) { fw_.word = word;}
    uint64_t get_fw() const { return fw_.word;}

    //!Read second word from the block.
    void read_sw(uint64_t word) { sw_.word = word;}
    uint64_t get_sw() const { return sw_.word;}

    //!Read third word from the block.
    void read_tw(uint64_t word) { tw_.word = word;}
    uint64_t get_tw() const { return tw_.word;}

    // local phi in chamber
    void setPhi(int i) {phiPos_ = i;}
    int phi() const {return phiPos_;}
    
    uint64_t lsData() const {
      return uint64_t(sw_.lsData1) << 48 | tw_.lsData2;
    }
    uint64_t msData() const {
      return uint64_t(fw_.msData1) << 48 | sw_.msData2;
    }
    
    uint16_t bc() const {
      if (ver_==2) return fw_.bcV2;
      return fw_.bc;
    }
    uint8_t  ec() const {
      if (ver_==2) return fw_.ecV2;
      return fw_.ec;
    }

    void setVersion(int i) {ver_ = i;}
    int version() const {return ver_;}

    /// quality flag - bit: 0 good, 1 crc fail, 2 b1010 fail, 3 b1100 fail, 4 b1110 
    uint8_t quality();

    /// v3
    uint8_t   header     () const {return fw_.header;      }
    uint8_t   crcCheck   () const {return fw_.crcCheck;    }
    uint8_t   position   () const {return fw_.pos;         }

    /// v2
    uint8_t   b1010      () const { return fw_.b1010;      }
    uint8_t   b1100      () const { return fw_.b1100;      }
    uint8_t   b1110      () const { return fw_.b1110;      }
    uint8_t   flag       () const { return fw_.flag;       }
    uint16_t  chipID     () const { return fw_.chipID;     }
    uint16_t  crc        () const { return tw_.crc;        }
    
    uint16_t crc_cal(uint16_t crc_in, uint16_t dato);    
    uint16_t checkCRC();
    
    static const int nChannels = 128;
    static const int sizeChipID = 12;
    
  private:
    
    int      ver_;        /// vfat version
    int      phiPos_;     /// phi position of vfat in chamber
    
    VFATfirst  fw_;
    VFATsecond sw_;
    VFATthird  tw_;
  };
}
#endif
