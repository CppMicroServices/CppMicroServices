/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <map>
#include <string>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>
#include <locale>
#include <cctype>
#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices {
namespace scrimpl {
namespace util {

template <typename T>
void ThrowIfEmpty(const T&, const std::string&)
{
}

template <typename T>
void ThrowIfEmptyHelper(const T& value, const std::string& key)
{
  if (value.empty())
  {
    std::string msg;
    if (key.empty())
    {
      msg = "Value cannot be empty.";
    }
    else
    {
      msg = "Value for the name '" + key + "' cannot be empty.";
    }
    throw std::runtime_error(msg);
  }
}
  
// @brief Throws if the container T is of type @c Any or @c string or @c AnyMap and it's empty
template <typename TargetType>
void ThrowIfValueAbsentInChoices(const TargetType&, const std::vector<TargetType>&)
{
}

template <> void ThrowIfEmpty(const std::vector<cppmicroservices::Any>& value, const std::string& key);
template <> void ThrowIfEmpty(const std::string& value, const std::string& key);
template <> void ThrowIfEmpty(const cppmicroservices::AnyMap& value, const std::string& key);
template <> void ThrowIfValueAbsentInChoices(const std::string& value, const std::vector<std::string>& choices);
/*
 * @brief This class is used to validate the objects of the service
 *        description.
 *
 * In addition, it returns the values of the objects
 * in a type-safe way and checks if certain container types are
 * not empty.
 */
class ObjectValidator
{
public:

  /*
   * @param scrmap A @c MapType object that has a @c std::string key
   *        and an @c Any value
   * @param _key The key whose value is needed
   * @param isOptional Set to @c true if the key is optional (not a mandatory key)
   * @throws out_of_range exception if @p isOptional is false and @p scrmap doesn't have
   *         the key @p _key
   *
   * Example:
   * Suppose a key 'name' doesn't exist in the map 'scrmap', then
   *
   * @code
   * ObjectValidator(scrmap, "name", true); // works fine
   * ObjectValidator(scrmap, "name"); // throws out_of_range exception
   * @endcode
   */
  template <typename MapType>
  ObjectValidator(const MapType& scrmap, std::string _key, bool isOptional=false)
    : key(std::move(_key))
    , keyExists(true)
  {
    isOptional ? HandleOptionalObject(scrmap) : HandleMandatoryObject(scrmap);
  }

  /*
   * @brief This overload is useful when we want to cast a value of type @c Any to
   *        another type.
   * @param _value An object of type @c Any
   *
   * @note This can be done by using @c cppmicroservices::any_cast but
   *       by using this constructor and the GetValue() member function, we
   *       can also verify that certain underlying containers are non-empty.
   *
   * Example:
   * The verification that the Any object is of type std::string and that
   * it's non-empty is done by
   *
   * @code
   * auto str = ObjectValidator(any_object).GetValue<std::string>();
   * @endcode
   */
  ObjectValidator(cppmicroservices::Any _value)
    : keyExists(true)
    , value(std::move(_value))
  {
  }

  /*
   * @returns false if @c isOptional is @c true and
   *          the key @c _key doesn't exist in the map @c scrmap.
   *          (Refer to the three argument constructor)
   *          Otherwise, return @c true.
   *
   * Example:
   * Suppose an optional key 'name' is to be verified
   * in the map 'scrmap',
   *
   * @code
   * auto object = ObjectValidator(scrmap, "name", true);
   * @endcode
   *
   * For the above statement, @c object.KeyExists() returns @c true if the key
   * @c name exists in the map and returns @c false if it doesn't exist.
   */
  bool KeyExists() const
  {
    return keyExists;
  }

