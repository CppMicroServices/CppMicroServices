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

#ifndef USSERVLETCONTAINER_P_H
#define USSERVLETCONTAINER_P_H

#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

class CivetServer;

US_BEGIN_NAMESPACE

class ModuleContext;

class HttpServlet;
class ServletContainer;
class ServletContext;
class ServletHandler;

struct ServletContainerPrivate : private ServiceTrackerCustomizer<HttpServlet, ServletHandler*>
{
  typedef ServiceTrackerCustomizer<HttpServlet, ServletHandler*>::TrackedType TrackerType;

  ServletContainerPrivate(ServletContainer* q);

  void Start();
  void Stop();

  std::string GetMimeType(const ServletContext* context, const std::string& file) const;

  ModuleContext* m_Context;
  CivetServer* m_Server;
  ServiceTracker<HttpServlet, TrackedTypeTraits<HttpServlet, ServletHandler*> > m_ServletTracker;

  std::map<std::string, ServletContext*> m_ServletContextMap;
  std::string m_ContextPath;

private:

  ServletContainer* const q;
  std::list<ServletHandler*> m_Handler;

  virtual TrackedType AddingService(const ServiceReferenceType& reference);
  virtual void ModifiedService(const ServiceReferenceType& /*reference*/, TrackedType /*service*/);
  virtual void RemovedService(const ServiceReferenceType& reference, TrackedType handler);
};

US_END_NAMESPACE

#endif // USSERVLETCONTAINER_P_H
