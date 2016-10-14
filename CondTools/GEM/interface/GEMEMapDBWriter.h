#ifndef GEMEMAPDBWRITER
#define GEMEMAPDBWRITER

#include "CondCore/PopCon/interface/PopConAnalyzer.h"
#include "CondTools/GEM/interface/GEMEMapSourceHandler.h"


class GEMEMapDBWriter : public popcon::PopConAnalyzer<GEMEMap>
{
	public:
		GEMEMapDBWriter(const edm::ParameterSet&);
	private: 
		void initSource(const edm::Event& evt, const edm::EventSetup& est);
                int m_validate;
};


#endif
