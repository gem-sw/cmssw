#ifndef GEMEMAPSOURCEHANDLER
#define GEMEMAPSOURCEHANDLER

#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>

#include "FWCore/Framework/interface/MakerMacros.h"
#include "CondCore/PopCon/interface/PopConSourceHandler.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"

#include "CondFormats/GEMObjects/interface/GEMEMap.h"
#include "CondFormats/DataRecord/interface/GEMEMapRcd.h"

/*
//#include "CondCore/PopCon/interface/LogReader.h"
#include "CondCore/DBCommon/interface/DbTransaction.h"
#include "CondCore/DBCommon/interface/DbSession.h"
#include "CondCore/DBCommon/interface/DbConnection.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
*/

namespace popcon
{
	class GEMEMapSourceHandler : public popcon::PopConSourceHandler<GEMEMap>
	{

        public:

          GEMEMapSourceHandler(const edm::ParameterSet& ps);
          ~GEMEMapSourceHandler();
          void getNewObjects();
          std::string id() const {return m_name;};
      
        private:
      
          GEMEMap * eMap;
          std::string m_name;
          int m_dummy;
          int m_validate;
          std::string m_connect;
          std::string m_authpath;
      
      	};
}
#endif
