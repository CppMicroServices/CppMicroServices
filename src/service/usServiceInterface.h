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

#include <usConfig.h>

/**
 * \ingroup MicroServices
 *
 * Returns a unique id for a given type.
 *
 * This template method is specialized in the macro
 * #US_DECLARE_SERVICE_INTERFACE to return a unique id
 * for each service interface.
 *
 * @tparam T The service interface type.
 * @return A unique id for the service interface type T.
 */
template<class T> inline const char* us_service_interface_iid()
{ return 0; }

template<class T> inline const char* us_service_impl_name(T* /*impl*/)
{ return "(unknown implementation)"; }

#if defined(QT_DEBUG) || defined(QT_NO_DEBUG)
#include <qobject.h>
#include US_BASECLASS_HEADER

#define US_DECLARE_SERVICE_INTERFACE(_service_interface_type, _service_interface_id)               \
  template<> inline const char* qobject_interface_iid<_service_interface_type *>()                 \
  { return _service_interface_id; }                                                                \
  template<> inline const char* us_service_interface_iid<_service_interface_type *>()              \
  { return _service_interface_id; }                                                                \
  template<> inline _service_interface_type *qobject_cast<_service_interface_type *>(QObject *object) \
  { return dynamic_cast<_service_interface_type*>(reinterpret_cast<US_BASECLASS_NAME *>((object ? object->qt_metacast(_service_interface_id) : 0))); } \
  template<> inline _service_interface_type *qobject_cast<_service_interface_type *>(const QObject *object) \
  { return dynamic_cast<_service_interface_type*>(reinterpret_cast<US_BASECLASS_NAME *>((object ? const_cast<QObject *>(object)->qt_metacast(_service_interface_id) : 0))); }

#else

/**
 * \ingroup MicroServices
 *
 * \brief Declare a CppMicroServices service interface.
 *
 * This macro associates the given identifier \e _service_interface_id (a string literal) to the
 * interface class called _service_interface_type. The Identifier must be unique. For example:
 *
 * \code
 * #include <usServiceInterface.h>
 *
 * struct ISomeInterace { ... };
 *
 * US_DECLARE_SERVICE_INTERFACE(ISomeInterface, "com.mycompany.service.ISomeInterface/1.0")
 * \endcode
 *
 * This macro is normally used right after the class definition for _service_interface_type, in a header file.
 *
 * If you want to use #US_DECLARE_SERVICE_INTERFACE with interface classes declared in a
 * namespace then you have to make sure the #US_DECLARE_SERVICE_INTERFACE macro call is not
 * inside a namespace though. For example:
 *
 * \code
 * #include <usServiceInterface.h>
 *
 * namespace Foo
 * {
 *   struct ISomeInterface { ... };
 * }
 *
 * US_DECLARE_SERVICE_INTERFACE(Foo::ISomeInterface, "com.mycompany.service.ISomeInterface/1.0")
 * \endcode
 *
 * @param _service_interface_type The service interface type.
 * @param _service_interface_id A string literal representing a globally unique identifier.
 */
#define US_DECLARE_SERVICE_INTERFACE(_service_interface_type, _service_interface_id)               \
  template<> inline const char* us_service_interface_iid<_service_interface_type *>()              \
  { return _service_interface_id; }                                                                \

#endif

#endif // USSERVICEINTERFACE_H
