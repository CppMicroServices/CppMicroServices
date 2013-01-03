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

class US_EXPORT ModuleResourceStream : private ModuleResourceBuffer, public std::istream
{

public:

  ModuleResourceStream(const ModuleResource& resource,
                       std::ios_base::openmode mode = std::ios_base::in);

private:

  // purposely not implemented
  ModuleResourceStream(const ModuleResourceStream&);
  ModuleResourceStream& operator=(const ModuleResourceStream&);
};

US_END_NAMESPACE

#endif // USMODULERESOURCESTREAM_H
