/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USMODULERESOURCESTREAM_H
#define USMODULERESOURCESTREAM_H

#include "usModuleResourceBuffer_p.h"

#include <fstream>

US_BEGIN_NAMESPACE

class ModuleResource;

/**
 * \ingroup MicroServices
 *
 * An input stream class for ModuleResource objects.
 *
 * This class provides access to the resource data embedded in a module's
 * shared library via a STL input stream interface.
 *
 * \see ModuleResource for an example how to use this class.
 */
class US_Core_EXPORT ModuleResourceStream : private ModuleResourceBuffer, public std::istream
{

public:

  /**
   * Construct a %ModuleResourceStream object.
   *
   * @param resource The ModuleResource object for which an input stream
   * should be constructed.
   * @param mode The open mode of the stream. If \c std::ios_base::binary
   * is used, the resource data will be treated as binary data, otherwise
   * the data is interpreted as text data and the usual platform specific
   * end-of-line translations take place.
   */
  ModuleResourceStream(const ModuleResource& resource,
                       std::ios_base::openmode mode = std::ios_base::in);

private:

  // purposely not implemented
  ModuleResourceStream(const ModuleResourceStream&);
  ModuleResourceStream& operator=(const ModuleResourceStream&);
};

US_END_NAMESPACE

#endif // USMODULERESOURCESTREAM_H
