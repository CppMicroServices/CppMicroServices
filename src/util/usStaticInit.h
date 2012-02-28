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


Extracted from qglobal.h from Qt 4.7.3 and adapted for CppMicroServices.
Original copyright (c) Nokia Corporation. Usage covered by the
GNU Lesser General Public License version 2.1
(http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) and the Nokia Qt
LGPL Exception version 1.1 (file LGPL_EXCEPTION.txt in Qt 4.7.3 package).

=========================================================================*/

#ifndef US_STATIC_INIT_H
#define US_STATIC_INIT_H

#include <usConfig.h>
#include "usThreads.h"

US_BEGIN_NAMESPACE

// POD for US_GLOBAL_STATIC
template <typename T>
class GlobalStatic : public US_DEFAULT_THREADING<GlobalStatic<T> >
{
public:

  GlobalStatic(T* p = 0, bool destroyed = false) : pointer(p), destroyed(destroyed) {}

  T* pointer;
  bool destroyed;

private:

  // purposely not implemented
  GlobalStatic(const GlobalStatic&);
  GlobalStatic& operator=(const GlobalStatic&);
};

template<typename T>
struct DefaultGlobalStaticDeleter
{
  void operator()(GlobalStatic<T>& globalStatic) const
  {
    delete globalStatic.pointer;
    globalStatic.pointer = 0;
    globalStatic.destroyed = true;
  }
};

// Created as a function-local static to delete a GlobalStatic<T>
template <typename T, template<typename T_> class Deleter = DefaultGlobalStaticDeleter>
class GlobalStaticDeleter
{
public:
  GlobalStatic<T> &globalStatic;

  GlobalStaticDeleter(GlobalStatic<T> &_globalStatic)
    : globalStatic(_globalStatic)
  { }

  inline ~GlobalStaticDeleter()
  {
    Deleter<T> deleter;
    deleter(globalStatic);
  }
};

US_END_NAMESPACE


#define US_GLOBAL_STATIC_INIT(TYPE, NAME)                                \
  static US_PREPEND_NAMESPACE(GlobalStatic)<TYPE>& this_##NAME()         \
  {                                                                      \
    static US_PREPEND_NAMESPACE(GlobalStatic)<TYPE> l;                   \
    return l;                                                            \
  }

#define US_GLOBAL_STATIC(TYPE, NAME)                                     \
  US_GLOBAL_STATIC_INIT(TYPE, NAME)                                      \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE;                                                \
      bool ok = false;                                                   \
      {                                                                  \
        US_PREPEND_NAMESPACE(GlobalStatic)<TYPE>::Lock lock(this_##NAME()); \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static US_PREPEND_NAMESPACE(GlobalStaticDeleter)<TYPE> cleanup(this_##NAME()); \
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#define US_GLOBAL_STATIC_WITH_DELETER(TYPE, NAME, DELETER)               \
  US_GLOBAL_STATIC_INIT(TYPE, NAME)                                      \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE;                                                \
      bool ok = false;                                                   \
      {                                                                  \
        US_PREPEND_NAMESPACE(GlobalStatic)<TYPE>::Lock lock(this_##NAME()); \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static US_PREPEND_NAMESPACE(GlobalStaticDeleter)<TYPE, DELETER > cleanup(this_##NAME()); \
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#define US_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                     \
  US_GLOBAL_STATIC_INIT(TYPE, NAME)                                      \
  static TYPE *NAME()                                                    \
  {                                                                      \
    if (!this_##NAME().pointer && !this_##NAME().destroyed)              \
    {                                                                    \
      TYPE *x = new TYPE ARGS;                                           \
      bool ok = false;                                                   \
      {                                                                  \
        US_PREPEND_NAMESPACE(GlobalStatic)<TYPE>::Lock lock(this_##NAME()); \
        if (!this_##NAME().pointer)                                      \
        {                                                                \
          this_##NAME().pointer = x;                                     \
          ok = true;                                                     \
        }                                                                \
      }                                                                  \
      if (!ok)                                                           \
        delete x;                                                        \
      else                                                               \
        static US_PREPEND_NAMESPACE(GlobalStaticDeleter)<TYPE> cleanup(this_##NAME()); \
    }                                                                    \
    return this_##NAME().pointer;                                        \
  }

#endif // US_STATIC_INIT_H
