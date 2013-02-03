/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
class US_EXPORT ModuleResourceStream : private ModuleResourceBuffer, public std::istream
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
