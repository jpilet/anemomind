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
  html_TimedValueDiagram
  )

add_library(html_TimedValueDiagram
  TimedValueDiagram.h
  TimedValueDiagram.cpp
  )

target_link_libraries(html_TimedValueDiagram
  plot_PlotUtils
  plot_CairoUtils
  )