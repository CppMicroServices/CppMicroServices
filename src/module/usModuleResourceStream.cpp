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

#include "usModuleResourceStream.h"

#include "usModuleResource.h"

// 'this' used in base member initializer list
US_MSVC_PUSH_DISABLE_WARNING(4355)

US_BEGIN_NAMESPACE

ModuleResourceStream::ModuleResourceStream(const ModuleResource& resource, std::ios_base::openmode mode)
  : ModuleResourceBuffer(resource.GetData(), resource.GetSize(), mode | std::ios_base::in)
  , std::istream(this)
{
}

US_END_NAMESPACE

US_MSVC_POP_WARNING
