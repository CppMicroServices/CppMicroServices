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
#ifndef REFERENCEAUTOGENFILES_HPP
#define REFERENCEAUTOGENFILES_HPP

namespace codegen
{

#if NEVER
    const std::string REF_SRC = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "ServiceComponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;
using scd::Binder;
using scd::StaticBinder;
using scd::DynamicBinder;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<test::ISpellCheckService>, std::shared_ptr<test::IDictionaryService>>({{"dictionary"}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";
#else
    const std::string REF_SRC = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>, std::shared_ptr<DictionaryService::IDictionaryService>>({{"dictionary"}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";
#endif

    const std::string REF_SRC_DYN = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  binders.push_back(std::make_shared<scd::DynamicBinder<DSSpellCheck::SpellCheckImpl, DictionaryService::IDictionaryService>>("dictionary", &DSSpellCheck::SpellCheckImpl::Binddictionary, &DSSpellCheck::SpellCheckImpl::Unbinddictionary));
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>>({{}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_SRC_DYN_INJ_SOME = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  binders.push_back(std::make_shared<scd::DynamicBinder<DSSpellCheck::SpellCheckImpl, DictionaryService::IDictionaryService>>("dictionary1", &DSSpellCheck::SpellCheckImpl::Binddictionary1, &DSSpellCheck::SpellCheckImpl::Unbinddictionary1));
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>>({{}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_SRC_STAT_INJ_SOME = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>, std::shared_ptr<DictionaryService::IDictionaryService>>({{"dictionary1"}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_SRC_STAT_DYN_INJ_SOME = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  binders.push_back(std::make_shared<scd::DynamicBinder<DSSpellCheck::SpellCheckImpl, DictionaryService::IDictionaryService>>("dictionary0", &DSSpellCheck::SpellCheckImpl::Binddictionary0, &DSSpellCheck::SpellCheckImpl::Unbinddictionary0));
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>, std::shared_ptr<DictionaryService::IDictionaryService>>({{"dictionary1"}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_MULT_COMPS = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_Foo_Impl1()
{
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<Foo::Impl1, std::tuple<Foo::Interface>>();

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_Foo_Impl1(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}


extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_Foo_Impl2()
{
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<Foo::Impl2, std::tuple<Foo::Interface>>();

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_Foo_Impl2(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_MULT_COMPS_SAME_IMPL = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_FooImpl1()
{
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<Foo::Impl1, std::tuple<Foo::Interface>>();

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_FooImpl1(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}


extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_FooImpl2()
{
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<Foo::Impl1, std::tuple<Foo::Interface>>();

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_FooImpl2(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

    const std::string REF_MULT_CARD = R"manifestsrc(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"
#include "SpellCheckerImpl.hpp"

namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;

extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_DSSpellCheck_SpellCheckImpl()
{
  std::vector<std::shared_ptr<scd::Binder<DSSpellCheck::SpellCheckImpl>>> binders;
  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<DSSpellCheck::SpellCheckImpl, std::tuple<SpellCheck::ISpellCheckService>, std::vector<std::shared_ptr<DictionaryService::IDictionaryService>>, std::shared_ptr<Foo::Interface>>({{"dictionary", "foo"}}, binders);

  return componentInstance;
}

extern "C" US_ABI_EXPORT void DeleteInstance_DSSpellCheck_SpellCheckImpl(scd::ComponentInstance* componentInstance)
{
  delete componentInstance;
}

)manifestsrc";

} // namespace codegen

#endif //  REFERENCEAUTOGENFILES_HPP
