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


#ifndef USBASE_H
#define USBASE_H

#include <usConfig.h>
#include <usServiceInterface.h>

US_BEGIN_NAMESPACE

class Base
{

public:

  virtual ~Base() {}
  virtual const char* GetNameOfClass() const { return "US_PREPEND_NAMESPACE(Base)"; }
};

US_END_NAMESPACE

template<> inline const char* us_service_impl_name(US_PREPEND_NAMESPACE(Base)* impl)
{ return impl->GetNameOfClass(); }

#endif // USBASE_H
