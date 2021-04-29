/*=============================================================================

 file at the top-level directory of this distribution and at
 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 limitations under the License.

 =============================================================================*/

#ifndef CM_METADATAPARSERIMPL_HPP
#define CM_METADATAPARSERIMPL_HPP

#include "cppmicroservices/logservice/LogService.hpp"

#include "MetadataParser.hpp"

namespace cppmicroservices {
  namespace util {
      /*
       * Represents a concrete implementation (Version 1) of the MetadataParser
       */
      class MetadataParserImplV1 final : public MetadataParser
      {
      public:
        MetadataParserImplV1(std::shared_ptr<cppmicroservices::logservice::LogService> logger);

        ~MetadataParserImplV1() override = default;
        MetadataParserImplV1(const MetadataParserImplV1&) = delete;
        MetadataParserImplV1& operator=(const MetadataParserImplV1&) = delete;
        MetadataParserImplV1(MetadataParserImplV1&&) = delete;
        MetadataParserImplV1& operator=(MetadataParserImplV1&&) = delete;

        /*
         * @brief Parse and return the vector of ComponentMetadata
         * @param metadata The value of the key "cm" in the manifest
         * @returns the vector of @ConfigurationMetadata objects
         */
        std::vector<ConfigurationMetadata> ParseAndGetConfigurationMetadata(const AnyMap& scrmap) const override;

      private:
        std::shared_ptr<cppmicroservices::logservice::LogService> logger;
      };
  } // util
} // cppmicroservices

#endif //CM_METADATAPARSERIMPL_HPP
