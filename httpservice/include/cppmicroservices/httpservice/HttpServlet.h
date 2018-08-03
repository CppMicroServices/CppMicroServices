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

#ifndef CPPMICROSERVICES_HTTPSERVLET_H
#define CPPMICROSERVICES_HTTPSERVLET_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <mutex>

namespace cppmicroservices {

class HttpServletRequest;
class HttpServletResponse;
class ServletContext;
class ServletConfig;

struct HttpServletPrivate;

class US_HttpService_EXPORT HttpServlet
  : public std::enable_shared_from_this<HttpServlet>
{
public:
  static const std::string PROP_CONTEXT_ROOT;

  HttpServlet();

  /**
   * Called by the servlet container to indicate to a servlet that the
   * servlet is being placed into service.
   *
   * The servlet container calls the <code>Init</code>
   * method exactly once after instantiating the servlet.
   * The <code>Init</code> method must complete successfully
   * before the servlet can receive any requests.
   *
   * The servlet container cannot place the servlet into service
   * if the <code>Init</code> method
   * <ol>
   * <li>Throws a <code>ServletException</code>
   * <li>Does not return within a time period defined by the Web server
   * </ol>
   *
   *
   * @param config   a <code>ServletConfig</code> object
   *     containing the servlet's
   *      configuration and initialization parameters
   *
   * @exception ServletException  if an exception has occurred that
   *     interferes with the servlet's normal
   *     operation
   *
   * @see     UnavailableException
   * @see     #GetServletConfig
   *
   */
  virtual void Init(const ServletConfig& config);

  /**
   *
   * Called by the servlet container to indicate to a servlet that the
   * servlet is being taken out of service.  This method is
   * only called once all threads within the servlet's
   * <code>service</code> method have exited or after a timeout
   * period has passed. After the servlet container calls this
   * method, it will not call the <code>service</code> method again
   * on this servlet.
   *
   * This method gives the servlet an opportunity
   * to clean up any resources that are being held (for example, memory,
   * file handles, threads) and make sure that any persistent state is
   * synchronized with the servlet's current state in memory.
   *
   */
  virtual void Destroy();

  /**
   * Returns a ServletConfig object, which contains
   * initialization and startup parameters for this servlet.
   * The <code>ServletConfig</code> object returned is the one
   * passed to the <code>Init</code> method.
   *
   * @return  the <code>ServletConfig</code> object
   *   that initializes this servlet
   *
   * @see   #Init
   *
   */
  ServletConfig GetServletConfig() const;

  virtual void Service(HttpServletRequest& request,
                       HttpServletResponse& response);

  std::shared_ptr<ServletContext> GetServletContext() const;

  virtual ~HttpServlet();

protected:
  virtual long long GetLastModified(HttpServletRequest& request);

  virtual void DoGet(HttpServletRequest& request,
                     HttpServletResponse& response);
  virtual void DoHead(HttpServletRequest& request,
                      HttpServletResponse& response);
  virtual void DoDelete(HttpServletRequest& request,
                        HttpServletResponse& response);
  virtual void DoPost(HttpServletRequest& request,
                      HttpServletResponse& response);
  virtual void DoPut(HttpServletRequest& request,
                     HttpServletResponse& response);

  virtual void DoTrace(HttpServletRequest& request,
                       HttpServletResponse& response);

  std::unique_lock<std::mutex> Lock() const;

private:
  HttpServletPrivate* d;

  friend class ServletHandler;
  //friend class ServletContainerPrivate;
};
}

#endif // CPPMICROSERVICES_HTTPSERVLET_H
