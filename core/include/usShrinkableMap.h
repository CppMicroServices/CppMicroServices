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

#ifndef USSHRINKABLEMAP_H
#define USSHRINKABLEMAP_H

#include "usGlobalConfig.h"

#include <map>

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServicesUtils
 *
 * A std::map style associative container allowing query and removal
 * operations only.
 */
template<class Key, class T>
class ShrinkableMap
{
private:
  static std::map<Key,T> emptyContainer;

public:

  typedef std::map<Key,T> container_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::key_type key_type;
  typedef typename container_type::mapped_type mapped_type;
  typedef typename container_type::value_type value_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;

  ShrinkableMap()
    : container(emptyContainer)
  {
  }

  iterator begin()
  {
    return container.begin();
  }

  const_iterator begin() const
  {
    return container.begin();
  }

  iterator end()
  {
    return container.end();
  }

  const_iterator end() const
  {
    return container.end();
  }

  void erase(iterator pos)
  {
    return container.erase(pos);
  }

  void erase(iterator first, iterator last)
  {
    return container.erase(first, last);
  }

  size_type erase(const Key& key)
  {
    return container.erase(key);
  }

  bool empty() const
  {
    return container.empty();
  }

  void clear()
  {
    container.clear();
  }

  size_type size() const
  {
    return container.size();
  }

  size_type max_size() const
  {
    return container.max_size();
  }

  T& operator[](const Key& key)
  {
    return container[key];
  }

  size_type count(const Key& key) const
  {
    return container.count(key);
  }

  iterator find(const Key& key)
  {
    return container.find(key);
  }

  const_iterator find(const Key& key) const
  {
    return container.find(key);
  }

  std::pair<iterator,iterator> equal_range(const Key& key)
  {
    return container.equal_range(key);
  }

  std::pair<const_iterator,const_iterator> equal_range(const Key& key) const
  {
    return container.equal_range(key);
  }

  iterator lower_bound(const Key& key)
  {
    return container.lower_bound(key);
  }

  const_iterator lower_bound(const Key& key) const
  {
    return container.lower_bound(key);
  }

  iterator upper_bound(const Key& key)
  {
    return container.upper_bound(key);
  }

  const_iterator upper_bound(const Key& key) const
  {
    return container.upper_bound(key);
  }

private:

  friend class ServiceHooks;

  ShrinkableMap(container_type& container)
    : container(container)
  {}

  container_type& container;
};

template<class Key, class T>
std::map<Key,T> ShrinkableMap<Key,T>::emptyContainer;

US_END_NAMESPACE

#endif // USSHRINKABLEMAP_H
