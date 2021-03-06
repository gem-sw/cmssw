#ifndef GEMDigi_ME0DigiPreReco_h
#define GEMDigi_ME0DigiPreReco_h

/** \class ME0DigiPreReco
 *
 * Digi for ME0
 *  
 * \author Marcello Maggi
 *
 */

#include <boost/cstdint.hpp>
#include <iosfwd>

class ME0DigiPreReco{

public:
//  explicit ME0DigiPreReco (float x, float y, float ex, float ey, float corr, float tof);
  explicit ME0DigiPreReco (float x, float y, float ex, float ey, float corr, float tof, int pdgid);
  ME0DigiPreReco ();

  bool operator==(const ME0DigiPreReco& digi) const;
  bool operator!=(const ME0DigiPreReco& digi) const;
  bool operator<(const ME0DigiPreReco& digi) const;

  float x() const { return x_; }
  float y() const { return y_; }
  float ex() const { return ex_; }
  float ey() const { return ey_; }
  float corr() const { return corr_; }
  float tof() const { return tof_;}
//cesare changes
  int pdgid() const { return pdgid_;}
  void print() const;

private:
  float x_;
  float y_;
  float ex_;
  float ey_;
  float corr_;
  float tof_;
//cesare changes
  int pdgid_;
};

std::ostream & operator<<(std::ostream & o, const ME0DigiPreReco& digi);

#endif

