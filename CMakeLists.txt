cmake_minimum_required(VERSION 3.2.1)

project(
  smpp
  VERSION 0.1.0
  LANGUAGES CXX)

add_library(smpp INTERFACE)

target_include_directories(smpp INTERFACE include/)

include(GNUInstallDirs)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")

add_subdirectory(example)
add_subdirectory(test)
