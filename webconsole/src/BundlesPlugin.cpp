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

#include "BundlesPlugin.h"

#include "cppmicroservices/httpservice/ServletContext.h"

#include "cppmicroservices/webconsole/WebConsoleConstants.h"
#include "cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h"

#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Constants.h"

#include <cmath>

namespace cppmicroservices {

static const std::string REQ_BUNDLE_ID = "bundles_req_id";
static const std::string REQ_BUNDLE_TYPE = "bundles_req_type";
static const std::string REQ_BUNDLE_RES_PATH = "bundles_req_respath";

std::string NumToString(int64_t val);

bool SetIfExists(AbstractWebConsolePlugin::TemplateData& entry,
                 std::string const& field,
                 std::string const& key,
                 AnyMap const& headers)
{
  try {
    auto val = headers.at(key);
    entry[field] = val.ToString();
    return true;
  } catch (std::exception const&) {
  }

  return false;
}

std::string SizeTag(const std::pair<std::size_t, std::size_t>& p)
{
  std::stringstream ss;
  double factor = 1.;
  std::string postfix = "B";

  if (p.first > 1000) {
    if (p.first > 1000 * 1000) {
      if (p.first > 1000 * 1000 * 1000) {
        ss << std::lround(
          (static_cast<double>(p.first) / 1000.0 / 1000.0 / 1000.0));
        factor = 1000 * 1000 * 1000;
        postfix = "GB";
      } else {
        ss << std::lround((static_cast<double>(p.first) / 1000.0 / 1000.0));
        factor = 1000 * 1000;
        postfix = "MB";
      }
    } else {
      ss << std::lround((static_cast<double>(p.first) / 1000.0));
      factor = 1000;
      postfix = "KB";
    }
  } else {
    ss << p.first;
  }

  ss << " (" << std::lround((static_cast<double>(p.second) / factor)) << ") "
     << postfix;
  return ss.str();
}

BundlesPlugin::BundlesPlugin()
  : SimpleWebConsolePlugin("bundles", "Bundles", "")
{}

void BundlesPlugin::RenderContent(HttpServletRequest& request,
                                  HttpServletResponse& response)
{
  switch (static_cast<RequestType>(
    any_cast<int>(request.GetAttribute(REQ_BUNDLE_TYPE)))) {
    case RequestType::MainPage: {
      BundleResource res =
        GetBundleContext().GetBundle().GetResource("/templates/bundles.html");
      if (!res)
        break;

      auto& data = std::static_pointer_cast<WebConsoleDefaultVariableResolver>(
                     GetVariableResolver(request))
                     ->GetData();
      data["bundles"] = GetBundlesData();

      BundleResourceStream rs(res, std::ios_base::binary);
      response.GetOutputStream() << rs.rdbuf();
      return;
    }
    case RequestType::Bundle: {
      BundleResource res =
        GetBundleContext().GetBundle().GetResource("/templates/bundle.html");
      if (!res)
        break;

      std::string pluginRoot =
        request.GetAttribute(WebConsoleConstants::ATTR_PLUGIN_ROOT).ToString();
      auto id = any_cast<long>(request.GetAttribute(REQ_BUNDLE_ID));
      auto& data = std::static_pointer_cast<WebConsoleDefaultVariableResolver>(
                     GetVariableResolver(request))
                     ->GetData();
      GetBundleData(id, data, pluginRoot);

      BundleResourceStream rs(res, std::ios_base::binary);
      response.GetOutputStream() << rs.rdbuf();
      return;
    }
    case RequestType::Resource: {
      auto id = any_cast<long>(request.GetAttribute(REQ_BUNDLE_ID));
      Bundle bundle = GetContext().GetBundle(id);
      if (!bundle)
        break;

      std::string resPath =
        any_cast<std::string>(request.GetAttribute(REQ_BUNDLE_RES_PATH));
      BundleResource res = bundle.GetResource(resPath);
      if (!res)
        break;

      response.SetHeader("Cache", "no-cache");
      std::string mime_type = GetServletContext()->GetMimeType(resPath);
      if (!mime_type.empty()) {
        response.SetHeader("Content-Type", mime_type);
      }

      BundleResourceStream rs(res, std::ios::binary);
      response.GetOutputStream() << rs.rdbuf();
      return;
    }
    case RequestType::Unknown:
    default:
      break;
  }

  response.SetStatus(response.SC_NOT_FOUND);
}

bool BundlesPlugin::IsHtmlRequest(HttpServletRequest& request)
{
  RequestType requestType = RequestType::Unknown;
  long bundleId = -1;
  std::string resPath;

  std::string pathInfo = request.GetPathInfo();
  if (pathInfo == "/bundles") {
    requestType = RequestType::MainPage;
  } else if (pathInfo.size() > 9 && pathInfo.compare(0, 9, "/bundles/") == 0) {
    try {
      bundleId = std::stol(pathInfo.substr(9));

      std::string::size_type pos = pathInfo.find_first_of('/', 9);
      if (pos == std::string::npos) {
        requestType = RequestType::Bundle;
      } else {
        std::string sub = pathInfo.substr(pos);
        const std::string res_prefix = "/resources/";
        if (sub.size() > res_prefix.size() &&
            sub.compare(0, res_prefix.size(), res_prefix) == 0) {
          requestType = RequestType::Resource;
          resPath = sub.substr(res_prefix.size() - 1);
        }
      }
    } catch (const std::exception&) {
      requestType = RequestType::Unknown;
    }
  }

  request.SetAttribute(REQ_BUNDLE_TYPE, static_cast<int>(requestType));
  request.SetAttribute(REQ_BUNDLE_ID, bundleId);
  request.SetAttribute(REQ_BUNDLE_RES_PATH, resPath);

  return requestType != RequestType::Resource;
}

AbstractWebConsolePlugin::TemplateData BundlesPlugin::GetBundlesData() const
{
  TemplateData data(TemplateData::Type::List);

  auto bundles = GetContext().GetBundles();
  std::sort(
    bundles.begin(), bundles.end(), [](Bundle const& b1, Bundle const& b2) {
      return b1.GetBundleId() < b2.GetBundleId();
    });
  for (auto& bundle : bundles) {
    TemplateData entry;

    AnyMap headers = bundle.GetHeaders();

    entry["id"] = NumToString(bundle.GetBundleId());
    entry["bsn"] = bundle.GetSymbolicName();
    SetIfExists(entry, "name", Constants::BUNDLE_NAME, headers);
    SetIfExists(entry, "description", Constants::BUNDLE_DESCRIPTION, headers);
    entry["version"] = bundle.GetVersion().ToString();
    SetIfExists(entry, "vendor", Constants::BUNDLE_VENDOR, headers);
    entry["state"] = static_cast<std::stringstream const&>(std::stringstream()
                                                           << bundle.GetState())
                       .str();

    data << std::move(entry);
  }

  return data;
}

std::pair<std::size_t, std::size_t> BundlesPlugin::GetResourceJsonTree(
  Bundle& bundle,
  const std::string& parentPath,
  const BundleResource& currResource,
  std::string& json,
  int level,
  const std::string& pluginRoot) const
{
  std::pair<std::size_t, std::size_t> totalSize(0, 0);
  std::string indent;
  for (int i = 0; i < level; i++) {
    indent += "  ";
  }

  if (currResource.IsFile()) {
    char lm_buf[50] = { 0 };
    time_t lm_t = currResource.GetLastModified();
#ifdef US_PLATFORM_WINDOWS
    ctime_s(lm_buf, 50, &lm_t);
#else
    ctime_r(&lm_t, lm_buf);
#endif
    std::string lm_str(lm_buf);
    std::string::size_type pos = lm_str.find_last_not_of(" \r\n");
    lm_str = lm_str.substr(0, pos != std::string::npos ? pos + 1 : pos);
    json += indent + "{ text : '" + currResource.GetName() + "',\n" + indent +
            "  icon : 'glyphicon glyphicon-open',\n" + indent + "  href : '" +
            pluginRoot + "/" + NumToString(bundle.GetBundleId()) +
            "/resources" + currResource.GetResourcePath() + "',\n" + indent +
            "  tags : ['Size: " +
            SizeTag(std::make_pair(currResource.GetSize(),
                                   currResource.GetCompressedSize())) +
            "',\n" + indent + "          'Last modified: " + lm_str + "'] }\n";
    totalSize.first += currResource.GetSize();
    totalSize.second += currResource.GetCompressedSize();
  } else {
    json += indent + "{ text : '" +
            currResource.GetResourcePath().substr(parentPath.size()) + "',\n";
    json += indent + "  selectable : false\n";
    std::vector<std::string> children = currResource.GetChildren();
    if (children.empty()) {
      json += "}";
    } else {
      json += indent + ", nodes : [\n";
      std::vector<BundleResource> childDirs;
      std::vector<BundleResource> childFiles;
      for (auto& child : children) {
        BundleResource childResource =
          bundle.GetResource(currResource.GetResourcePath() + "/" + child);
        if (childResource.IsValid()) {
          if (childResource.IsDir()) {
            childDirs.push_back(childResource);
          } else {
            childFiles.push_back(childResource);
          }
        }
      }

      for (auto& childDir : childDirs) {
        auto size = GetResourceJsonTree(bundle,
                                        currResource.GetResourcePath(),
                                        childDir,
                                        json,
                                        level + 1,
                                        pluginRoot);
        totalSize.first += size.first;
        totalSize.second += size.second;
        json += indent + ",\n";
      }
      for (auto & childFile : childFiles) {
        auto size = GetResourceJsonTree(bundle,
                                        currResource.GetResourcePath(),
                                        childFile,
                                        json,
                                        level + 1,
                                        pluginRoot);
        totalSize.first += size.first;
        totalSize.second += size.second;
        json += indent + ",\n";
      }
      json += indent + "],\n";
      json += indent + "tags : ['Size: " + SizeTag(totalSize) + "']\n";
      json += indent + "}\n";
    }
  }

  return totalSize;
}

void BundlesPlugin::GetBundleData(long id,
                                  TemplateData& data,
                                  const std::string& pluginRoot) const
{
  auto bundle = GetBundleContext().GetBundle(id);
  if (!bundle)
    return;

  auto headers = bundle.GetHeaders();
  SetIfExists(data, "bundle-name", Constants::BUNDLE_NAME, headers);
  data["bundle-bsn"] = bundle.GetSymbolicName();

  // ----------------- Get bundle manifest data ---------------------

  data["bundle-manifest"] = Any(bundle.GetHeaders()).ToJSON();

  // --------------- Get bundle resource information ------------------

  std::string res_json = "[";
  BundleResource resource = bundle.GetResource("/");
  if (resource.IsValid()) {
    GetResourceJsonTree(bundle, "", resource, res_json, 0, pluginRoot);
  }
  res_json += "]";
  data["bundle-resources"] = res_json;

  // ------------- Get registered service information ----------------

  auto services = TemplateData::List();
  for (auto& s : bundle.GetRegisteredServices()) {
    TemplateData service;
    service["id"] = s.GetProperty(Constants::SERVICE_ID).ToStringNoExcept();
    service["types"] = s.GetProperty(Constants::OBJECTCLASS).ToStringNoExcept();
    service["ranking"] =
      s.GetProperty(Constants::SERVICE_RANKING).ToStringNoExcept();
    service["scope"] =
      s.GetProperty(Constants::SERVICE_SCOPE).ToStringNoExcept();

    AnyMap props(AnyMap::ORDERED_MAP);
    for (auto const& p : s.GetPropertyKeys()) {
      props.insert(std::make_pair(p, s.GetProperty(p)));
    }
    service["props"] = Any(props).ToJSON();

    services << std::move(service);
  }
  data["services"] = std::move(services);
}
}
