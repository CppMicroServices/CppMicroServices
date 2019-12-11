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

#include "cppmicroservices/shellservice/ShellService.h"
#include "cppmicroservices/GetBundleContext.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"

#include "scheme-private.h"
#include "scheme.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace cppmicroservices;

#define sc_int(sc, ival) (sc->vptr->mk_integer(sc, ival))
#define sc_string(sc, sval) (sc->vptr->mk_string(sc, sval))

namespace {

Bundle get_bundle(const std::string& bsn)
{
  for (auto b : GetBundleContext().GetBundles()) {
    if (b.GetSymbolicName() == bsn)
      return b;
  }
  return {};
}
}

extern "C"
{

  // id, name, version, state
  static const int numFields = 5;
  static const int fieldWidth[numFields] = { 4, 26, 10, 12, 40 };

  pointer us_bundle_ids(scheme* sc, pointer /*args*/)
  {
    auto bundles = GetBundleContext().GetBundles();
    std::set<long> ids;
    for (auto const& b : bundles) {
      ids.insert(b.GetBundleId());
    }
    pointer result = sc->NIL;
    for (auto iter = ids.rbegin(), iterEnd = ids.rend(); iter != iterEnd;
         ++iter) {
      result = cons(sc, sc_int(sc, *iter), result);
    }
    return result;
  }

  pointer us_bundle_info(scheme* sc, pointer args)
  {
    if (args == sc->NIL) {
      std::cerr << "Empty argument list" << std::endl;
      return sc->NIL;
    }

    if (sc->vptr->list_length(sc, args) != 1) {
      return sc->NIL;
    }

    const char delimChar = '-';
    char delim[50];
    memset(delim, delimChar, 50);

    pointer arg = pair_car(args);
    Bundle bundle;
    if (is_string(arg)) {
      std::string name = sc->vptr->string_value(arg);
      if (name == "_header_") {
        pointer result = sc->NIL;
        pointer infoList = sc->NIL;
        for (int fi = numFields - 1; fi >= 0; --fi) {
          delim[fieldWidth[fi]] = '\0';
          infoList = immutable_cons(sc, sc_string(sc, delim), infoList);
          delim[fieldWidth[fi]] = delimChar;
        }
        result = immutable_cons(sc, infoList, result);

        infoList = sc->NIL;
        infoList = immutable_cons(sc, sc_string(sc, "Location"), infoList);
        infoList = immutable_cons(sc, sc_string(sc, "State"), infoList);
        infoList = immutable_cons(sc, sc_string(sc, "Version"), infoList);
        infoList = immutable_cons(sc, sc_string(sc, "Symbolic Name"), infoList);
        infoList = immutable_cons(sc, sc_string(sc, "Id"), infoList);
        return immutable_cons(sc, infoList, result);
      } else {
        bundle = get_bundle(name);
      }
    } else if (is_integer(arg)) {
      bundle = GetBundleContext().GetBundle(ivalue(arg));
    } else {
      return sc->NIL;
    }

    if (!bundle) {
      return sc->NIL;
    }

    pointer id = sc_int(sc, bundle.GetBundleId());
    pointer name = sc_string(sc, bundle.GetSymbolicName().c_str());
    pointer location = sc_string(sc, bundle.GetLocation().c_str());
    pointer version = sc_string(sc, bundle.GetVersion().ToString().c_str());
    std::stringstream strState;
    strState << bundle.GetState();
    pointer state = sc_string(sc, strState.str().c_str());

    pointer result = sc->NIL;
    result = immutable_cons(sc, location, result);
    result = immutable_cons(sc, state, result);
    result = immutable_cons(sc, version, result);
    result = immutable_cons(sc, name, result);
    result = immutable_cons(sc, id, result);

    // (id, name, version, state, location)
    return result;
  }

  pointer us_display_bundle_info(scheme* sc, pointer args)
  {
    if (!sc->vptr->is_list(sc, args) || args == sc->NIL) {
      std::cerr << "Expected a non-empty list" << std::endl;
      return sc->F;
    }

    args = pair_car(args);
    int count = sc->vptr->list_length(sc, args);
    for (int j = 0; j < count; ++j) {
      pointer infoList = pair_car(args);
      args = pair_cdr(args);

      if (!sc->vptr->is_list(sc, infoList) || infoList == sc->NIL) {
        std::cerr << "Expected a non-empty list" << std::endl;
        return sc->F;
      }

      int l = sc->vptr->list_length(sc, infoList);
      for (int i = 0; i < std::min(l, 4); ++i) {
        pointer arg = pair_car(infoList);
        infoList = pair_cdr(infoList);
        if (sc->vptr->is_string(arg)) {
          std::cout << std::left << std::setw(fieldWidth[i])
                    << sc->vptr->string_value(arg);
        } else if (sc->vptr->is_number(arg)) {
          std::cout << std::left << std::setw(fieldWidth[i])
                    << sc->vptr->ivalue(arg);
        }
      }
      std::cout << std::endl;
    }
    return sc->T;
  }

  pointer us_bundle_start(scheme* sc, pointer args)
  {
    if (args == sc->NIL) {
      std::cerr << "Empty argument list" << std::endl;
      return sc->F;
    }

    if (sc->vptr->list_length(sc, args) != 1) {
      return sc->F;
    }

    pointer arg = pair_car(args);

    Bundle bundle;
    if (is_string(arg)) {
      std::string name = sc->vptr->string_value(arg);
      bundle = get_bundle(name);
    } else if (is_integer(arg)) {
      bundle = GetBundleContext().GetBundle(ivalue(arg));
    }

    if (bundle) {
      try {
        bundle.Start();
        return sc->T;
      } catch (const std::exception& e) {
        std::cerr << e.what();
      }
    }

    return sc->F;
  }

  pointer us_bundle_stop(scheme* sc, pointer args)
  {
    if (args == sc->NIL) {
      std::cerr << "Empty argument list" << std::endl;
      return sc->F;
    }

    if (sc->vptr->list_length(sc, args) != 1) {
      return sc->F;
    }

    pointer arg = pair_car(args);

    Bundle bundle;
    if (is_string(arg)) {
      std::string name = sc->vptr->string_value(arg);
      bundle = get_bundle(name);
    } else if (is_integer(arg)) {
      bundle = GetBundleContext().GetBundle(ivalue(arg));
    }

    if (bundle) {
      try {
        bundle.Stop();
        return sc->T;
      } catch (const std::exception& e) {
        std::cerr << e.what();
      }
    }

    return sc->F;
  }

  pointer us_install(scheme* sc, pointer args)
  {
    if (args == sc->NIL) {
      std::cerr << "Empty argument list" << std::endl;
      return sc->F;
    }

    if (sc->vptr->list_length(sc, args) != 1) {
      return sc->F;
    }

    pointer arg = pair_car(args);
    if (is_string(arg)) {
      std::string name = sc->vptr->string_value(arg);
      try {
        GetBundleContext().InstallBundles(name);
        return sc->T;
      } catch (const std::exception& e) {
        std::cerr << e.what();
      }
    }

    return sc->F;
  }
}

