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


#include "usBundle.h"

#include "usBundleEvent.h"
#include "usBundleContextPrivate.h"
#include "usBundleActivator.h"
#include "usBundleArchive_p.h"
#include "usBundlePrivate.h"
#include "usBundleResource.h"
#include "usBundleThread_p.h"
#include "usBundleUtils_p.h"
#include "usCoreBundleContext_p.h"
#include "usResolver_p.h"

#include "usCoreConfig.h"

#include "usSharedLibrary.h"
#include "usUtils_p.h"

#include <thread>
#include <chrono>

namespace us {

Bundle::Bundle(const Bundle & b)
  : d(b.d)
  , c(b.c)
{
}

Bundle::Bundle(Bundle&& b)
  : d(std::move(b.d))
  , c(std::move(b.c))
{
}

Bundle& Bundle::operator=(const Bundle& b)
{
  this->d = b.d;
  this->c = b.c;
  return *this;
}

Bundle& Bundle::operator=(Bundle&& b)
{
  this->d = std::move(b.d);
  this->c = std::move(b.c);
  return *this;
}

Bundle::Bundle()
{
}

bool Bundle::operator==(const Bundle& rhs) const
{
  return *this ? (rhs ? d->coreCtx->id == rhs.d->coreCtx->id && d->id == rhs.d->id : false) : !rhs;
}

bool Bundle::operator!=(const Bundle& rhs) const
{
  return !(*this == rhs);
}

bool Bundle::operator<(const Bundle& rhs) const
{
  return *this ? (rhs ? (d->coreCtx->id == rhs.d->coreCtx->id ? d->id < rhs.d->id : d->coreCtx->id < rhs.d->coreCtx->id) : true) : false;
}

Bundle::operator bool() const
{
  return d != nullptr;
}

Bundle& Bundle::operator=(std::nullptr_t)
{
  d = nullptr;
  c = nullptr;
  return *this;
}

Bundle::Bundle(const std::shared_ptr<BundlePrivate>& d)
  : d(d)
  , c(d ? d->coreCtx->shared_from_this() : nullptr)
{
}

Bundle::~Bundle()
{
}

Bundle::State Bundle::GetState() const
{
  return static_cast<State>(d->state.load());
}

void Bundle::Start()
{
  d->Start(0);
}

void Bundle::Start(uint32_t options)
{
  d->Start(options);
}

void Bundle::Stop()
{
  Stop(0);
}

void Bundle::Stop(uint32_t options)
{
  d->Stop(options);
}

void Bundle::Uninstall()
{
  {
    auto l = d->coreCtx->resolver.Lock(); US_UNUSED(l);
    //BundleGeneration current = current();

    switch (static_cast<Bundle::State>(d->state.load()))
    {
    case STATE_UNINSTALLED:
      throw std::logic_error("Bundle is in UNINSTALLED state");
    case STATE_STARTING: // Lazy start
    case STATE_ACTIVE:
    case STATE_STOPPING:
    {
      std::exception_ptr exception;
      try
      {
        d->WaitOnOperation(d->coreCtx->resolver, l, "Bundle::Uninstall", true);
        exception = (d->state & (STATE_ACTIVE | STATE_STARTING)) != 0 ? d->Stop0(l) : nullptr;
      }
      catch (...)
      {
        // Force to install
        d->SetStateInstalled(false, l);
        d->coreCtx->resolver.NotifyAll();
        exception = std::current_exception();
      }
      d->operation = BundlePrivate::OP_UNINSTALLING;
      if (exception != nullptr)
      {
        try { std::rethrow_exception(exception); }
        catch (const std::exception& e)
        {
          // $TODO framework event
          //coreCtx->FrameworkError(this, exception);
          US_WARN << e.what();
        }
      }
      // Fall through
    }
    case STATE_RESOLVED:
    case STATE_INSTALLED:
    {
      d->coreCtx->bundleRegistry.Remove(d->location, d->id);
      if (d->operation != BundlePrivate::OP_UNINSTALLING)
      {
        try
        {
          d->WaitOnOperation(d->coreCtx->resolver, l, "Bundle::Uninstall", true);
          d->operation = BundlePrivate::OP_UNINSTALLING;
        }
        catch (const std::exception& )
        {
          // Make sure that bundleContext is invalid
          std::shared_ptr<BundleContextPrivate> ctx;
          if ((ctx = d->bundleContext.Exchange(ctx)))
          {
            ctx->Invalidate();
          }
          d->operation = BundlePrivate::OP_UNINSTALLING;
          // $TODO framework error
          //d->coreCtx->FrameworkError(this, e);
        }
      }
      if (d->state == STATE_UNINSTALLED)
      {
        d->operation = BundlePrivate::OP_IDLE;
        throw std::logic_error("Bundle is in UNINSTALLED state");
      }

      d->state = STATE_INSTALLED;
      d->GetBundleThread()->BundleChanged(
            BundleEvent(
              BundleEvent::UNRESOLVED,
              Bundle(d)
              ),
            l);
      d->bactivator = nullptr;
      d->state = STATE_UNINSTALLED;
      // Purge old archive
      //BundleGeneration oldGen = current;
      //generations.set(0, new BundleGeneration(oldGen));
      //oldGen.purge(false);
      d->Purge();
      d->barchive->SetLastModified(Clock::now());
      d->operation = BundlePrivate::OP_IDLE;
      if (!d->bundleDir.empty())
      {
        try
        {
          if (fs::Exists(d->bundleDir)) fs::RemoveDirectoryRecursive(d->bundleDir);
        }
        catch (const std::exception& e)
        {
          // $TODO framework error
          // d->coreCtx->FrameworkError(this,
          //                     "Failed to delete bundle data" + e.what());
          US_WARN << "Failed to delete bundle data" << e.what();
        }
        d->bundleDir.clear();
      }
      // id, location and headers survives after uninstall.

      // There might be bundle threads that are running start or stop
      // operation. This will wake them and give them an chance to terminate.
      d->coreCtx->resolver.NotifyAll();
      break;
    }
    }
  }
  d->coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::UNINSTALLED, Bundle(d)));
}

