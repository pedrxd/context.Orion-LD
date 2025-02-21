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
#include <string>                                              // std::string
#include <vector>                                              // std::vector

extern "C"
{
#include "kjson/KjNode.h"                                      // KjNode
#include "kjson/kjRender.h"                                    // kjFastRender
#include "kjson/kjRenderSize.h"                                // kjFastRenderSize
}

#include "logMsg/logMsg.h"                                     // LM_*

#include "orionld/common/orionldState.h"                       // orionldState



// -----------------------------------------------------------------------------
//
// kjTreeLogFunction -
//
void kjTreeLogFunction(KjNode* tree, const char* msg, const char* fileName, int lineNo)
{
  if (tree == NULL)
  {
    LM(("%s[%d]: %s: NULL Tree", fileName, lineNo, msg));
    return;
  }

  int bufSize = kjFastRenderSize(tree);

  // Too big trees will not be rendered - this is just tracing
  if (bufSize < 10 * 1024)
  {
    char* buf = kaAlloc(&orionldState.kalloc, bufSize + 512);

    if (buf != NULL)
    {
      bzero(buf, bufSize);
      kjFastRender(tree, buf);
      LM(("%s[%d]: %s: %s", fileName, lineNo, msg, buf));
    }
  }
  else
    LM(("%s[%d]: %s: %s", fileName, lineNo, msg, "Tree too big to be rendered"));
}
