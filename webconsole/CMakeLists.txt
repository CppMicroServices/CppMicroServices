set(_srcs
  src/AbstractWebConsolePlugin.cpp
  src/BundlesPlugin.cpp
  src/ServicesPlugin.cpp
  src/SettingsPlugin.cpp
  src/SimpleWebConsolePlugin.cpp
  src/VariableResolverStreamBuffer.cpp
  src/WebConsoleActivator.cpp
  src/WebConsoleConstants.cpp
  src/WebConsoleServlet.cpp
  src/WebConsoleDefaultVariableResolver.cpp
  src/WebConsoleVariableResolver.cpp
)

set(_private_headers
  src/BundlesPlugin.h
  src/ServicesPlugin.h
  src/SettingsPlugin.h
  src/VariableResolverStreamBuffer.h
  src/WebConsoleServlet.h
)

set(_public_headers
  include/cppmicroservices/webconsole/AbstractWebConsolePlugin.h
  include/cppmicroservices/webconsole/mustache.hpp
  include/cppmicroservices/webconsole/SimpleWebConsolePlugin.h
  include/cppmicroservices/webconsole/WebConsoleConstants.h
  include/cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h
  include/cppmicroservices/webconsole/WebConsoleVariableResolver.h
)

set(_resources
  manifest.json

  templates/main_footer.html
  templates/main_header.html
  templates/settings.html
  templates/bundles.html
  templates/bundle.html
  templates/services.html
  templates/service_interface.html

  res/css/bootstrap.min.css
  res/css/bootstrap-theme.min.css
  res/css/bootstrap-treeview.min.css

  res/css/console.css
  res/css/jquery.json-viewer.css

  res/fonts/glyphicons-halflings-regular.eot
  res/fonts/glyphicons-halflings-regular.svg
  res/fonts/glyphicons-halflings-regular.ttf
  res/fonts/glyphicons-halflings-regular.woff
  res/fonts/glyphicons-halflings-regular.woff2

  res/js/bootstrap.min.js
  res/js/bootstrap-treeview.min.js
  res/js/jquery.min.js
  res/js/jquery.json-viewer.js
)

usMacroCreateBundle(WebConsole
  VERSION "0.1.0"
  DEPENDS HttpService
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  SOURCES ${_srcs}
  RESOURCES ${_resources}
)
