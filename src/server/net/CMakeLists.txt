add_executable(net_WebServerDemo
               WebServerDemo.cpp
              )

target_depends_on_poco_util(net_WebServerDemo)
target_depends_on_poco_net(net_WebServerDemo)
target_depends_on_poco_foundation(net_WebServerDemo)