namespace cppmicroservices {

struct ShellService::Impl
{
  Impl()
    : m_Scheme(scheme_init_new())
  {}
  ~Impl() { free(m_Scheme); }

  void InitSymbols();

  scheme* m_Scheme;

  std::map<char, std::set<std::string>> m_Symbols;
};

void ShellService::Impl::InitSymbols()
{
  //std::cout << "Calling oblist" << std::endl;
  pointer result = scheme_apply0(m_Scheme, "oblist");
  if (m_Scheme->vptr->is_list(m_Scheme, result)) {
    int len = m_Scheme->vptr->list_length(m_Scheme, result);
    //std::cout << "Result is a list with len = " << len << std::endl;
    for (int i = 0; i < len; ++i) {
      pointer elem = m_Scheme->vptr->pair_car(result);
      if (m_Scheme->vptr->is_list(m_Scheme, elem)) {
        int len2 = m_Scheme->vptr->list_length(m_Scheme, elem);
        if (len2 == 1) {
          pointer symElem = m_Scheme->vptr->pair_car(elem);
          if (m_Scheme->vptr->is_symbol(symElem)) {
            const char* symName = m_Scheme->vptr->symname(symElem);
            if (symName) {
              //std::cout << "Found sym: " << symName << std::endl;
              m_Symbols[symName[0]].insert(symName);
            }
          }
        }
      }
      result = m_Scheme->vptr->pair_cdr(result);
    }
  }
}

ShellService::ShellService()
  : d(new Impl)
{
  if (d->m_Scheme == nullptr) {
    throw std::runtime_error("Could not initialize Scheme interpreter");
  }
  scheme_set_output_port_file(d->m_Scheme, stdout);

  BundleResource schemeInitRes =
    GetBundleContext().GetBundle().GetResource("tinyscheme/init.scm");
  if (schemeInitRes) {
    this->LoadSchemeResource(schemeInitRes);
  } else {
    std::cerr << "Scheme file init.scm not found";
  }

  std::vector<BundleResource> schemeResources =
    GetBundleContext().GetBundle().FindResources("/", "*.scm", false);
  for (auto & schemeResource : schemeResources) {
    if (schemeResource) {
      this->LoadSchemeResource(schemeResource);
    }
  }

  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-bundle-ids"),
                mk_foreign_func(d->m_Scheme, us_bundle_ids));
  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-bundle-info"),
                mk_foreign_func(d->m_Scheme, us_bundle_info));
  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-display-bundle-info"),
                mk_foreign_func(d->m_Scheme, us_display_bundle_info));
  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-bundle-start"),
                mk_foreign_func(d->m_Scheme, us_bundle_start));
  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-bundle-stop"),
                mk_foreign_func(d->m_Scheme, us_bundle_stop));
  scheme_define(d->m_Scheme,
                d->m_Scheme->global_env,
                mk_symbol(d->m_Scheme, "us-install"),
                mk_foreign_func(d->m_Scheme, us_install));
}

