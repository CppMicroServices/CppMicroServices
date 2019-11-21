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

#ifndef METADATAPARSERIMPL_HPP
#define METADATAPARSERIMPL_HPP

#include "MetadataParser.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace scrimpl {
namespace metadata {
/*
 * Represents a concrete implementation (Version 1) of the MetadataParser
 */
class MetadataParserImplV1
  : public MetadataParser
{
public:
  MetadataParserImplV1(std::shared_ptr<cppmicroservices::logservice::LogService> logger)
    : logger(std::move(logger))
  {
  }

  MetadataParserImplV1(const MetadataParserImplV1&) = delete;
  MetadataParserImplV1& operator=(const MetadataParserImplV1&) = delete;
  MetadataParserImplV1(MetadataParserImplV1&&) = delete;
  MetadataParserImplV1& operator=(MetadataParserImplV1&&) = delete;

  ~MetadataParserImplV1() override = default;

  /*
   * @brief Parse and return the ServiceMetadata object
   * @param metadata The value of the key "service" in the manifest
   * @returns the @c ServiceMetadata object
   */
  ServiceMetadata CreateServiceMetadata(const AnyMap& metadata) const;

  /*
   * @brief Parse and return the vector of ReferenceMetadata object
   * @param refs A vector of references of type Any (i.e. the value of the
   *             key @c "references" in the manifest)
   * @returns a vector of @c ReferenceMetadata objects
   */
  std::vector<ReferenceMetadata>
  CreateReferenceMetadatas(const std::vector<cppmicroservices::Any>& refs) const;

  /*
   * @brief Parse and return the ReferenceMetadata object
   * @param metadata An array value of the key "references" in the manifest
   * @returns the @c ReferenceMetadata object
   */
  ReferenceMetadata CreateReferenceMetadata(const AnyMap& metadata) const;

  /*
   * @brief Parse and return the ComponentMetadata object
   * @param metadata An element in the array of the key "components" in the manifest
   * @returns the shared_ptr to a @c ComponentMetadata object
   */
  std::shared_ptr<ComponentMetadata> CreateComponentMetadata(const AnyMap& metadata) const;

  /*
   * @brief Parse and return the vector of ComponentMetadata's
   * @param metadata The value of the key "scr" in the manifest
   * @returns the vector of shared_ptrs to the created @ComponentMetadata objects
   */
  std::vector<std::shared_ptr<ComponentMetadata>> ParseAndGetComponentsMetadata(const AnyMap& scrmap) const override;

private:
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
};
}
}
}

#endif //METADATAPARSERIMPL_HPP
