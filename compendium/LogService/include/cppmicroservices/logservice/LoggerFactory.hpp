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
#ifndef CPPMICROSERVICES_LOGGER_FACTORY_H_
#define CPPMICROSERVICES_LOGGER_FACTORY_H_
#include "cppmicroservices/logservice/Logger.hpp"
#include "cppmicroservices/Bundle.h"

#include <cstdint>
#include <exception>
#include <string>
#include <memory>

namespace cppmicroservices::logservice
{
	 /**
         \defgroup gr_logservice LoggerFactory

         \brief Groups LoggerFactory class related symbols.
         */

	 /**
         * \ingroup MicroService
         * \ingroup gr_logservice
         *
         * LoggerFactory is the service interface that allows a bundle to obtain a Logger.
         * It provides methods to obtain Logger; then a bundle can start logging messages to the Log Service by calling one of the Logger methods.
         *
         * @remarks This class is thread safe.
         */

	class LoggerFactory
	{
	    public:

	        inline static const std::string ROOT_LOGGER_NAME = "ROOT";
	
                virtual ~LoggerFactory() = default;

		/**
                 * Return the Logger named with the specified name.
                 * @param name The name to use for the logger name. 
                 * 
		 * If the name parameter is equal to ROOT_LOGGER_NAME, then the root logger is returned.
                 */
                [[nodiscard]] virtual std::shared_ptr<Logger> getLogger(std::string const& name = ROOT_LOGGER_NAME) const = 0;

		 /**
                 * Return the Logger for the specified bundle, named with the specified name.
		 * @param bundle The bundle associated with the Logger. 
                 * @param name The name to use for the logger name.
                 *
                 * If the name parameter is equal to ROOT_LOGGER_NAME, then the root logger is returned.
                 */
                [[nodiscard]] virtual std::shared_ptr<Logger> getLogger(const cppmicroservices::Bundle& bundle, std::string const& name = ROOT_LOGGER_NAME) const = 0; 
        };
}

#endif
