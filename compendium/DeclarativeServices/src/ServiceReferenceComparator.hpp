/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  =============================================================================*/

#ifndef __SERVICEREFERENCECOMPARATOR_HPP__
#define __SERVICEREFERENCECOMPARATOR_HPP__

#include "cppmicroservices/Constants.h"

using cppmicroservices::Constants::SERVICE_RANKING;
using cppmicroservices::Constants::SERVICE_ID;

namespace cppmicroservices {
namespace scrimpl {

/**
 * This functor is used to sort a container of ServiceReferences. The comparator implements the "<"
 * function such that a reference with lower ranking is less than a reference with higher ranking.
 * In case both references have the same ranking, the reference with higher id is less than the
 * reference with lower id.
 */
class ServiceReferenceComparator
{
public:
  bool operator() (const ServiceReferenceU& lhs,
                   const ServiceReferenceU& rhs)
  {
    auto lRankAny = lhs.GetProperty(SERVICE_RANKING);
    auto rRankAny = rhs.GetProperty(SERVICE_RANKING);
    int lRank = lRankAny.Empty() ? 0 : any_cast<int>(lRankAny);
    int rRank = rRankAny.Empty() ? 0 : any_cast<int>(rRankAny);
    if(lRank == rRank)
    {
      auto lId = any_cast<long>(lhs.GetProperty(SERVICE_ID));
      auto rId = any_cast<long>(rhs.GetProperty(SERVICE_ID));
      return lId > rId;
    }
    else
    {
      return lRank < rRank;
    }
  };
};
}
}


#endif /* __SERVICEREFERENCECOMPARATOR_HPP__ */
