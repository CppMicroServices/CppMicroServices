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

#ifndef METADATAPARSERFACTORY_HPP
#define METADATAPARSERFACTORY_HPP

#include "MetadataParserImpl.hpp"

namespace cppmicroservices {
  namespace cmimpl {
    namespace metadata {
      /*
       * Houses a factory member function to return a @c MetadataParser
       */
      class MetadataParserFactory final
      {
      public:
        /*
         * @brief returns a @c MetadataParser object
         * @param version The version of the metadataparser to be parsed
         * @param logger A @c LogService object
         * @returns an unique_ptr to the created @c MetadataParser object
         * @throws std::runtime_error if the version isn't supported
         */
        static std::unique_ptr<MetadataParser> Create(uint64_t version, std::shared_ptr<cppmicroservices::logservice::LogService> logger)
        {
          switch (version)
          {
            case 1:
              return std::make_unique<MetadataParserImplV1>(logger);
            default:
              throw std::runtime_error("Unsupported manifest file version '" + std::to_string(version) + "'");
              return nullptr;
          }
        }
      };
    } // metadata
  } // cmimpl
} // cppmicroservices

#endif //METADATAPARSERFACTORY_HPP

