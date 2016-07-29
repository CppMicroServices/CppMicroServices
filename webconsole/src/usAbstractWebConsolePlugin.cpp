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

#include "usAbstractWebConsolePlugin.h"

#include "usWebConsoleConstants.h"

#include "usBundle.h"
#include "usBundleContext.h"
#include "usBundleResource.h"
#include "usBundleResourceStream.h"
#include "usHttpServletResponse.h"
#include "usHttpServletRequest.h"
#include "usServletContext.h"
#include "usWebConsoleDefaultVariableResolver.h"

#include <iostream>


namespace us {

std::string AbstractWebConsolePlugin::GetCategory() const
{
  return std::string();
}

bool AbstractWebConsolePlugin::IsHtmlRequest(HttpServletRequest&)
{
  return true;
}

void AbstractWebConsolePlugin::DoGet(HttpServletRequest& request, HttpServletResponse& response)
{
  if (!SpoolResource(request, response))
  {
    // detect if this is an html request
    if (IsHtmlRequest(request))
    {
      // start the html response, write the header, open body and main div
      std::ostream& os = StartResponse(request, response);

      // render top navigation
      //RenderTopNavigation(request, os);

      // wrap content in a separate div
      //pw.println( "<div id='content'>" );
      RenderContent(request, response);
      //pw.println( "</div>" );

      // close the main div, body, and html
      EndResponse(os);
    }
    else
    {
      RenderContent(request, response);
    }
  }
}

std::shared_ptr<WebConsoleVariableResolver> AbstractWebConsolePlugin::GetVariableResolver(HttpServletRequest& request)
{
  Any resolverAny = request.GetAttribute(WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER());
  if (!resolverAny.Empty() && resolverAny.Type() == typeid(std::shared_ptr<WebConsoleVariableResolver>))
  {
    return any_cast<std::shared_ptr<WebConsoleVariableResolver>>(resolverAny);
  }

  auto resolver = std::make_shared<WebConsoleDefaultVariableResolver>();
  (*resolver)["appRoot"] = request.GetAttribute(WebConsoleConstants::ATTR_APP_ROOT()).ToString();
  (*resolver)["pluginRoot"] = request.GetAttribute(WebConsoleConstants::ATTR_PLUGIN_ROOT()).ToString();
  SetVariableResolver(request, resolver);

  return resolver;
}

void AbstractWebConsolePlugin::SetVariableResolver(HttpServletRequest& request, std::shared_ptr<WebConsoleVariableResolver> resolver)
{
  request.SetAttribute(WebConsoleConstants::ATTR_CONSOLE_VARIABLE_RESOLVER(), resolver);
}

std::ostream& AbstractWebConsolePlugin::StartResponse(HttpServletRequest& request, HttpServletResponse& response)
{
  response.SetCharacterEncoding("utf-8");
  response.SetContentType("text/html");

  std::ostream& os = response.GetOutputStream();

  // support localization of the plugin title
  std::string title = GetTitle();
  if (title[0] == '%')
  {
    title = "{$" + title.substr(1) + "}";
  }

  auto resolver = this->GetVariableResolver(request);
  if (std::shared_ptr<WebConsoleDefaultVariableResolver> r = std::dynamic_pointer_cast<WebConsoleDefaultVariableResolver>(resolver))
  {
    (*r)["labelMap"] = request.GetAttribute(WebConsoleConstants::ATTR_LABEL_MAP()).ToString();

    //    r.put("head.title", title); //$NON-NLS-1$
    (*r)["head.label"] = GetLabel();
    //    r.put("head.cssLinks", getCssLinks(appRoot)); //$NON-NLS-1$
    //    r.put("brand.name", brandingPlugin.getBrandName()); //$NON-NLS-1$
    //    r.put("brand.product.url", brandingPlugin.getProductURL()); //$NON-NLS-1$
    //    r.put("brand.product.name", brandingPlugin.getProductName()); //$NON-NLS-1$
    //    r.put("brand.product.img", toUrl( brandingPlugin.getProductImage(), appRoot )); //$NON-NLS-1$
    //    r.put("brand.favicon", toUrl( brandingPlugin.getFavIcon(), appRoot )); //$NON-NLS-1$
    //    r.put("brand.css", toUrl( brandingPlugin.getMainStyleSheet(), appRoot )); //$NON-NLS-1$
  }
  os << GetHeader();

  return os;
}

void AbstractWebConsolePlugin::RenderTopNavigation(HttpServletRequest& /*request*/, std::ostream& /*writer*/)
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

void AbstractWebConsolePlugin::EndResponse(std::ostream& os)
{
  os << GetFooter();
}

std::vector<std::string> AbstractWebConsolePlugin::GetCssReferences() const
{
  return std::vector<std::string>();
}

std::string AbstractWebConsolePlugin::ReadTemplateFile(
    const std::string& templateFile,
    us::BundleContext context
    ) const
{
  std::string result;

  if (!context)
  {
    context = us::GetBundleContext();
  }

  us::BundleResource res = context.GetBundle().GetResource(templateFile);
  if (!res)
  {
    std::cout << "Resource file '" << templateFile << "' not found in bundle '"
              << context.GetBundle().GetSymbolicName() << "'" << std::endl;
    return result;
  }

  us::BundleResourceStream resStream(res, std::ios::binary);
  resStream.seekg(0, std::ios::end);
  result.resize(static_cast<std::size_t>(resStream.tellg()));
  resStream.seekg(0, std::ios::beg);
  resStream.read(&result[0], result.size());
  return result;
}

std::string AbstractWebConsolePlugin::GetHeader() const
{
  static std::string HEADER;
  if (HEADER.empty())
  {
    HEADER = this->ReadTemplateFile("/templates/main_header.html");
  }
  return HEADER;
}

std::string AbstractWebConsolePlugin::GetFooter() const
{
  static std::string FOOTER;
  if (FOOTER.empty())
  {
    FOOTER = this->ReadTemplateFile("/templates/main_footer.html");
  }
  return FOOTER;
}

BundleResource AbstractWebConsolePlugin::GetResource(const std::string& /*path*/) const
{
  return BundleResource();
}

bool AbstractWebConsolePlugin::SpoolResource(HttpServletRequest& request, HttpServletResponse& response) const
{
  std::string pi = request.GetPathInfo();
  us::BundleResource res = this->GetResource(pi);
  if (!res)
  {
    return false;
  }

  // check whether we may return 304/UNMODIFIED
  long long lastModified = res.GetLastModified();
  if (lastModified > 0)
  {
    long long ifModifiedSince = request.GetDateHeader( "If-Modified-Since" );
    if (ifModifiedSince >= lastModified)
    {
      response.SetStatus(HttpServletResponse::SC_NOT_MODIFIED);
      return true;
    }
    // have to send, so set the last modified header now
    response.SetDateHeader("Last-Modified", lastModified);
  }

  us::BundleResourceStream resStream(res, std::ios::binary);

  // describe the contents
  response.SetContentType(GetServletContext()->GetMimeType(pi));
  int size = res.GetSize();
  response.SetIntHeader("Content-Length", size);

  // spool the actual contents
  response.GetOutputStream() << resStream.rdbuf();

  return true;
}

}
