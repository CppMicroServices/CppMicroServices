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


#ifndef USFUNCTOR_H
#define USFUNCTOR_H

#include <usConfig.h>

US_BEGIN_NAMESPACE

template<typename Arg>
class FunctorImpl
{
public:

  virtual void operator()(Arg) = 0;
  virtual FunctorImpl* Clone() const = 0;

  bool operator==(const FunctorImpl& o) const
  { return typeid(*this) == typeid(o) && IsEqual(o); }

  virtual ~FunctorImpl() {}

private:

  virtual bool IsEqual(const FunctorImpl& o) const = 0; 
};

template<typename Arg, typename Fun>
class FunctorHandler : public FunctorImpl<Arg>
{
public:

  FunctorHandler(const Fun& fun) : m_Fun(fun) {}

  FunctorHandler* Clone() const
  { return new FunctorHandler(*this); }

  void operator()(Arg a)
  { m_Fun(a); }

private:

  bool IsEqual(const FunctorImpl<Arg>& o) const
  { return this->m_Fun == static_cast<const FunctorHandler&>(o).m_Fun; }

  Fun m_Fun;
};

template<typename Arg, typename PointerToObj, typename PointerToMemFn>
class MemFunHandler : public FunctorImpl<Arg>
{
public:

  MemFunHandler(const PointerToObj& pObj, PointerToMemFn pMemFn)
    : m_pObj(pObj), m_pMemFn(pMemFn)
  {}

  MemFunHandler* Clone() const
  { return new MemFunHandler(*this); }

  void operator()(Arg a)
  { ((*m_pObj).*m_pMemFn)(a); }

private:

  bool IsEqual(const FunctorImpl<Arg>& o) const
  { return this->m_pObj == static_cast<const MemFunHandler&>(o).m_pObj &&
           this->m_pMemFn == static_cast<const MemFunHandler&>(o).m_pMemFn; }

  PointerToObj m_pObj;
  PointerToMemFn m_pMemFn;

};

template<typename Arg>
class Functor
{
public:

  Functor() : m_Impl(0) {}

  template<class Fun>
  Functor(const Fun& fun)
    : m_Impl(new FunctorHandler<Arg, Fun>(fun))
  {}

  template<class PtrObj, typename MemFn>
  Functor(const PtrObj& p, MemFn memFn)
    : m_Impl(new MemFunHandler<Arg, PtrObj, MemFn>(p, memFn))
  {}

  Functor(const Functor& f) : m_Impl(f.m_Impl->Clone()) {}

  Functor& operator=(const Functor& f)
  {
    Impl* tmp = f.m_Impl->Clone();
    std::swap(tmp, m_Impl);
    delete tmp;
  }

  bool operator==(const Functor& f) const
  { return (*m_Impl) == (*f.m_Impl); }

  ~Functor() { delete m_Impl; }

  void operator()(Arg a)
  {
    (*m_Impl)(a);
  }

private:

  typedef FunctorImpl<Arg> Impl;
  Impl* m_Impl;
};

US_END_NAMESPACE

#endif // USFUNCTOR_H
