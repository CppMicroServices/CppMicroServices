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

#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"
#include "usHttpServletResponse.h"
#include "usHttpServletRequest.h"
#include "usServletContext.h"
#include "usWebConsoleDefaultVariableResolver.h"

US_BEGIN_NAMESPACE

struct AbstractWebConsolePluginPrivate
{
  AbstractWebConsolePluginPrivate()
    : m_VariableResolver(NULL)
  {}

  ~AbstractWebConsolePluginPrivate()
  {
    delete m_VariableResolver;
  }

  WebConsoleVariableResolver* m_VariableResolver;
};

AbstractWebConsolePlugin::AbstractWebConsolePlugin()
  : d(new AbstractWebConsolePluginPrivate)
{

}

AbstractWebConsolePlugin::~AbstractWebConsolePlugin()
{
  delete d;
}

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

WebConsoleVariableResolver* AbstractWebConsolePlugin::GetVariableResolver(const HttpServletRequest& request) const
{
  if (d->m_VariableResolver == NULL)
  {
    WebConsoleDefaultVariableResolver* resolver = new WebConsoleDefaultVariableResolver();
    (*resolver)["appRoot"] = request.GetAttribute(WebConsoleConstants::ATTR_APP_ROOT()).ToString();
    (*resolver)["pluginRoot"] = request.GetAttribute(WebConsoleConstants::ATTR_PLUGIN_ROOT()).ToString();
    d->m_VariableResolver = resolver;
  }
  return d->m_VariableResolver;
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

  WebConsoleVariableResolver* resolver = this->GetVariableResolver(request);
  if (WebConsoleDefaultVariableResolver* r = dynamic_cast<WebConsoleDefaultVariableResolver*>(resolver))
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

std::string AbstractWebConsolePlugin::ReadTemplateFile(const std::string& templateFile, us::ModuleContext* context) const
{
  std::string result;

  if (context == NULL)
  {
    context = us::GetModuleContext();
  }

  us::ModuleResource res = context->GetModule()->GetResource(templateFile);
  if (!res)
  {
    std::cout << "Resource file '" << templateFile << "' not found in module '"
              << context->GetModule()->GetName() << "'" << std::endl;
    return result;
  }

  us::ModuleResourceStream resStream(res, std::ios::binary);
  resStream.seekg(0, std::ios::end);
  result.resize(resStream.tellg());
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

ModuleResource AbstractWebConsolePlugin::GetResource(const std::string& /*path*/) const
{
  return ModuleResource();
}

bool AbstractWebConsolePlugin::SpoolResource(HttpServletRequest& request, HttpServletResponse& response) const
{
  std::string pi = request.GetPathInfo();
  us::ModuleResource res = this->GetResource(pi);
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

  us::ModuleResourceStream resStream(res, std::ios::binary);

  // describe the contents
  response.SetContentType(GetServletContext()->GetMimeType(pi));
  int size = res.GetSize();
  if (res.IsCompressed())
  {
    resStream.seekg(0, std::ios::end);
    size = resStream.tellg();
    resStream.seekg(0, std::ios::beg);
  }
  response.SetIntHeader("Content-Length", size);

  // spool the actual contents
  response.GetOutputStream() << resStream.rdbuf();

  return true;
}

US_END_NAMESPACE
