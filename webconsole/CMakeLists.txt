set(_srcs
  src/AbstractWebConsolePlugin.cpp
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
  src/ServicesPlugin.h
  src/SettingsPlugin.h
  src/VariableResolverStreamBuffer.h
  src/WebConsoleServlet.h
)

set(_public_headers
  include/cppmicroservices/webconsole/AbstractWebConsolePlugin.h
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
  #templates/bundles.html
  #templates/bundle.html
  templates/services.html
  templates/service_interface.html

  res/css/main_style.css

  res/js/jquery.min.js
  res/js/pure.min.js
  res/js/tree.jquery.min.js
  res/js/jqtree.css

  res/KickStart/css/kickstart.css
  res/KickStart/css/kickstart-buttons.css
  res/KickStart/css/kickstart-forms.css
  res/KickStart/css/kickstart-grid.css
  res/KickStart/css/kickstart-menus.css
  res/KickStart/css/prettify.css
  res/KickStart/css/tiptip.css

  res/KickStart/css/fonts/fontawesome/css/font-awesome.min.css
  res/KickStart/css/fonts/fontawesome/css/font-awesome-ie7.min.css
  res/KickStart/css/fonts/fontawesome/font/FontAwesome.otf
  res/KickStart/css/fonts/fontawesome/font/fontawesome-webfont.eot
  res/KickStart/css/fonts/fontawesome/font/fontawesome-webfont.svg
  res/KickStart/css/fonts/fontawesome/font/fontawesome-webfont.ttf
  res/KickStart/css/fonts/fontawesome/font/fontawesome-webfont.woff

  res/KickStart/css/img/breadcrumbs-bg.gif
  res/KickStart/css/img/bx_loader.gif
  res/KickStart/css/img/controls.png
  res/KickStart/css/img/gray_jean.png
  res/KickStart/css/img/icon-arrow-right.png

  res/KickStart/js/kickstart.js
)

usMacroCreateBundle(WebConsole
  VERSION "0.99.0"
  DEPENDS HttpService
  INTERNAL_INCLUDE_DIRS src
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  SOURCES ${_srcs}
  RESOURCES ${_resources}
)
