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

#include <map>
#include <string>

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
template<class T> inline const char* us_service_interface_iid();

#if defined(QT_DEBUG) || defined(QT_NO_DEBUG)
#include <qobject.h>
#include US_BASECLASS_HEADER

#define US_DECLARE_SERVICE_INTERFACE(_service_interface_type, _service_interface_id)                  \
  template<> inline const char* qobject_interface_iid<_service_interface_type *>()                    \
  { return _service_interface_id; }                                                                   \
  template<> inline const char* us_service_interface_iid<_service_interface_type>()                   \
  { return _service_interface_id; }                                                                   \
  template<> inline _service_interface_type *qobject_cast<_service_interface_type *>(QObject *object) \
  { return reinterpret_cast<_service_interface_type*>(object ? object->qt_metacast(_service_interface_id) : 0); } \
  template<> inline _service_interface_type *qobject_cast<_service_interface_type *>(const QObject *object) \
  { return reinterpret_cast<_service_interface_type*>(object ? const_cast<QObject *>(object)->qt_metacast(_service_interface_id) : 0); }

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
  template<> inline const char* us_service_interface_iid<_service_interface_type>()              \
  { return _service_interface_id; }                                                                \

#endif

US_BEGIN_NAMESPACE

template<class Interface>
struct InterfaceT {};


typedef std::map<std::string, void*> InterfaceMap;

template<class S, class I1>
InterfaceMap MakeInterfaceMap(S* service, InterfaceT<I1>)
{
  InterfaceMap sim;
  sim.insert(std::make_pair(std::string(us_service_interface_iid<I1>()),
                            static_cast<void*>(static_cast<I1*>(service))));
  return sim;
}

class ServiceFactory;

template<class I1>
InterfaceMap MakeInterfaceMap(ServiceFactory* factory, InterfaceT<I1>)
{
  InterfaceMap sim;
  sim.insert(std::make_pair(std::string("org.cppmicroservices.factory"),
                            static_cast<void*>(factory)));
  sim.insert(std::make_pair(std::string(us_service_interface_iid<I1>()), static_cast<void*>(NULL)));
  return sim;
}

template<class S>
InterfaceMap MakeInterfaceMap(S* service, const std::string& interfaceId)
{
  InterfaceMap sim;
  sim.insert(std::make_pair(interfaceId,
                            static_cast<void*>(service)));
  return sim;
}

template<class I1>
I1* ExtractInterface(const InterfaceMap& map, InterfaceT<I1>)
{
  InterfaceMap::const_iterator iter = map.find(us_service_interface_iid<I1>());
  if (iter != map.end())
  {
    return reinterpret_cast<I1*>(iter->second);
  }
  return NULL;
}

US_END_NAMESPACE


#endif // USSERVICEINTERFACE_H
