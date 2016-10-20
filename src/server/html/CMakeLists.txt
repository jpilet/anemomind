add_library(html_HtmlDispatcher
  HtmlDispatcher.h
  HtmlDispatcher.cpp
  )

target_link_libraries(html_HtmlDispatcher
  common_DOMUtils
  anemobox_Dispatcher
  )

add_executable(html_makeHtmlOverview
  makeHtmlOverview.cpp
  )

target_link_libraries(html_makeHtmlOverview
  logimport_LogLoader
  html_HtmlDispatcher
  )