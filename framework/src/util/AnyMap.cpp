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

#include "cppmicroservices/AnyMap.h"

#include <stdexcept>

namespace cppmicroservices {

namespace detail {

std::size_t AnyMapCIHash::operator()(const std::string& key) const
{
  std::size_t h = 0;
  std::for_each(key.begin(), key.end(), [&h](char c )
  {
    h += tolower(c);
  });
  return h;
}

bool AnyMapCIEqual::operator()(const std::string& l, const std::string& r) const
{
  return l.size() == r.size() &&
      std::equal(l.begin(), l.end(), r.begin(), []( char a , char b )
  {
    return tolower(a) == tolower(b);
  });
}

const Any& GetDotted(const std::vector<Any>& v, const AnyMap::key_type& key);

const Any& GetDotted(const AnyMap& m, const AnyMap::key_type& key)
{
  auto pos = key.find(".");
  if (pos != AnyMap::key_type::npos)
  {
    auto head = key.substr(0, pos);
    auto tail = key.substr(pos + 1);

    auto& h = m.at(head);
    if (h.Type() == typeid(AnyMap))
    {
      return GetDotted(ref_any_cast<AnyMap>(h), tail);
    }
    else if (h.Type() == typeid(std::vector<Any>))
    {
      return GetDotted(ref_any_cast<std::vector<Any>>(h), tail);
    }
    throw std::invalid_argument("Unsupported Any type at '" + head + "' for dotted get");
  }
  else
  {
    return m.at(key);
  }
}

const Any& GetDotted(const std::vector<Any>& v, const AnyMap::key_type& key)
{
  auto pos = key.find(".");
  if (pos != AnyMap::key_type::npos)
  {
    auto head = key.substr(0, pos);
    auto tail = key.substr(pos + 1);

    const int index = std::stoi(head);
    auto& h = v.at(index < 0 ? v.size() + index : index);

    if (h.Type() == typeid(AnyMap))
    {
      return GetDotted(ref_any_cast<AnyMap>(h), tail);
    }
    else if (h.Type() == typeid(std::vector<Any>))
    {
      return GetDotted(ref_any_cast<std::vector<Any>>(h), tail);
    }
    throw std::invalid_argument("Unsupported Any type at '" + head + "' for dotted get");
  }
  else
  {
    const int index = std::stoi(key);
    return v.at(index < 0 ? v.size() + index : index);
  }
}

}

// ----------------------------------------------------------------
// -------------------  AnyMap::ConstIterator  --------------------

AnyMap::ConstIterator::ConstIterator()
{}

AnyMap::ConstIterator::ConstIterator(const iterator& it)
  : IteratorBase(it.type)
{
  switch (type)
  {
  case ORDERED:
    new (iter.o) OConstIter(it.o_it()); break;
  case UNORDERED:
    new (iter.uo) UOConstIter(it.uo_it()); break;
  case UNORDERED_CI:
    new (iter.uoci) UOCIConstIter(it.uoci_it()); break;
  case NONE:
    break;
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::ConstIterator::~ConstIterator()
{
  switch (type)
  {
  case ORDERED:
    o_it().~OConstIter(); break;
  case UNORDERED:
    uo_it().~UOConstIter(); break;
  case UNORDERED_CI:
    uoci_it().~UOCIConstIter(); break;
  case NONE:
    break;
  }
}

AnyMap::ConstIterator::ConstIterator(OConstIter&& it)
  : IteratorBase(ORDERED)
{
  new (iter.o) OConstIter(std::move(it));
}

AnyMap::ConstIterator::ConstIterator(UOConstIter&& it, IterType type)
  : IteratorBase(type)
{
  switch (type)
  {
  case UNORDERED:
    new (iter.uo) UOConstIter(std::move(it)); break;
  case UNORDERED_CI:
    new (iter.uoci) UOCIConstIter(std::move(it)); break;
  default:
    throw std::logic_error("type for unordered_map iterator not supported");
  }
}

AnyMap::ConstIterator::reference AnyMap::ConstIterator::operator* () const
{
  switch (type)
  {
  case ORDERED:
    return *o_it();
  case UNORDERED:
    return *uo_it();
  case UNORDERED_CI:
    return *uoci_it();
  case NONE:
    throw std::logic_error("cannot dereference an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::ConstIterator::pointer AnyMap::ConstIterator::operator-> () const
{
  switch (type)
  {
  case ORDERED:
    return o_it().operator ->();
  case UNORDERED:
    return uo_it().operator ->();
  case UNORDERED_CI:
    return uoci_it().operator ->();
  case NONE:
    throw std::logic_error("cannot dereference an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::ConstIterator::iterator& AnyMap::ConstIterator::operator++()
{
  switch (type)
  {
  case ORDERED:
    ++o_it(); break;
  case UNORDERED:
    ++uo_it(); break;
  case UNORDERED_CI:
    ++uoci_it(); break;
  case NONE:
    throw std::logic_error("cannot increment an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }

  return *this;
}

AnyMap::ConstIterator::iterator AnyMap::ConstIterator::operator++(int)
{
  iterator tmp = *this;
  switch (type)
  {
  case ORDERED:
    o_it()++; break;
  case UNORDERED:
    uo_it()++; break;
  case UNORDERED_CI:
    uoci_it()++; break;
  case NONE:
    throw std::logic_error("cannot increment an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
  return tmp;
}

bool AnyMap::ConstIterator::operator==(const iterator& x) const
{
  switch (type)
  {
  case ORDERED:
    return o_it() == x.o_it();
  case UNORDERED:
    return uo_it() == x.uo_it();
  case UNORDERED_CI:
    return uoci_it() == x.uoci_it();
  case NONE:
    return x.type == NONE;
  default:
    throw std::logic_error("invalid iterator type");
  }
}

bool AnyMap::ConstIterator::operator!=(const iterator& x) const
{
  return !this->operator ==(x);
}

AnyMap::ConstIterator::OConstIter const& AnyMap::ConstIterator::o_it() const
{
  return *reinterpret_cast<OConstIter const*>(iter.o);
}

AnyMap::ConstIterator::OConstIter& AnyMap::ConstIterator::o_it()
{
  return *reinterpret_cast<OConstIter*>(iter.o);
}

AnyMap::ConstIterator::UOConstIter const& AnyMap::ConstIterator::uo_it() const
{
  return *reinterpret_cast<UOConstIter const*>(iter.uo);
}

AnyMap::ConstIterator::UOConstIter& AnyMap::ConstIterator::uo_it()
{
  return *reinterpret_cast<UOConstIter*>(iter.uo);
}

AnyMap::ConstIterator::UOCIConstIter const& AnyMap::ConstIterator::uoci_it() const
{
  return *reinterpret_cast<UOCIConstIter const*>(iter.uoci);
}

AnyMap::ConstIterator::UOCIConstIter& AnyMap::ConstIterator::uoci_it()
{
  return *reinterpret_cast<UOCIConstIter*>(iter.uoci);
}

// ----------------------------------------------------------------
// ---------------------  AnyMap::Iterator  -----------------------

AnyMap::Iterator::Iterator()
{}

AnyMap::Iterator::Iterator(const iterator& it)
  : IteratorBase(it.type)
{
  switch (type)
  {
  case ORDERED:
    new (iter.o) OIter(it.o_it()); break;
  case UNORDERED:
    new (iter.uo) UOIter(it.uo_it()); break;
  case UNORDERED_CI:
    new (iter.uoci) UOCIIter(it.uoci_it()); break;
  case NONE:
    break;
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::Iterator::~Iterator()
{
  switch (type)
  {
  case ORDERED:
    o_it().~OIter(); break;
  case UNORDERED:
    uo_it().~UOIter(); break;
  case UNORDERED_CI:
    uoci_it().~UOCIIter(); break;
  case NONE:
    break;
  }
}

AnyMap::Iterator::Iterator(OIter&& it)
  : IteratorBase(ORDERED)
{
  new (iter.o) OIter(std::move(it));
}

AnyMap::Iterator::Iterator(UOIter&& it, IterType type)
  : IteratorBase(type)
{
  switch (type)
  {
  case UNORDERED:
    new (iter.uo) UOIter(std::move(it)); break;
  case UNORDERED_CI:
    new (iter.uoci) UOCIIter(std::move(it)); break;
  default:
    throw std::logic_error("type for unordered_map iterator not supported");
  }
}

AnyMap::Iterator::reference AnyMap::Iterator::operator* () const
{
  switch (type)
  {
  case ORDERED:
    return *o_it();
  case UNORDERED:
    return *uo_it();
  case UNORDERED_CI:
    return *uoci_it();
  case NONE:
    throw std::logic_error("cannot dereference an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::Iterator::pointer AnyMap::Iterator::operator-> () const
{
  switch (type)
  {
  case ORDERED:
    return o_it().operator ->();
  case UNORDERED:
    return uo_it().operator ->();
  case UNORDERED_CI:
    return uoci_it().operator ->();
  case NONE:
    throw std::logic_error("cannot dereference an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
}

AnyMap::Iterator::iterator& AnyMap::Iterator::operator++()
{
  switch (type)
  {
  case ORDERED:
    ++o_it(); break;
  case UNORDERED:
    ++uo_it(); break;
  case UNORDERED_CI:
    ++uoci_it(); break;
  case NONE:
    throw std::logic_error("cannot increment an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }

  return *this;
}

AnyMap::Iterator::iterator AnyMap::Iterator::operator++(int)
{
  iterator tmp = *this;
  switch (type)
  {
  case ORDERED:
    o_it()++; break;
  case UNORDERED:
    uo_it()++; break;
  case UNORDERED_CI:
    uoci_it()++; break;
  case NONE:
    throw std::logic_error("cannot increment an invalid iterator");
  default:
    throw std::logic_error("invalid iterator type");
  }
  return tmp;
}

bool AnyMap::Iterator::operator==(const iterator& x) const
{
  switch (type)
  {
  case ORDERED:
    return o_it() == x.o_it();
  case UNORDERED:
    return uo_it() == x.uo_it();
  case UNORDERED_CI:
    return uoci_it() == x.uoci_it();
  case NONE:
    return x.type == NONE;
  default:
    throw std::logic_error("invalid iterator type");
  }
}

bool AnyMap::Iterator::operator!=(const iterator& x) const
{
  return !this->operator ==(x);
}

AnyMap::Iterator::OIter const& AnyMap::Iterator::o_it() const
{
  return *reinterpret_cast<OIter const*>(iter.o);
}

AnyMap::Iterator::OIter& AnyMap::Iterator::o_it()
{
  return *reinterpret_cast<OIter*>(iter.o);
}

AnyMap::Iterator::UOIter const& AnyMap::Iterator::uo_it() const
{
  return *reinterpret_cast<UOIter const*>(iter.uo);
}

AnyMap::Iterator::UOIter& AnyMap::Iterator::uo_it()
{
  return *reinterpret_cast<UOIter*>(iter.uo);
}

AnyMap::Iterator::UOCIIter const& AnyMap::Iterator::uoci_it() const
{
  return *reinterpret_cast<UOCIIter const*>(iter.uoci);
}

AnyMap::Iterator::UOCIIter& AnyMap::Iterator::uoci_it()
{
  return *reinterpret_cast<UOCIIter*>(iter.uoci);
}

// ----------------------------------------------------------
// -------------------------  AnyMap  -----------------------

AnyMap::AnyMap(Type type)
  : type(type)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    new (&map.o) OrderedMap();
    break;
  case Type::UNORDERED_MAP:
    new (&map.uo) UnorderedMap();
    break;
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    new (&map.uoci) UnorderedMapCaseInsensitiveKeys();
    break;
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::AnyMap(const OrderedMap& m)
  : type(Type::ORDERED_MAP)
{
  new (&map.o) OrderedMap(m);
}

AnyMap::AnyMap(const UnorderedMap& m)
  : type(Type::UNORDERED_MAP)
{
  new (&map.uo) UnorderedMap(m);
}

AnyMap::AnyMap(const UnorderedMapCaseInsensitiveKeys& m)
  : type(Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
{
  new (&map.uoci) UnorderedMapCaseInsensitiveKeys(m);
}

AnyMap::AnyMap(const AnyMap& m)
  : type(m.type)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    new (&map.o) OrderedMap(m.o_m());
    break;
  case Type::UNORDERED_MAP:
    new (&map.uo) UnorderedMap(m.uo_m());
    break;
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    new (&map.uoci) UnorderedMapCaseInsensitiveKeys(m.uoci_m());
    break;
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap& AnyMap::operator =(const AnyMap& m)
{
  if (this == &m)
    return *this;

  switch (type)
  {
  case Type::ORDERED_MAP:
    o_m().~OrderedMap();
    break;
  case Type::UNORDERED_MAP:
    uo_m().~UnorderedMap();
    break;
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    uoci_m().~UnorderedMapCaseInsensitiveKeys();
    break;
  }

  switch (m.type)
  {
  case Type::ORDERED_MAP:
    new (&map.o) OrderedMap(m.o_m());
    break;
  case Type::UNORDERED_MAP:
    new (&map.uo) UnorderedMap(m.uo_m());
    break;
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    new (&map.uoci) UnorderedMapCaseInsensitiveKeys(m.uoci_m());
    break;
  }

  return *this;
}

AnyMap::~AnyMap()
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    o_m().~OrderedMap();
    break;
  case Type::UNORDERED_MAP:
    uo_m().~UnorderedMap();
    break;
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    uoci_m().~UnorderedMapCaseInsensitiveKeys();
    break;
  }
}

AnyMap::Type AnyMap::GetType() const
{
  return type;
}

AnyMap::iterator AnyMap::begin()
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return { o_m().begin() };
  case Type::UNORDERED_MAP:
    return { uo_m().begin(), iterator::UNORDERED };
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return { uoci_m().begin(), iterator::UNORDERED_CI };
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::const_iterator AnyMap::begin() const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return { o_m().begin() };
  case Type::UNORDERED_MAP:
    return { uo_m().begin(), const_iterator::UNORDERED };
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return { uoci_m().begin(), const_iterator::UNORDERED_CI };
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::const_iterator AnyMap::cbegin() const
{
  return begin();
}

AnyMap::iterator AnyMap::end()
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return { o_m().end() };
  case Type::UNORDERED_MAP:
    return { uo_m().end(), iterator::UNORDERED };
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return { uoci_m().end(), iterator::UNORDERED_CI };
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::const_iterator AnyMap::end() const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return { o_m().end() };
  case Type::UNORDERED_MAP:
    return { uo_m().end(), const_iterator::UNORDERED };
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return { uoci_m().end(), const_iterator::UNORDERED_CI };
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::const_iterator AnyMap::cend() const
{
  return end();
}

bool AnyMap::empty() const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().empty();
  case Type::UNORDERED_MAP:
    return uo_m().empty();
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().empty();
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::size_type AnyMap::size() const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().size();
  case Type::UNORDERED_MAP:
    return uo_m().size();
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().size();
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::size_type AnyMap::count(const AnyMap::key_type& key) const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().count(key);
  case Type::UNORDERED_MAP:
    return uo_m().count(key);
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().count(key);
  default:
    throw std::logic_error("invalid map type");
  }
}

void AnyMap::clear()
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().clear();
  case Type::UNORDERED_MAP:
    return uo_m().clear();
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().clear();
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::mapped_type& AnyMap::at(const key_type& key)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().at(key);
  case Type::UNORDERED_MAP:
    return uo_m().at(key);
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().at(key);
  default:
    throw std::logic_error("invalid map type");
  }
}

const AnyMap::mapped_type& AnyMap::at(const AnyMap::key_type& key) const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m().at(key);
  case Type::UNORDERED_MAP:
    return uo_m().at(key);
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m().at(key);
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::mapped_type& AnyMap::operator[](const AnyMap::key_type& key)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m()[key];
  case Type::UNORDERED_MAP:
    return uo_m()[key];
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m()[key];
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::mapped_type& AnyMap::operator[](AnyMap::key_type&& key)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return o_m()[std::move(key)];
  case Type::UNORDERED_MAP:
    return uo_m()[std::move(key)];
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return uoci_m()[std::move(key)];
  default:
    throw std::logic_error("invalid map type");
  }
}

std::pair<AnyMap::iterator, bool> AnyMap::insert(const value_type& value)
{
  switch (type)
  {
  case Type::ORDERED_MAP:
  {
    auto p = o_m().insert(value);
    return { iterator(std::move(p.first)), p.second };
  }
  case Type::UNORDERED_MAP:
  {
    auto p = uo_m().insert(value);
    return { iterator(std::move(p.first), iterator::UNORDERED), p.second };
  }
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
  {
    auto p = uoci_m().insert(value);
    return { iterator(std::move(p.first), iterator::UNORDERED_CI), p.second };
  }
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::const_iterator AnyMap::find(const key_type& key) const
{
  switch (type)
  {
  case Type::ORDERED_MAP:
    return { o_m().find(key) };
  case Type::UNORDERED_MAP:
    return { uo_m().find(key), const_iterator::UNORDERED };
  case Type::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
    return { uoci_m().find(key), const_iterator::UNORDERED_CI };
  default:
    throw std::logic_error("invalid map type");
  }
}

AnyMap::mapped_type& AnyMap::GetDotted(const AnyMap::key_type& key)
{
  return const_cast<mapped_type&>(static_cast<const AnyMap*>(this)->GetDotted(key));
}

const AnyMap::mapped_type& AnyMap::GetDotted(const AnyMap::key_type& key) const
{
  return detail::GetDotted(*this, key);
}

AnyMap::OrderedMap const& AnyMap::o_m() const
{
  return *reinterpret_cast<OrderedMap const*>(map.o);
}

AnyMap::OrderedMap& AnyMap::o_m()
{
  return *reinterpret_cast<OrderedMap*>(map.o);
}

AnyMap::UnorderedMap const& AnyMap::uo_m() const
{
  return *reinterpret_cast<UnorderedMap const*>(map.uo);
}

AnyMap::UnorderedMap& AnyMap::uo_m()
{
  return *reinterpret_cast<UnorderedMap*>(map.uo);
}

AnyMap::UnorderedMapCaseInsensitiveKeys const& AnyMap::uoci_m() const
{
  return *reinterpret_cast<UnorderedMapCaseInsensitiveKeys const*>(map.uoci);
}

AnyMap::UnorderedMapCaseInsensitiveKeys& AnyMap::uoci_m()
{
  return *reinterpret_cast<UnorderedMapCaseInsensitiveKeys*>(map.uoci);
}

template<>
std::ostream& any_value_to_string(std::ostream& os, const AnyMap& m)
{
  os << "{";
  typedef AnyMap::const_iterator Iterator;
  Iterator i1 = m.begin();
  const Iterator begin = i1;
  const Iterator end = m.end();
  for ( ; i1 != end; ++i1)
  {
    if (i1 == begin) os << i1->first << " : " << i1->second.ToString();
    else os << ", " << i1->first << " : " << i1->second.ToString();
  }
  os << "}";
  return os;
}

}
