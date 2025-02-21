/*
*
* Copyright 2022 FIWARE Foundation e.V.
*
* This file is part of Orion-LD Context Broker.
*
* Orion-LD Context Broker is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Orion-LD Context Broker is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
* General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Orion-LD Context Broker. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by this license please contact with
* orionld at fiware dot org
*
* Author: Ken Zangelin
*/
extern "C"
{
#include "kjson/KjNode.h"                                        // KjNode
#include "kjson/kjClone.h"                                       // kjClone
#include "kjson/kjLookup.h"                                      // kjLookup
#include "kjson/kjBuilder.h"                                     // kjInteger, kjFloat, kjString, kjChildAdd, ...
}

#include "logMsg/logMsg.h"                                       // LM_*

#include "orionld/types/OrionldTenant.h"                         // OrionldTenant
#include "orionld/types/RegistrationMode.h"                      // registrationMode
#include "orionld/regCache/RegCache.h"                           // RegCache, RegCacheItem
#include "orionld/regCache/regCacheIdPatternRegexCompile.h"      // regCacheIdPatternRegexCompile
#include "orionld/forwarding/FwdOperation.h"                     // fwdOperationMask
#include "orionld/kjTree/kjTreeLog.h"                            // kjTreeLog
#include "orionld/regCache/regCacheItemAdd.h"                    // Own interface



// -----------------------------------------------------------------------------
//
// regCounterAdd -
//
static void regCounterAdd(KjNode* regP, const char* name)
{
  KjNode* nodeP;

  nodeP = kjLookup(regP, name);
  if (nodeP == NULL)
  {
    nodeP = kjInteger(NULL, name, 0);
    kjChildAdd(regP, nodeP);
  }
}



// -----------------------------------------------------------------------------
//
// regTimestampAdd -
//
static void regTimestampAdd(KjNode* regP, const char* name)
{
  KjNode* nodeP;

  nodeP = kjLookup(regP, name);
  if (nodeP == NULL)
  {
    nodeP = kjFloat(NULL, name, 0);
    kjChildAdd(regP, nodeP);
  }
}



// -----------------------------------------------------------------------------
//
// regStringAdd -
//
static void regStringAdd(KjNode* regP, const char* name, const char* value)
{
  KjNode* nodeP;

  nodeP = kjLookup(regP, name);
  if (nodeP == NULL)
  {
    nodeP = kjString(NULL, name, value);
    kjChildAdd(regP, nodeP);
  }
}



// -----------------------------------------------------------------------------
//
// regCacheItemAdd -
//
RegCacheItem* regCacheItemAdd(RegCache* rcP, KjNode* regP, bool fromDb)
{
  RegCacheItem* rciP   = (RegCacheItem*) calloc(1, sizeof(RegCacheItem));

  //
  // Insert the new RegCacheItem LAST in rcP's linked list of registrations
  // MUST BE INSERTED LAST. If not, pagination doesn't work with the registration cache!!!
  //
  if (rcP->last == NULL)
    rcP->regList = rciP;
  else
    rcP->last->next = rciP;

  rcP->last = rciP;

  rciP->regTree = kjClone(NULL, regP);

  // Counters and timestamps - create if they don't exist
  if (fromDb == false)
  {
    regCounterAdd(rciP->regTree,   "timesSent");
    regCounterAdd(rciP->regTree,   "timesFailed");
    regTimestampAdd(rciP->regTree, "lastSuccess");
    regTimestampAdd(rciP->regTree, "lastFailure");
    regStringAdd(rciP->regTree,    "status", "active");
  }

  //
  // Some fields are "mirrored" inside RegCacheItem, for faster access
  // FIXME: Make a function of these 7 lines (it will grow)
  //        The exact same thing is done in orionldPatchRegistration
  //
  KjNode* operationsP  = kjLookup(rciP->regTree, "operations");
  KjNode* modeP        = kjLookup(rciP->regTree, "mode");
  KjNode* informationP = kjLookup(rciP->regTree, "information");

  rciP->opMask  = fwdOperationMask(operationsP);
  rciP->mode    = (modeP != NULL)? registrationMode(modeP->value.s) : RegModeInclusive;

  if (regCacheIdPatternRegexCompile(rciP, informationP) == false)
    LM_X(1, ("Internal Error (if this happens it's a bug of Orion-LD - the idPattern was checked in pcheckEntityInfo and all OK"));

  return rciP;
}
