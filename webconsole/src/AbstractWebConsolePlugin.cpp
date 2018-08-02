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

#include "cppmicroservices/webconsole/AbstractWebConsolePlugin.h"

#include "cppmicroservices/webconsole/WebConsoleConstants.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"
#include "cppmicroservices/httpservice/ServletContext.h"
#include "cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h"

#include <iostream>

namespace Kainjow {

std::ostream& operator<<(std::ostream& os, const Mustache::Data&)
{
  os << "Opaque mustache data";
  return os;
}
}

namespace cppmicroservices {

std::string NumToString(int64_t val)
{
#if defined(__ANDROID__)
  std::ostringstream os;
  os << val;
  return os.str();
#else
  return std::to_string(val);
#endif
}

std::string AbstractWebConsolePlugin::GetCategory() const
{
  return std::string();
}

bool AbstractWebConsolePlugin::IsHtmlRequest(HttpServletRequest&)
{
  return true;
}

void AbstractWebConsolePlugin::DoGet(HttpServletRequest& request,
                                     HttpServletResponse& response)
{
  if (!SpoolResource(request, response)) {
    // detect if this is an html request
    if (IsHtmlRequest(request)) {
      // start the html response, write the header, open body and main div
      std::ostream& os = StartResponse(request, response);

      // render top navigation
      //RenderTopNavigation(request, os);

      // wrap content in a separate div
      //pw.println( "<div id='content'>" );
      RenderContent(request, response);
      //pw.println( "</div>" );

      // close the main div, body, and html
      EndResponse(request, os);
    } else {
      RenderContent(request, response);
    }
  }
}

std::shared_ptr<WebConsoleVariableResolver>
AbstractWebConsolePlugin::GetVariableResolver(HttpServletRequest& request)
{
  Any resolverAny =
    request.GetAttribute(WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER);
  if (!resolverAny.Empty() &&
      resolverAny.Type() ==
        typeid(std::shared_ptr<WebConsoleVariableResolver>)) {
    return any_cast<std::shared_ptr<WebConsoleVariableResolver>>(resolverAny);
  }

  auto resolver = std::make_shared<WebConsoleDefaultVariableResolver>();
  auto& data = resolver->GetData();
  data["appRoot"] =
    request.GetAttribute(WebConsoleConstants::ATTR_APP_ROOT).ToString();
  data["pluginRoot"] =
    request.GetAttribute(WebConsoleConstants::ATTR_PLUGIN_ROOT).ToString();
  data["pluginLabel"] = GetLabel();
  data["pluginTitle"] = GetTitle();
  SetVariableResolver(request, resolver);

  return resolver;
}

void AbstractWebConsolePlugin::SetVariableResolver(
  HttpServletRequest& request,
  const std::shared_ptr<WebConsoleVariableResolver>& resolver)
{
  request.SetAttribute(WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER,
                       resolver);
}

std::ostream& AbstractWebConsolePlugin::StartResponse(
  HttpServletRequest& request,
  HttpServletResponse& response)
{
  response.SetCharacterEncoding("utf-8");
  response.SetContentType("text/html");

  std::ostream& os = response.GetOutputStream();

  // support localization of the plugin title
  std::string title = GetTitle();
  if (title[0] == '%') {
    title = "{$" + title.substr(1) + "}";
  }

  auto resolver = this->GetVariableResolver(request);
  if (std::shared_ptr<WebConsoleDefaultVariableResolver> r =
        std::dynamic_pointer_cast<WebConsoleDefaultVariableResolver>(
          resolver)) {
    auto& data = r->GetData();
    data["labelMap"] = any_cast<TemplateData>(
      request.GetAttribute(WebConsoleConstants::ATTR_LABEL_MAP));

    TemplateData head;
    head["title"] = title;
    head["label"] = GetLabel();
    //    r.put("head.cssLinks", getCssLinks(appRoot));
    data["head"] = std::move(head);

    TemplateData brand;
    brand["name"] = "C++ Micro Services";
    //    r.put("brand.name", brandingPlugin.getBrandName());
    //    r.put("brand.product.url", brandingPlugin.getProductURL());
    //    r.put("brand.product.name", brandingPlugin.getProductName());
    //    r.put("brand.product.img", toUrl( brandingPlugin.getProductImage(), appRoot ));
    //    r.put("brand.favicon", toUrl( brandingPlugin.getFavIcon(), appRoot ));
    //    r.put("brand.css", toUrl( brandingPlugin.getMainStyleSheet(), appRoot ));
    data["brand"] = std::move(brand);
  }
  os << GetHeader();

  return os;
}

void AbstractWebConsolePlugin::RenderTopNavigation(
  HttpServletRequest& /*request*/,
  std::ostream& /*writer*/)
{
  //  // assume pathInfo to not be null, else this would not be called
  //  std::string current = request.GetPathInfo();
  //  std::size_t slash = current.find_first_of('/', 1);
  //  current = current.substr(1, slash != std::string::npos ? slash-1 : slash);

  //  std::string appRoot = request.GetAttribute(WebConsoleConstants::ATTR_APP_ROOT).ToString();

  //  Map menuMap = ( Map ) request.getAttribute( OsgiManager.ATTR_LABEL_MAP_CATEGORIZED );
  //  this.renderMenu( menuMap, appRoot, pw );

  //  // render lang-box
  //  Map langMap = (Map) request.getAttribute(WebConsoleConstants.ATTR_LANG_MAP);
  //  if (null != langMap && !langMap.isEmpty())
  //  {
  //    // determine the currently selected locale from the request and fail-back
  //    // to the default locale if not set
  //    // if locale is missing in locale map, the default 'en' locale is used
  //    Locale reqLocale = request.getLocale();
  //    String locale = null != reqLocale ? reqLocale.getLanguage()
  //                                      : Locale.getDefault().getLanguage();
  //    if (!langMap.containsKey(locale))
  //    {
  //      locale = Locale.getDefault().getLanguage();
  //    }
  //    if (!langMap.containsKey(locale))
  //    {
  //      locale = "en"; //$NON-NLS-1$
  //    }

  //    pw.println("<div id='langSelect'>"); //$NON-NLS-1$
  //    pw.println(" <span>"); //$NON-NLS-1$
  //    printLocaleElement(pw, appRoot, locale, langMap.get(locale));
  //    pw.println(" </span>"); //$NON-NLS-1$
  //    pw.println(" <span class='flags ui-helper-hidden'>"); //$NON-NLS-1$
  //    for (Iterator li = langMap.keySet().iterator(); li.hasNext();)
  //    {
  //      // <img src="us.gif" alt="en" title="English"/>
  //      final Object l = li.next();
  //      if (!l.equals(locale))
  //      {
  //        printLocaleElement(pw, appRoot, l, langMap.get(l));
  //      }
  //    }

  //    pw.println(" </span>"); //$NON-NLS-1$
  //    pw.println("</div>"); //$NON-NLS-1$
  //  }
}

void AbstractWebConsolePlugin::EndResponse(HttpServletRequest& request,
                                           std::ostream& os)
{
  auto resolver = this->GetVariableResolver(request);
  if (std::shared_ptr<WebConsoleDefaultVariableResolver> r =
        std::dynamic_pointer_cast<WebConsoleDefaultVariableResolver>(
          resolver)) {
    auto& data = r->GetData();
    data["us-version"] = US_VERSION_STR;

    auto ctx = GetBundleContext();
    auto bundles = ctx.GetBundles();
    int active_count = 0;
    for (auto& b : bundles) {
      if (b.GetState() & (Bundle::STATE_ACTIVE | Bundle::STATE_STARTING)) {
        ++active_count;
      }
    }
    std::stringstream ss;
    ss << bundles.size();
    data["us-num-bundles"] = ss.str();
    ss.str(std::string());
    ss << active_count;
    data["us-num-active-bundles"] = ss.str();
  }

  os << GetFooter();
}

std::vector<std::string> AbstractWebConsolePlugin::GetCssReferences() const
{
  return std::vector<std::string>();
}

std::string AbstractWebConsolePlugin::ReadTemplateFile(
  const std::string& templateFile,
  cppmicroservices::BundleContext context) const
{
  std::string result;

  if (!context) {
    context = cppmicroservices::GetBundleContext();
  }

  cppmicroservices::BundleResource res =
    context.GetBundle().GetResource(templateFile);
  if (!res) {
    std::cout << "Resource file '" << templateFile << "' not found in bundle '"
              << context.GetBundle().GetSymbolicName() << "'" << std::endl;
    return result;
  }

  cppmicroservices::BundleResourceStream resStream(res, std::ios::binary);
  resStream.seekg(0, std::ios::end);
  result.resize(static_cast<std::size_t>(resStream.tellg()));
  resStream.seekg(0, std::ios::beg);
  resStream.read(&result[0], result.size());
  return result;
}

std::string AbstractWebConsolePlugin::GetHeader() const
{
  static std::string HEADER;
  if (HEADER.empty()) {
    HEADER = this->ReadTemplateFile("/templates/main_header.html");
  }
  return HEADER;
}

std::string AbstractWebConsolePlugin::GetFooter() const
{
  static std::string FOOTER;
  if (FOOTER.empty()) {
    FOOTER = this->ReadTemplateFile("/templates/main_footer.html");
  }
  return FOOTER;
}

BundleResource AbstractWebConsolePlugin::GetResource(
  const std::string& /*path*/) const
{
  return BundleResource();
}

bool AbstractWebConsolePlugin::SpoolResource(
  HttpServletRequest& request,
  HttpServletResponse& response) const
{
  std::string pi = request.GetPathInfo();
  cppmicroservices::BundleResource res = this->GetResource(pi);
  if (!res) {
    return false;
  }

  // check whether we may return 304/UNMODIFIED
  long long lastModified = res.GetLastModified();
  if (lastModified > 0) {
    long long ifModifiedSince = request.GetDateHeader("If-Modified-Since");
    if (ifModifiedSince >= lastModified) {
      response.SetStatus(HttpServletResponse::SC_NOT_MODIFIED);
      return true;
    }
    // have to send, so set the last modified header now
    response.SetDateHeader("Last-Modified", lastModified);
  }

  cppmicroservices::BundleResourceStream resStream(res, std::ios::binary);

  // describe the contents
  response.SetContentType(GetServletContext()->GetMimeType(pi));
  int size = res.GetSize();
  response.SetIntHeader("Content-Length", size);

  // spool the actual contents
  response.GetOutputStream() << resStream.rdbuf();

  return true;
}
}
