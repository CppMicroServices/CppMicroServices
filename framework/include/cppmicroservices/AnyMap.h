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

#ifndef CPPMICROSERVICES_ANYMAP_H
#define CPPMICROSERVICES_ANYMAP_H

#include "cppmicroservices/Any.h"

#include <unordered_map>
#include <string>

namespace cppmicroservices {

namespace detail {

struct AnyMapCIHash
{
  std::size_t operator()(const std::string& key) const;
};

struct AnyMapCIEqual
{
  bool operator()(const std::string& l, const std::string& r) const;
};

}

/**
 * \ingroup MicroServicesUtils
 *
 * A map data structure which wraps different STL map types.
 *
 * This is a convenience class providing a STL associative container
 * interface for different underlying container types. Supported underlying
 * types are
 * - \c AnyMap::OrderedMap (a STL map)
 * - \c AnyMap::UnorderedMap (a STL unordered map)
 * - \c AnyMap::UnorderedMapCaseInsensitiveKeys (a STL unordered map with case insensitive key comparison)
 *
 * This class provides most of the STL functions for associated containers,
 * including forward iterators. It is typically not instantiated by clients
 * directly, but obtained via framework API calls.
 *
 * The \c AnyMap is a recursive data structure, and its values can be
 * retrieved via standard map functions or by using a dotted key notation
 * specifying a compound key.
 *
 * @see GetDotted(const std::string&)
 */
class US_Framework_EXPORT AnyMap
{

public:

  typedef std::string key_type;
  typedef Any mapped_type;
  typedef std::pair<const key_type, mapped_type> value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;

  typedef std::map<std::string, Any> OrderedMap;
  typedef std::unordered_map<std::string, Any> UnorderedMap;
  typedef std::unordered_map<std::string, Any, detail::AnyMapCIHash, detail::AnyMapCIEqual> UnorderedMapCaseInsensitiveKeys;

  enum class Type
  {
    ORDERED_MAP,
    UNORDERED_MAP,
    UNORDERED_MAP_CASEINSENSITIVE_KEYS
  };

private:

  class IteratorBase
  {
    friend class AnyMap;

  protected:

    enum IterType
    {
      NONE,
      ORDERED,
      UNORDERED,
      UNORDERED_CI
    };

    IterType type;

    IteratorBase()
      : type(NONE)
    {}

    IteratorBase(IterType type)
      : type(type)
    {}

  public:

    typedef AnyMap::value_type value_type;

    typedef std::forward_iterator_tag iterator_category;
    typedef AnyMap::difference_type difference_type;

  };

public:

  class ConstIterator : public IteratorBase
  {
  private:

    typedef OrderedMap::const_iterator OConstIter;
    typedef UnorderedMap::const_iterator UOConstIter;
    typedef UnorderedMapCaseInsensitiveKeys::const_iterator UOCIConstIter;

  public:

    typedef AnyMap::const_reference reference;
    typedef AnyMap::const_pointer pointer;

    typedef ConstIterator iterator;

    ConstIterator();
    ConstIterator(const iterator& it);
    ~ConstIterator();

    ConstIterator(OConstIter&& it);
    ConstIterator(UOConstIter&& it, IterType type);

    reference operator* () const;
    pointer operator-> () const;

    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& x) const;
    bool operator!=(const iterator& x) const;

  private:

    OConstIter const& o_it() const;
    OConstIter& o_it();
    UOConstIter const& uo_it() const;
    UOConstIter& uo_it();
    UOCIConstIter const& uoci_it() const;
    UOCIConstIter& uoci_it();

    union {
      char o[sizeof(OConstIter)];
      char uo[sizeof(UOConstIter)];
      char uoci[sizeof(UOCIConstIter)];
    } iter;
  };

  struct Iterator : public IteratorBase
  {
  private:

    typedef OrderedMap::iterator OIter;
    typedef UnorderedMap::iterator UOIter;
    typedef UnorderedMapCaseInsensitiveKeys::iterator UOCIIter;

  public:

    typedef AnyMap::reference reference;
    typedef AnyMap::pointer pointer;

    typedef Iterator iterator;

    Iterator();
    Iterator(const iterator& it);
    ~Iterator();

    Iterator(OIter&& it);
    Iterator(UOIter&& it, IterType type);

    reference operator* () const;
    pointer operator-> () const;

    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& x) const;
    bool operator!=(const iterator& x) const;

  private:

    OIter const& o_it() const;
    OIter& o_it();
    UOIter const& uo_it() const;
    UOIter& uo_it();
    UOCIIter const& uoci_it() const;
    UOCIIter& uoci_it();

    union {
      char o[sizeof(OIter)];
      char uo[sizeof(UOIter)];
      char uoci[sizeof(UOCIIter)];
    } iter;

  };

  typedef Iterator iterator;
  typedef ConstIterator const_iterator;

  AnyMap(Type type);
  AnyMap(const OrderedMap& m);
  AnyMap(const UnorderedMap& m);
  AnyMap(const UnorderedMapCaseInsensitiveKeys& m);

  AnyMap(const AnyMap& m);
  AnyMap& operator=(const AnyMap& m);

  ~AnyMap();

  Type GetType() const;

  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;

  bool empty() const;
  size_type size() const;
  size_type count(const key_type& key) const;
  void clear();

  mapped_type& at(const key_type& key);
  const mapped_type& at(const key_type& key) const;

  mapped_type& operator[](const key_type& key);
  mapped_type& operator[](key_type&& key);

  std::pair<iterator, bool> insert(const value_type& value);
  const_iterator find(const key_type& key) const;

  /**
   * Get a key's value, using a compound key notation.
   *
   * A compound key consists of one or more key names, concatenated with
   * the '.' (dot) character. Each key except the last requires the referenced
   * Any object to be of type \c AnyMap or \c std::vector<Any>. Containers
   * of type \c std::vector<Any> are indexed using 0-based numerical key names.
   *
   * For example, a \c AnyMap object holding data of the following layout
   * \code{.json}
   * {
   *   one: 1,
   *   two: "two",
   *   three: {
   *     a: "anton",
   *     b: [ 3, 8 ]
   *   }
   * }
   * \endcode
   * can be queried using the following notation:
   * \code
   * map.GetDotted("one"); // returns Any(1)
   * map.GetDotted("three.a"); // returns Any(std::string("anton"))
   * map.GetDotted("three.b.1"); // returns Any(8)
   * \endcode
   *
   * @param key The key hierachy to query.
   * @return A reference to the key's value.
   *
   * @throws std::invalid_argument if the \c Any value for a given key is not of type \c AnyMap or \c std::vector<Any>.
   * std::out_of_range if the key is not found or a numerical index would fall out of the range of an \c int type.
   */
  mapped_type& GetDotted(const key_type& key);
  const mapped_type& GetDotted(const key_type& key) const;


private:

  OrderedMap const& o_m() const;
  OrderedMap& o_m();
  UnorderedMap const& uo_m() const;
  UnorderedMap& uo_m();
  UnorderedMapCaseInsensitiveKeys const& uoci_m() const;
  UnorderedMapCaseInsensitiveKeys& uoci_m();

  Type type;

  union {
    char o[sizeof(OrderedMap)];
    char uo[sizeof(UnorderedMap)];
    char uoci[sizeof(UnorderedMapCaseInsensitiveKeys)];
  } map;
};

template<>
US_Framework_EXPORT std::ostream& any_value_to_string(std::ostream& os, const AnyMap& m);

}

#endif // CPPMICROSERVICES_ANYMAP_H
