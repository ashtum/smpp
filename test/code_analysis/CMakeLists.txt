find_package(Boost 1.81 REQUIRED)

add_executable(code_analysis main.cpp)
target_link_libraries(code_analysis smpp pthread Boost::headers)

target_compile_features(code_analysis PUBLIC cxx_std_20)
target_compile_options(code_analysis PUBLIC -Wall -Wfatal-errors -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -Wno-unused-parameter)
