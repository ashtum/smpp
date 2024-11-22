cmake_minimum_required(VERSION 3.8)

project(
  smpp
  VERSION 0.1.0
  LANGUAGES CXX)

find_package(Boost 1.81 REQUIRED)

add_library(smpp INTERFACE)

target_include_directories(smpp INTERFACE include/)
target_link_libraries(smpp INTERFACE Boost::headers)

include(GNUInstallDirs)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")

if (SMPP_BUILD_TESTS)
  add_subdirectory(test)
endif ()

if (SMPP_BUILD_EXAMPLES)
  add_subdirectory(example)
endif ()
