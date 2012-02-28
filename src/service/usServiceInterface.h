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


#ifndef USSERVICEINTERFACE_H
#define USSERVICEINTERFACE_H

template<class T> inline const char* us_service_interface_iid()
{ return 0; }

#if defined(QT_DEBUG) || defined(QT_NODEBUG)
#include <qobject.h>

#define US_DECLARE_SERVICE_INTERFACE(IFace, IId)                                 \
  template<> inline const char* qobject_interface_iid<IFace *>()                 \
  { return IId; }                                                                \
  template<> inline const char* us_service_interface_iid<IFace *>()              \
  { return IId; }                                                                \
  template<> inline IFace *qobject_cast<IFace *>(QObject *object)                \
  { return reinterpret_cast<IFace *>((object ? object->qt_metacast(IId) : 0)); } \
  template<> inline IFace *qobject_cast<IFace *>(const QObject *object)          \
  { return reinterpret_cast<IFace *>((object ? const_cast<QObject *>(object)->qt_metacast(IId) : 0)); }

#else

#define US_DECLARE_SERVICE_INTERFACE(IFace, IId)                                 \
  template<> inline const char* us_service_interface_iid<IFace *>()              \
  { return IId; }                                                                \

#endif

template<class T> inline const char* us_service_impl_name(T* /*impl*/)
{ return "(unknown implementation)"; }

#endif // USSERVICEINTERFACE_H
