add_library(html_HtmlDispatcher
  HtmlDispatcher.h
  HtmlDispatcher.cpp
  )

target_link_libraries(html_HtmlDispatcher
  common_DOMUtils
  anemobox_Dispatcher
  )