BundleContext Bundle::GetBundleContext() const
{
  return MakeBundleContext(d->bundleContext.Load()->shared_from_this());
}

long Bundle::GetBundleId() const
{
  return d->id;
}

std::string Bundle::GetLocation() const
{
  return d->location;
}

std::string Bundle::GetSymbolicName() const
{
  return d->symbolicName;
}

BundleVersion Bundle::GetVersion() const
{
  return d->version;
}

std::map<std::string, Any> Bundle::GetProperties() const
{
  return d->bundleManifest.GetProperties();
}

Any Bundle::GetProperty(const std::string& key) const
{
  Any property(d->bundleManifest.GetValue(key));

  // Clients must be able to query both a bundle's properties
  // and the framework's properties through any Bundle's
  // GetProperty function.
  // The Framework's properties include both the launch properties
  // used to initialize the Framework and all relevant
  // "org.cppmicroservices.*" properties.
  if (property.Empty())
  {
    auto props = d->coreCtx->frameworkProperties.find(key);
    if (props != d->coreCtx->frameworkProperties.end())
    {
      property = (*props).second;
    }
  }
  return property;
}

std::vector<std::string> Bundle::GetPropertyKeys() const
{
  return d->bundleManifest.GetKeys();
}

std::vector<ServiceReferenceU> Bundle::GetRegisteredServices() const
{
  d->CheckUninstalled();
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetRegisteredByBundle(d.get(), sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

std::vector<ServiceReferenceU> Bundle::GetServicesInUse() const
{
  d->CheckUninstalled();
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetUsedByBundle(d.get(), sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

BundleResource Bundle::GetResource(const std::string& path) const
{
  d->CheckUninstalled();
  return d->barchive->GetResource(path);
}

std::vector<BundleResource> Bundle::FindResources(const std::string& path, const std::string& filePattern,
                                                  bool recurse) const
{
  d->CheckUninstalled();
  return d->barchive->FindResources(path, filePattern, recurse);
}

Bundle::TimeStamp Bundle::GetLastModified() const
{
  return d->barchive ? d->barchive->GetLastModified() : d->timeStamp;
}

std::ostream& operator<<(std::ostream& os, Bundle::State state)
{
  switch (state)
  {
  case Bundle::STATE_UNINSTALLED: return os << "UNINSTALLED";
  case Bundle::STATE_INSTALLED: return os << "INSTALLED";
  case Bundle::STATE_RESOLVED: return os << "RESOLVED";
  case Bundle::STATE_STARTING: return os << "STARTING";
  case Bundle::STATE_ACTIVE: return os << "ACTIVE";
  case Bundle::STATE_STOPPING: return os << "STOPPING";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Bundle& bundle)
{
  os << "Bundle[" << "id=" << bundle.GetBundleId() <<
        ", loc=" << bundle.GetLocation() <<
        ", name=" << bundle.GetSymbolicName() <<
        ", state=" << bundle.GetState() << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, Bundle const * bundle)
{
  return operator<<(os, *bundle);
}

}
