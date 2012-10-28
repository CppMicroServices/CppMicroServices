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

#ifndef USTESTUTILSHAREDLIBRARY_H
#define USTESTUTILSHAREDLIBRARY_H

#include "usConfig.h"

#include <string>

US_BEGIN_NAMESPACE

class SharedLibraryHandle
{
public:

  SharedLibraryHandle();

  SharedLibraryHandle(const std::string& name);

  virtual ~SharedLibraryHandle();

  void Load();

  void Load(const std::string& name);

  void Unload();

  std::string GetAbsolutePath(const std::string& name);

  std::string GetAbsolutePath();

  static std::string GetLibraryPath();

  static std::string Suffix();

  static std::string Prefix();

private:

  SharedLibraryHandle(const SharedLibraryHandle&);
  SharedLibraryHandle& operator = (const SharedLibraryHandle&);

  std::string m_Name;
  void* m_Handle;
};

US_END_NAMESPACE

#endif // USTESTUTILSHAREDLIBRARY_H
