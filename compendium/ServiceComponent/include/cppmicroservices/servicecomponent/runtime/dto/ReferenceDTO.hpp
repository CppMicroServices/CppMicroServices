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

#ifndef ReferenceDTO_hpp
#define ReferenceDTO_hpp

#include <string>

#include <cppmicroservices/servicecomponent/ServiceComponentExport.h>

namespace cppmicroservices
{
    namespace service
    {
        namespace component
        {
            namespace runtime
            {
                namespace dto
                {

                    /**
                     \defgroup gr_referencedto ReferenceDTO
                     \brief Groups ReferenceDTO related symbols.
                     */

                    /**
                     * \ingroup gr_referencedto
                     *
                     * A representation of a declared reference to a service.
                     */
                    struct US_ServiceComponent_EXPORT ReferenceDTO
                    {
                        /**
                         * The name of the reference.
                         *
                         * <p>
                         * This is declared in the \c name attribute of the \c reference
                         * element. This must be the default name if the component description does
                         * not declare a name for the reference.
                         */
                        std::string name;

                        /**
                         * The service interface of the reference.
                         *
                         * <p>
                         * This is declared in the \c interface attribute of the
                         * \c reference element.
                         */
                        std::string interfaceName;

                        /**
                         * The cardinality of the reference.
                         *
                         * <p>
                         * This is declared in the \c cardinality attribute of the
                         * \c reference element. This must be the default cardinality if the
                         * component description does not declare a cardinality for the reference.
                         */
                        std::string cardinality;

                        /**
                         * The policy of the reference.
                         *
                         * <p>
                         * This is declared in the \c policy attribute of the \c reference
                         * element. This must be the default policy if the component description
                         * does not declare a policy for the reference.
                         */
                        std::string policy;

                        /**
                         * The policy option of the reference.
                         *
                         * <p>
                         * This is declared in the \c policy-option attribute of the
                         * \c reference element. This must be the default policy option if the
                         * component description does not declare a policy option for the reference.
                         */
                        std::string policyOption;

                        /**
                         * The target of the reference.
                         *
                         * <p>
                         * This is declared in the \c target attribute of the \c reference
                         * element. This must be an empty string if the component description does not
                         * declare a target for the reference.
                         */
                        std::string target;
/**
                         * The require-bind of the reference.
                         *
                         * <p>
                         * This is declared in the \c require-bind attribute of the \c reference
                         * element. This specifies whether, for this specific reference, the service
                         * must implement a \c bind and \c unbind method regardless of the whether
                         * the \c inject-references property for the \c component is observed
                         */
                        bool requireBind;

                        /**
                         * The name of the bind method of the reference.
                         *
                         * <p>
                         * This is declared in the \c bind attribute of the \c reference
                         * element. This must be an empty string if the component description does not
                         * declare a bind method for the reference.
                         */
                        std::string bind;

                        /**
                         * The name of the unbind method of the reference.
                         *
                         * <p>
                         * This is declared in the \c unbind attribute of the \c reference
                         * element. This must be an empty string if the component description does not
                         * declare an unbind method for the reference.
                         */
                        std::string unbind;

                        /**
                         * The name of the updated method of the reference.
                         *
                         * <p>
                         * This is declared in the \c updated attribute of the
                         * \c reference element. This must be an empty string if the component
                         * description does not declare an updated method for the reference.
                         */
                        std::string updated;

                        /**
                         * The scope of the reference.
                         *
                         * <p>
                         * This is declared in the \c scope attribute of the \c reference
                         * element. This must be the default scope if the component description does
                         * not declare a scope for the reference.
                         */
                        std::string scope;
                    };
                } // namespace dto
            }     // namespace runtime
        }         // namespace component
    }             // namespace service
} // namespace cppmicroservices

#endif /* ReferenceDTO_hpp */
