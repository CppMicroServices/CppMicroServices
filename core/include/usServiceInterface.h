/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <usGlobalConfig.h>
#include <usServiceException.h>

#include <map>
#include <string>
#include <typeinfo>
#include <tuple>

namespace us {

class ServiceFactory;

/**
 * @ingroup MicroServices
 *
 * A map containing interfaces ids and their corresponding service object
 * pointers. InterfaceMap instances represent a complete service object
 * which implementes one or more service interfaces. For each implemented
 * service interface, there is an entry in the map with the key being
 * the service interface id and the value a pointer to the service
 * interface implementation.
 *
 * To create InterfaceMap instances, use the MakeInterfaceMap helper class.
 *
 * @note This is a low-level type and should only rarely be used.
 *
 * @see MakeInterfaceMap
 */
typedef std::map<std::string, void*> InterfaceMap;

/// \cond
namespace detail
{
  US_Core_EXPORT std::string GetDemangledName(const std::type_info& typeInfo);
}
/// \endcond

/**
 * \ingroup MicroServices
 *
 * Returns a unique id for a given type. By default, the
 * demangled name of \c T is returned.
 *
 * This template method may be specialized directly or be
 * using the macro #US_DECLARE_SERVICE_INTERFACE to return
 * a custom id for each service interface.
 *
 * @tparam T The service interface type.
 * @return A unique id for the service interface type T.
 */
template<class T> std::string us_service_interface_iid()
{
  return us::detail::GetDemangledName(typeid(T));
}

/// \cond
template<> inline std::string us_service_interface_iid<void>() { return std::string(); }
/// \endcond

/// \cond
namespace detail
{

  template <class Interfaces, size_t size>
  struct InsertInterfaceHelper
  {
      static void insert(InterfaceMap& im, const Interfaces& interfaces)
      {
          im.insert(std::make_pair(std::string(us_service_interface_iid<typename std::remove_pointer<typename std::tuple_element<size-1, Interfaces>::type>::type>()),
                                   static_cast<void*>(std::get<size-1>(interfaces))));
          InsertInterfaceHelper<Interfaces,size-1>::insert(im, interfaces);
      }
  };

  template<class T>
  struct InsertInterfaceHelper<T,0>
  {
      static void insert(InterfaceMap&, const T&) {}
  };

  template<class Interfaces>
  void InsertInterfaceTypes(InterfaceMap& im, const Interfaces& interfaces)
  {
      InsertInterfaceHelper<Interfaces, std::tuple_size<Interfaces>::value>::insert(im, interfaces);
  }

  template<template<class...> class List, class ...Args>
  struct InterfacesTuple {
      typedef List<typename std::add_pointer<Args>::type...> type;

      template<class Impl>
      static type create(Impl* impl) { return type(static_cast<typename std::add_pointer<Args>::type>(impl)...); }
  };

  template<class T, class... List>
  struct Contains : std::true_type {};

  template<class T, class Head, class... Rest>
  struct Contains<T, Head, Rest...>
    : std::conditional< std::is_same<T, Head>::value,
          std::true_type,
          Contains<T, Rest...>
          >::type
  {};

  template<class T>
  struct Contains<T> : std::false_type {};

}
/// \endcond

}


/**
 * \ingroup MicroServices
 *
 * \brief Declare a service interface id.
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
 * The usage of this macro is optional and the service interface id which is automatically
 * associated with any type is usually good enough (the demangled type name). However, care must
 * be taken if the default id is compared with a string literal hard-coding a service interface
 * id. E.g. the default id for templated types in the STL may differ between platforms. For
 * user-defined types and templates the ids are typically consistent, but platform specific
 * default template arguments will lead to different ids.
 *
 * This macro is normally used right after the class definition for _service_interface_type,
 * in a header file.
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
  template<> inline std::string us_service_interface_iid<_service_interface_type>()                \
  { return _service_interface_id; }                                                                \


namespace us {

/**
 * @ingroup MicroServices
 *
 * Helper class for constructing InterfaceMap instances based
 * on service implementations or service factories.
 *
 * Example usage:
 * \code
 * MyService service; // implementes I1 and I2
 * InterfaceMap im = MakeInterfaceMap<I1,I2>(&service);
 * \endcode
 *
 * @see InterfaceMap
 */
