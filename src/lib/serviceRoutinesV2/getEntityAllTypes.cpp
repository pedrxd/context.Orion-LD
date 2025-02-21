/*
*
* Copyright 2015 Telefonica Investigacion y Desarrollo, S.A.U
*
* This file is part of Orion Context Broker.
*
* Orion Context Broker is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Orion Context Broker is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
* General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Orion Context Broker. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by this license please contact with
* iot_support at tid dot es
*
* Author: Ken Zangelin
*/
#include <string>
#include <vector>

#include "orionld/common/orionldState.h"                  // orionldState
#include "orionld/types/OrionldHeader.h"                  // orionldHeaderAdd

#include "common/statistics.h"
#include "common/clockFunctions.h"
#include "rest/ConnectionInfo.h"
#include "rest/HttpHeaders.h"                             // HTTP_*
#include "ngsi/ParseData.h"
#include "serviceRoutinesV2/getEntityAllTypes.h"
#include "orionTypes/EntityTypeVectorResponse.h"
#include "mongoBackend/mongoQueryTypes.h"



/* ****************************************************************************
*
* getEntityAllTypes -
*
* GET /v2/types
*
* Payload In:  None
* Payload Out: EntityTypeVectorResponse
*
* URI parameters:
*   - options=noAttrDetail
*
*/
std::string getEntityAllTypes
(
  ConnectionInfo*            ciP,
  int                        components,
  std::vector<std::string>&  compV,
  ParseData*                 parseDataP
)
{
  EntityTypeVectorResponse  response;
  std::string               answer;
  unsigned int              totalTypes   = 0;
  bool                      noAttrDetail = orionldState.uriParamOptions.noAttrDetail;
  unsigned int*             totalTypesP  = NULL;

  // NGSIv2 uses options=count to request count
  if (orionldState.uriParams.count)
  {
    totalTypesP = &totalTypes;
  }

  if (orionldState.uriParamOptions.values)
  {
    TIMED_MONGO(mongoEntityTypesValues(&response, orionldState.tenantP, ciP->servicePathV, totalTypesP));
  }
  else  // default
  {
    TIMED_MONGO(mongoEntityTypes(&response,
                                 orionldState.tenantP,
                                 ciP->servicePathV,
                                 orionldState.apiVersion,
                                 totalTypesP,
                                 noAttrDetail));
  }
  TIMED_RENDER(answer = response.toJson(orionldState.uriParamOptions.values));

  if (orionldState.uriParams.count)
    orionldHeaderAdd(&orionldState.out.headers, HttpNgsiv2Count, NULL, totalTypes);

  response.release();
  return answer;
}
