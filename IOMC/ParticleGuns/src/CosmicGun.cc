
#include <ostream>

#include "IOMC/ParticleGuns/interface/CosmicGun.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "CLHEP/Random/RandFlat.h"

using namespace edm;
using namespace std;

CosmicGun::CosmicGun(const ParameterSet& pset) : BaseFlatGunProducer(pset)
{
   ParameterSet defpset;
   ParameterSet pgun_params = pset.getParameter<ParameterSet>("PGunParameters");
  
   fMinPt = pgun_params.getParameter<double>("MinPt");
   fMaxPt = pgun_params.getParameter<double>("MaxPt");
  
  produces<HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();
}

CosmicGun::~CosmicGun()
{
   // no need to cleanup GenEvent memory - done in HepMCProduct
}

bool myIsMuonPassScint(double dVx, double dVy, double dVz, double dPx, double dPy, double dPz) {

  double ScintilXMin = -1000.0;
  double ScintilXMax =  1000.0;
  double ScintilYMin = -605.6;
  double ScintilYMax =  950.0;
  
  double ScintilLowerZ = -114.85;
  double ScintilUpperZ = 1540.15;
  
  double dTLower = ( ScintilLowerZ - dVz ) / dPz;  
  double dXLower = dVx + dTLower * dPx;
  double dYLower = dVy + dTLower * dPy;
  
  double dTUpper = ( ScintilUpperZ - dVz ) / dPz;
  double dXUpper = dVx + dTUpper * dPx;
  double dYUpper = dVy + dTUpper * dPy;
  
  if (( ScintilXMin <= dXLower && dXLower <= ScintilXMax && ScintilYMin <= dYLower && dYLower <= ScintilYMax ) &&
      ( ScintilXMin <= dXUpper && dXUpper <= ScintilXMax && ScintilYMin <= dYUpper && dYUpper <= ScintilYMax ))
  {
    return true;
  }
  
  else return false;
}

void CosmicGun::produce(Event &e, const EventSetup& es) 
{
   edm::Service<edm::RandomNumberGenerator> rng;
   CLHEP::HepRandomEngine* engine = &rng->getEngine(e.streamID());

   if ( fVerbosity > 0 )
   {
      cout << " CosmicGun : Begin New Event Generation" << endl ; 
   }
   
   fEvt = new HepMC::GenEvent() ;
   
   double dVx;
   double dVy;
   double dVz = 1540.15; // same Y as the upper scintillator
   HepMC::GenVertex* Vtx = NULL;

   // loop over particles

   int barcode = 1 ;
   for (unsigned int ip=0; ip<fPartIDs.size(); ++ip)
   {
       double px, py, pz, mom;
       double phi, theta;
       int j = 0;
       
       while (j < 10000) // j < 10000 to avoid too long computational time
       {
         dVx = CLHEP::RandFlat::shoot(engine, -1000.0, 1000.0) ;
         dVy = CLHEP::RandFlat::shoot(engine, -605.6, 950.0) ;
         
         mom   = CLHEP::RandFlat::shoot(engine, fMinPt, fMaxPt) ;
         phi   = CLHEP::RandFlat::shoot(engine, fMinPhi, fMaxPhi) ;
         theta = 0;

         if (fIsThetaFlat)
         {
             theta  = CLHEP::RandFlat::shoot(engine, fMinTheta, fMaxTheta);
         }
         
         if (!fIsThetaFlat)
         {
             double u = CLHEP::RandFlat::shoot(engine, 0.0, 0.785398); // u = Uniform[0;Pi/4]
             theta = 0;
             while(abs(u-(0.5*theta+0.25*sin(2*theta)))>0.000015)
             {
                 theta+=0.00001;
             }             
         }

         px     =  mom*sin(theta)*cos(phi) ;
         py     =  mom*sin(theta)*sin(phi) ;
         pz     =  mom*cos(theta) ;

         if ( myIsMuonPassScint(dVx, dVy, dVz, px, py, pz) == true ) break; // muon passing through both the scintillators => valid: the loop can be stopped
         
         j++;
         
       }
       
       int PartID = fPartIDs[ip] ;
       const HepPDT::ParticleData* 
       PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID))) ;
       double mass   = PData->mass().value() ;
       Vtx = new HepMC::GenVertex(HepMC::FourVector(dVx,dVy,dVz));

       double energy2= mom*mom + mass*mass ;
       double energy = sqrt(energy2) ;
       HepMC::FourVector p(px,py,pz,energy) ;
       HepMC::GenParticle* Part = new HepMC::GenParticle(p,PartID,1);
       Part->suggest_barcode( barcode ) ;
       barcode++ ;
       Vtx->add_particle_out(Part);

       if ( fAddAntiParticle )
       {
          HepMC::FourVector ap(-px,-py,-pz,energy) ;
	      int APartID = -PartID ;
	      if ( PartID == 22 || PartID == 23 )
	      {
	        APartID = PartID ;
	      }	  
	      HepMC::GenParticle* APart = new HepMC::GenParticle(ap,APartID,1);
	      APart->suggest_barcode( barcode ) ;
	      barcode++ ;
	      Vtx->add_particle_out(APart) ;
       }
   }

   fEvt->add_vertex(Vtx) ;
   fEvt->set_event_number(e.id().event()) ;
   fEvt->set_signal_process_id(20) ; 
        
   if ( fVerbosity > 0 )
   {
      fEvt->print() ;  
   }

   unique_ptr<HepMCProduct> BProduct(new HepMCProduct()) ;
   BProduct->addHepMCData( fEvt );
   e.put(std::move(BProduct), "unsmeared");

   unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
   e.put(std::move(genEventInfo));
    
   if ( fVerbosity > 0 )
   {
      cout << " CosmicGun : Event Generation Done " << endl;
   }
}
