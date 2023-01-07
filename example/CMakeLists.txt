find_package(fmt REQUIRED)
find_package(Boost 1.81 REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server smpp pthread fmt::fmt Boost::headers)

add_executable(client client.cpp)
target_link_libraries(client smpp pthread fmt::fmt Boost::headers)

target_compile_features(client PUBLIC cxx_std_20)
target_compile_options(client PUBLIC -Wall -Wfatal-errors -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -Wno-unused-parameter)

target_compile_features(server PUBLIC cxx_std_20)
target_compile_options(server PUBLIC -Wall -Wfatal-errors -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -Wno-unused-parameter)
