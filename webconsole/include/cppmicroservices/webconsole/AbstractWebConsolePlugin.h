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

#ifndef CPPMICROSERVICES_ABSTRACTWEBCONSOLEPLUGIN_H
#define CPPMICROSERVICES_ABSTRACTWEBCONSOLEPLUGIN_H

#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/httpservice/HttpServlet.h"
#include "cppmicroservices/webconsole/WebConsoleExport.h"
#include "cppmicroservices/webconsole/mustache.hpp"

#include <string>
#include <vector>

namespace Kainjow {

US_WebConsole_EXPORT std::ostream& operator<<(std::ostream& os,
                                              const Mustache::Data&);
}

namespace cppmicroservices {

class BundleContext;
class Bundle;
class BundleResource;

class HttpServletRequest;
class HttpServletResponse;
struct WebConsoleVariableResolver;

struct AbstractWebConsolePluginPrivate;

/**
 * The Web Console can be extended by registering an OSGi service for the interface
 * HttpServlet with the service property
 * <code>org.cppmicroservices.webconsole.label</code> set to the label (last segment in the URL)
 * of the page. The respective service is called a Web Console Plugin or a plugin
 * for short.
 *
 * To help rendering the response the Web Console bundle provides two
 * options. One of the options is to extend the AbstractWebConsolePlugin overwriting
 * the RenderContent(HttpServletRequest&, HttpServletResponse&) method.
 */
class US_WebConsole_EXPORT AbstractWebConsolePlugin : public HttpServlet
{
public:
  using TemplateData = Kainjow::Mustache::Data;

  /**
   * Retrieves the label. This is the last component in the servlet path.
   *
   * @return the label.
   */
  virtual std::string GetLabel() const = 0;

  /**
   * Retrieves the title of the plug-in. It is displayed in the page header
   * and is also included in the title of the HTML document.
   *
   * @return the plugin title.
   */
  virtual std::string GetTitle() const = 0;

  /**
   * This method should return category string which will be used to render
   * the plugin in the navigation menu. Default implementation returns null,
   * which will result in the plugin link rendered as top level menu item.
   * Concrete implementations wishing to be rendered as a sub-menu item under
   * a category should override this method and return a string or define
   * <code>org.cppmicroservices.webconsole.category</code> service property.
   * Currently only single level categories are supported. So, this should be
   * a simple string.
   *
   * @return category
   */
  virtual std::string GetCategory() const;

  virtual std::shared_ptr<WebConsoleVariableResolver> GetVariableResolver(
    HttpServletRequest& request);

  virtual void SetVariableResolver(
    HttpServletRequest& request,
    const std::shared_ptr<WebConsoleVariableResolver>& resolver);

protected:
  /**
   * Detects whether this request is intended to have the headers and
   * footers of this plugin be rendered or not. This method always returns
   * <code>true</code> and may be overwritten by plugins to detect
   * from the actual request, whether or not to render the header and
   * footer.
   *
   * @param request the original request passed from the HTTP server
   * @return <code>true</code> if the page should have headers and footers rendered
   */
  virtual bool IsHtmlRequest(HttpServletRequest& request);

  /**
   * Renders the web console page for the request. This consist of the
   * following five parts called in order:
   * <ol>
   * <li>Send back a requested resource
   * <li>{@link #StartResponse(HttpServletRequest&, HttpServletResponse&)}</li>
   * <li>{@link #RenderTopNavigation(HttpServletRequest&, std::ostream&)}</li>
   * <li>{@link #RenderContent(HttpServletRequest&, HttpServletResponse&)}</li>
   * <li>{@link #EndResponse(HttpServletRequest&, std::ostream&)}</li>
   * </ol>
   *
   * \rststar
   * .. note::
   *
   *    If a resource is sent back for the request only the first
   *    step is executed. Otherwise the first step is a null-operation actually
   *    and the latter four steps are executed in order.
   * \endrststar
   *
   * If the {@link #IsHtmlRequest(HttpServletRequest&)} method returns
   * <code>false</code> only the
   * {@link #RenderContent(HttpServletRequest&, HttpServletResponse&)} method is
   * called.
   *
   * @see HttpServlet#DoGet(HttpServletRequest&, HttpServletResponse&)
   */
  virtual void DoGet(HttpServletRequest& request,
                     HttpServletResponse& response);

  /**
   * This method is used to render the content of the plug-in. It is called internally
   * from the Web Console.
   *
   * @param request the HTTP request send from the user
   * @param response the HTTP response object, where to render the plugin data.
   */
  virtual void RenderContent(HttpServletRequest& request,
                             HttpServletResponse& response) = 0;

  /**
   * This method is responsible for generating the top heading of the page.
   *
   * @param request the HTTP request coming from the user
   * @param response the HTTP response, where data is rendered
   * @return the stream that was used for generating the response.
   * @see #EndResponse(HttpServletRequest&, std::ostream&)
   */
  std::ostream& StartResponse(HttpServletRequest& request,
                              HttpServletResponse& response);

  /**
   * This method is called to generate the top level links with the available plug-ins.
   *
   * @param request the HTTP request coming from the user
   * @param writer the writer, where the HTML data is rendered
   */
  void RenderTopNavigation(HttpServletRequest& request, std::ostream& writer);

  /**
   * This method is responsible for generating the footer of the page.
   *
   * @param request the HTTP request coming from the user
   * @param writer the writer, where the HTML data is rendered
   * @see #StartResponse(HttpServletRequest&, HttpServletResponse&)
   */
  void EndResponse(HttpServletRequest& request, std::ostream& writer);

  /**
   * Returns a list of CSS reference paths or <code>null</code> if no
   * additional CSS files are provided by the plugin.
   *
   * The result is an array of strings which are used as the value of
   * the <code>href</code> attribute of the <code>&lt;link&gt;</code> elements
   * placed in the head section of the HTML generated. If the reference is
   * a relative path, it is turned into an absolute path by prepending the
   * value of the WebConsoleConstants#ATTR_APP_ROOT request attribute.
   *
   * @return The list of additional CSS files to reference in the head
   *      section or an empty list if no such CSS files are required.
   */
  std::vector<std::string> GetCssReferences() const;

protected:
  std::string ReadTemplateFile(const std::string& templateFile,
                               cppmicroservices::BundleContext context =
                                 cppmicroservices::GetBundleContext()) const;

private:
  std::string GetHeader() const;
  std::string GetFooter() const;

  virtual BundleResource GetResource(const std::string& path) const;

  /**
   * If the request addresses a resource which may be served by the
   * <code>GetResource</code> method of this instance, this method serves it
   * and returns <code>true</code>. Otherwise <code>false</code> is returned.
   *
   * If <code>true</code> is returned, the request is considered complete and
   * request processing terminates. Otherwise request processing continues
   * with normal plugin rendering.
   *
   * @param request The request object
   * @param response The response object
   * @return <code>true</code> if the request causes a resource to be sent back.
   *
   * @throws std::exception If an error occurs accessing or spooling the resource.
   */
  bool SpoolResource(HttpServletRequest& request,
                     HttpServletResponse& response) const;
};
}

#endif // CPPMICROSERVICES_ABSTRACTWEBCONSOLEPLUGIN_H