  /*
   * @returns @c scrmap[key] after casting the value to @c ReturnType
   *
   * @throws runtime_error if @c KeyExists() returns false i.e
   *         when @c isOptional = true and the key doesn't exist in the @c scrmap
   * @throws BadAnyCastException if the value @c scrmap[key] cannot be
   *         cast to @c ReturnType
   * @throws runtime_error if the value @c scrmap[key] is one of types
   *         @c vector<Any> or @c AnyMap or @c std::string and it is empty.
   *
   * Examples:
   * 1. Suppose a key @c name doesn't exist in the map @c scrmap
   *
   * @code
   * auto object = ObjectValidator(scrmap, "name", true);
   * object.GetValue<std::string>(); // throws runtime_error because "name"
   *                                 // doesn't exist in the scrmap
   * @endcode
   *
   * 2. Suppose a key @c name exists in the map @c scrmap and the value is
   *    of type @c std::string.
   *
   * @code
   * auto object = ObjectValidator(scrmap, "name");
   * object.GetValue<int>(); // throws BadAnyCastException because the underlying type
   *                         // is std::string
   * @endcode
   *
   * 3. Suppose a key @c name exists in the map @c scrmap and the value of
   *    type @c std::string but the string is empty, then
   *
   * @code
   * auto object = ObjectValidator(scrmap, "name");
   * object.GetValue<std::string>(); // throws runtime_error because the value is empty
   * @endcode
   */
  template <typename ReturnType>
  ReturnType GetValue() const
  {
    if (!KeyExists())
    {
      throw std::runtime_error("ObjectValidator::GetValue() is called on a object that doesn't exist.");
    }
    return CheckAndGetValue<ReturnType>();
  }

  /*
   * @brief Assigns the value @c scrmap[key] to @p target
   *
   * @param target the target object to be assigned
   * @param choices the optional vector of items for membership check
   *
   * @throws BadAnyCastException if the value @c scrmap[key] is not the
   *         same type as @p target
   * @throws runtime_error if the value @c scrmap[key] is one of types
   *         @c vector<Any> or @c AnyMap or @c std::string and it is empty.
   * @throws runtime_error if the value @c scrmap[key] is not present in
   *         the vector @p choices if @p choices is non-empty. This exception
   *         won't be thrown if @p choices is empty
   * @note if the key @c key is not found in @c scrmap, this function
   *       does nothing
   *
   * Examples:
   * @code
   * object.AssignValueTo(a, {"foo", "bar", "baz"});
   * @endcode
   *
   * This code segment throws if the value scrmap[a] is not one of
   * "foo", "bar" or "baz".
   */
  template <typename TargetType>
  void AssignValueTo(TargetType& target, const std::vector<TargetType>& choices = {}) const
  {
    if (KeyExists())
    {
      TargetType value = CheckAndGetValue<TargetType>();
      ThrowIfValueAbsentInChoices(value, choices);
      target = std::move(value);
    }
  }

private:
  template <typename ReturnType>
  ReturnType CheckAndGetValue() const
  {
    try
    {
      ReturnType retval = cppmicroservices::any_cast<ReturnType>(value);
      ThrowIfEmpty(retval, key);
      return retval;
    }
    catch (const cppmicroservices::BadAnyCastException& exp)
    {
      std::string msg;
      if (!key.empty())
      {
        msg += "Unexpected type for the name '" + key + "'. ";
      }
      msg += "Exception: " + std::string(exp.what()) + ". Actual type is " + value.Type().name();
      throw cppmicroservices::BadAnyCastException(msg);
    }
  }

  template <typename MapType>
  void HandleMandatoryObject(const MapType& scrmap)
  {
    try
    {
      value = scrmap.at(key);
    }
    catch (const std::out_of_range&)
    {
      std::string msg = "Missing key '" + key + "' in the manifest.";
      throw std::out_of_range(msg);
    }
  }

  template <typename MapType>
  void HandleOptionalObject(const MapType& scrmap)
  {
    auto iter = scrmap.find(key);
    if (iter == scrmap.end())
    {
      keyExists = false;
    }
    else
    {
      value = scrmap.at(key);
    }
  }

  std::string key;
  bool keyExists;
  cppmicroservices::Any value;
};

// @brief If the client simply wants the Any data, return it.
template <>
inline cppmicroservices::Any ObjectValidator::GetValue<cppmicroservices::Any>() const
{
  return value;
}
}
}
}

#endif // UTIL_HPP