template<class ...Interfaces>
struct MakeInterfaceMap
{
  ServiceFactory* m_factory;

  typename detail::InterfacesTuple<std::tuple, Interfaces...>::type m_interfaces;

  /**
   * Constructor taking a service implementation pointer.
   *
   * @param impl A service implementation pointer, which must
   *        be castable to a all specified service interfaces.
   */
  template<class Impl>
  MakeInterfaceMap(Impl* impl)
    : m_factory(nullptr)
    , m_interfaces(detail::InterfacesTuple<std::tuple, Interfaces...>::create(impl))
  {}

  /**
   * Constructor taking a service factory.
   *
   * @param factory A service factory.
   */
  MakeInterfaceMap(ServiceFactory* factory)
    : m_factory(factory)
  {
    if (factory == nullptr)
    {
      throw ServiceException("The service factory argument must not be nullptr.");
    }
  }

  operator InterfaceMap ()
  {
    InterfaceMap sim;
    detail::InsertInterfaceTypes(sim, m_interfaces);

    if (m_factory)
    {
      sim.insert(std::make_pair(std::string("org.cppmicroservices.factory"),
                                static_cast<void*>(m_factory)));
    }

    return sim;
  }
};

/**
 * @ingroup MicroServices
 *
 * Extract a service interface pointer from a given InterfaceMap instance.
 *
 * @param map a InterfaceMap instance.
 * @return The service interface pointer for the service interface id of the
 *         \c I1 interface type or nullptr if \c map does not contain an entry
 *         for the given type.
 *
 * @see MakeInterfaceMap
 */
template<class Interface>
Interface* ExtractInterface(const InterfaceMap& map)
{
  InterfaceMap::const_iterator iter = map.find(us_service_interface_iid<Interface>());
  if (iter != map.end())
  {
    return reinterpret_cast<Interface*>(iter->second);
  }
  return nullptr;
}

/**
 * @ingroup MicroServices
 *
 * Extract a service interface pointer from a given InterfaceMap instance.
 *
 * @param map a InterfaceMap instance.
 * @param interfaceId The interface id string.
 * @return The service interface pointer for the service interface id or
 *         \c nullptr if \c map does not contain an entry for the given type.
 *
 * @see ExtractInterface(const InterfaceMap&)
 */
inline void* ExtractInterface(const InterfaceMap& map, const std::string& interfaceId)
{
  if (interfaceId.empty() && !map.empty())
  {
    return map.begin()->second;
  }

  auto iter = map.find(interfaceId);
  if (iter != map.end())
  {
    return iter->second;
  }
  return nullptr;
}

///@{
/**
 * @ingroup MicroServices
 *
 * Cast the argument to a \c ServiceFactory pointer. Useful when calling
 * \c BundleContext::RegisterService with a service factory, for example:
 *
 * \code
 * MyServiceFactory* factory;
 * context->RegisterService<ISomeInterface>(ToFactory(factory));
 * \endcode
 *
 * @param factory The service factory. May be a pointer or reference type.
 * @return A \c ServiceFactory pointer to the passed \c factory instance.
 *
 * @see BundleContext::RegisterService(ServiceFactory* factory, const ServiceProperties& properties)
 */
template<class T>
ServiceFactory* ToFactory(
        T& factory,
        typename std::enable_if<std::is_class <T>::value, T>::type* = nullptr)
{
    return static_cast<ServiceFactory*>(&factory);
}

template<class T>
ServiceFactory* ToFactory(
        T factory,
        typename std::enable_if<std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value>::type* = nullptr)
{
    return static_cast<ServiceFactory*>(factory);
}
///@}

}


#endif // USSERVICEINTERFACE_H
