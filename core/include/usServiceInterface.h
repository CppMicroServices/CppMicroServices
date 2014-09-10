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

#include <usGlobalConfig.h>
#include <usServiceException.h>

#include <map>
#include <string>
#include <typeinfo>

US_BEGIN_NAMESPACE
std::string GetDemangledName(const std::type_info& typeInfo);
US_END_NAMESPACE

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
  return US_PREPEND_NAMESPACE(GetDemangledName)(typeid(T));
}

/// \cond
template<> inline std::string us_service_interface_iid<void>() { return std::string(); }
/// \endcond


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


US_BEGIN_NAMESPACE

class ServiceFactory;

/**
 * @ingroup MicroServices
 *
 * A helper type used in several methods to get proper
 * method overload resolutions.
 */
template<class Interface>
struct InterfaceType {};

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


template<class I>
bool InsertInterfaceType(InterfaceMap& im, I* i)
{
  if (us_service_interface_iid<I>().empty())
  {
    throw ServiceException(std::string("The interface class ") + typeid(I).name() +
                                " uses an invalid id in its US_DECLARE_SERVICE_INTERFACE macro call.");
  }
  im.insert(std::make_pair(std::string(us_service_interface_iid<I>()),
                           static_cast<void*>(static_cast<I*>(i))));
  return true;
}

template<>
inline bool InsertInterfaceType<void>(InterfaceMap&, void*)
{
  return false;
}


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
 * The MakeInterfaceMap supports service implementations with
 * up to three service interfaces.
 *
 * @see InterfaceMap
 */
template<class I1, class I2 = void, class I3 = void>
struct MakeInterfaceMap
{
  ServiceFactory* m_factory;
  I1* m_interface1;
  I2* m_interface2;
  I3* m_interface3;

  /**
   * Constructor taking a service implementation pointer.
   *
   * @param impl A service implementation pointer, which must
   *        be castable to a all specified service interfaces.
   */
  template<class Impl>
  MakeInterfaceMap(Impl* impl)
    : m_factory(NULL)
    , m_interface1(static_cast<I1*>(impl))
    , m_interface2(static_cast<I2*>(impl))
    , m_interface3(static_cast<I3*>(impl))
  {}

  /**
   * Constructor taking a service factory.
   *
   * @param factory A service factory.
   */
  MakeInterfaceMap(ServiceFactory* factory)
    : m_factory(factory)
    , m_interface1(NULL)
    , m_interface2(NULL)
    , m_interface3(NULL)
  {
    if (factory == NULL)
    {
      throw ServiceException("The service factory argument must not be NULL.");
    }
  }

  operator InterfaceMap ()
  {
    InterfaceMap sim;
    InsertInterfaceType(sim, m_interface1);
    InsertInterfaceType(sim, m_interface2);
    InsertInterfaceType(sim, m_interface3);

    if (m_factory)
    {
      sim.insert(std::make_pair(std::string("org.cppmicroservices.factory"),
                                static_cast<void*>(m_factory)));
    }

    return sim;
  }
};

/// \cond
template<class I1, class I2>
struct MakeInterfaceMap<I1,I2,void>
{
  ServiceFactory* m_factory;
  I1* m_interface1;
  I2* m_interface2;

  template<class Impl>
  MakeInterfaceMap(Impl* impl)
    : m_factory(NULL)
    , m_interface1(static_cast<I1*>(impl))
    , m_interface2(static_cast<I2*>(impl))
  {}

  MakeInterfaceMap(ServiceFactory* factory)
    : m_factory(factory)
    , m_interface1(NULL)
    , m_interface2(NULL)
  {
    if (factory == NULL)
    {
      throw ServiceException("The service factory argument must not be NULL.");
    }
  }

  operator InterfaceMap ()
  {
    InterfaceMap sim;
    InsertInterfaceType(sim, m_interface1);
    InsertInterfaceType(sim, m_interface2);

    if (m_factory)
    {
      sim.insert(std::make_pair(std::string("org.cppmicroservices.factory"),
                                static_cast<void*>(m_factory)));
    }

    return sim;
  }
};

template<class I1>
struct MakeInterfaceMap<I1,void,void>
{
  ServiceFactory* m_factory;
  I1* m_interface1;

  template<class Impl>
  MakeInterfaceMap(Impl* impl)
    : m_factory(NULL)
    , m_interface1(static_cast<I1*>(impl))
  {}

  MakeInterfaceMap(ServiceFactory* factory)
    : m_factory(factory)
    , m_interface1(NULL)
  {
    if (factory == NULL)
    {
      throw ServiceException("The service factory argument must not be NULL.");
    }
  }

  operator InterfaceMap ()
  {
    InterfaceMap sim;
    InsertInterfaceType(sim, m_interface1);

    if (m_factory)
    {
      sim.insert(std::make_pair(std::string("org.cppmicroservices.factory"),
                                static_cast<void*>(m_factory)));
    }

    return sim;
  }
};

template<>
struct MakeInterfaceMap<void,void,void>;
/// \endcond

/**
 * @ingroup MicroServices
 *
 * Extract a service interface pointer from a given InterfaceMap instance.
 *
 * @param map a InterfaceMap instance.
 * @return The service interface pointer for the service interface id of the
 *         \c I1 interface type or NULL if \c map does not contain an entry
 *         for the given type.
 *
 * @see MakeInterfaceMap
 */
template<class I1>
I1* ExtractInterface(const InterfaceMap& map)
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