ShellService::~ShellService()
{
  scheme_deinit(d->m_Scheme);
}

void ShellService::ExecuteCommand(const std::string& cmd)
{
  std::size_t pos = cmd.find_first_not_of(' ');
  if (pos == std::string::npos)
    return;

  std::string command = cmd.substr(pos);
  if (command[0] != '(') {
    command = "(" + command + ")";
  }
  scheme_load_string(d->m_Scheme, command.c_str());
}

std::vector<std::string> ShellService::GetCompletions(const std::string& in)
{
  std::vector<std::string> result;

  if (in.empty()) {
    return result;
  }

  if (d->m_Symbols.empty()) {
    d->InitSymbols();
  }

  std::size_t pos = in.find_first_not_of(" (");
  std::string prefix = in.substr(0, pos);
  std::string cmd = in.substr(pos);

  if (cmd.empty())
    return result;

  std::map<char, std::set<std::string>>::const_iterator iter =
    d->m_Symbols.find(cmd[0]);
  if (iter == d->m_Symbols.end()) {
    return result;
  }

  for (const auto & symIter : iter->second) {
    if (symIter.size() < cmd.size())
      continue;
    if (symIter.compare(0, cmd.size(), cmd) == 0) {
      result.push_back(prefix + symIter);
    }
  }
  return result;
}

void ShellService::LoadSchemeResource(const BundleResource& res)
{
  std::cout << "Reading " << res.GetResourcePath();
  BundleResourceStream resStream(res);
  int resBufLen = res.GetSize() + 1;
  auto* resBuf = new char[resBufLen];
  resStream.read(resBuf, resBufLen);
  if (resStream.eof() || resStream.good()) {
    resBuf[static_cast<std::size_t>(resStream.gcount())] = '\0';
    scheme_load_string(d->m_Scheme, resBuf);
  } else {
    std::cerr << "Could not read " << res.GetResourcePath()
              << " file from resource";
  }
  delete[] resBuf;
}
}
