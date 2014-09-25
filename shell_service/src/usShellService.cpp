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

#include "usShellService.h"

#include "usModuleRegistry.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include "scheme.h"
#include "scheme-private.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <map>
#include <set>

#include <malloc.h>

US_USE_NAMESPACE

extern "C" {

pointer lm(scheme* sc, pointer /*args*/)
{
  std::vector<Module*> modules = ModuleRegistry::GetLoadedModules();
  std::stringstream ss;
  pointer result = sc->NIL;
  for (std::vector<Module*>::reverse_iterator iter = modules.rbegin(),
       iterEnd = modules.rend(); iter != iterEnd; ++iter)
  {
    result = cons(sc, sc->vptr->mk_integer(sc, (*iter)->GetModuleId()), result);
  }
  return result;
}

}

US_BEGIN_NAMESPACE

struct ShellService::Impl
{
  Impl() : m_Scheme(scheme_init_new()) {}
  ~Impl() { free(m_Scheme); }

  void InitSymbols();

  scheme* m_Scheme;

  std::map<char, std::set<std::string> > m_Symbols;
};

void ShellService::Impl::InitSymbols()
{
  //std::cout << "Calling oblist" << std::endl;
  pointer result = scheme_apply0(m_Scheme, "oblist");
  if (m_Scheme->vptr->is_list(m_Scheme, result))
  {
    int len = m_Scheme->vptr->list_length(m_Scheme, result);
    //std::cout << "Result is a list with len = " << len << std::endl;
    for (int i = 0; i < len; ++i)
    {
      pointer elem = m_Scheme->vptr->pair_car(result);
      if (m_Scheme->vptr->is_list(m_Scheme, elem))
      {
        int len2 = m_Scheme->vptr->list_length(m_Scheme, elem);
        if (len2 == 1)
        {
          pointer symElem = m_Scheme->vptr->pair_car(elem);
          if (m_Scheme->vptr->is_symbol(symElem))
          {
            const char* symName = m_Scheme->vptr->symname(symElem);
            if (symName)
            {
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
  if (d->m_Scheme == NULL)
  {
    throw std::runtime_error("Could not initialize Scheme interpreter");
  }
  scheme_set_output_port_file(d->m_Scheme, stdout);

  ModuleResource schemeInitRes = GetModuleContext()->GetModule()->GetResource("tinyscheme/init.scm");
  if (schemeInitRes)
  {
    ModuleResourceStream schemeInit(schemeInitRes);
    int schemeInitBufLen = schemeInitRes.GetSize() + 1;
    char* schemeInitBuf = new char[schemeInitBufLen];
    schemeInit.read(schemeInitBuf, schemeInitBufLen);
    if (schemeInit.eof() || schemeInit.good())
    {
      schemeInitBuf[schemeInit.tellg()] = '\0';
      scheme_load_string(d->m_Scheme, schemeInitBuf);
    }
    else
    {
      US_WARN << "Could not read Scheme init.scm file from resource";
    }
    delete[] schemeInitBuf;
  }
  else
  {
    US_WARN << "Scheme file init.scm not found";
  }

  scheme_define(d->m_Scheme, d->m_Scheme->global_env, mk_symbol(d->m_Scheme, "lm"), mk_foreign_func(d->m_Scheme, lm));
}

ShellService::~ShellService()
{
  scheme_deinit(d->m_Scheme);
}

void ShellService::ExecuteCommand(const std::string& cmd)
{
  scheme_load_string(d->m_Scheme, cmd.c_str());
}

std::vector<std::string> ShellService::GetCompletions(const std::string& in)
{
  std::vector<std::string> result;

  if (in.empty())
  {
    return result;
  }

  if (d->m_Symbols.empty())
  {
    d->InitSymbols();
  }

  std::size_t pos = in.find_first_not_of(" (");
  std::string prefix = in.substr(0, pos);
  std::string cmd = in.substr(pos);

  if (cmd.empty()) return result;

  std::map<char, std::set<std::string> >::const_iterator iter = d->m_Symbols.find(cmd[0]);
  if (iter == d->m_Symbols.end())
  {
    return result;
  }

  for (std::set<std::string>::const_iterator symIter = iter->second.begin(), symIterEnd = iter->second.end();
       symIter != symIterEnd; ++symIter)
  {
    if (symIter->size() < cmd.size()) continue;
    if (symIter->compare(0, cmd.size(), cmd) == 0)
    {
      result.push_back(prefix + *symIter);
    }
  }
  return result;
}

US_END_NAMESPACE
