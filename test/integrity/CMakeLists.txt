find_package(fmt)
find_package(Boost 1.81 REQUIRED)

add_executable(integrity main.cpp)
target_link_libraries(integrity smpp pthread fmt::fmt Boost::headers)

target_compile_features(integrity PUBLIC cxx_std_20)
target_compile_options(integrity PUBLIC -Wall -Wfatal-errors -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -Wno-unused-parameter)
