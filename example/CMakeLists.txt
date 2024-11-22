add_executable(server server.cpp)
target_link_libraries(server smpp)
target_compile_features(server PRIVATE cxx_std_20)
target_compile_options(server PRIVATE
    -Wall
    -Wfatal-errors
    -Wextra
    -pedantic
    -pedantic-errors
    -Wno-unused-parameter)

add_executable(client client.cpp)
target_link_libraries(client smpp)
target_compile_features(client PRIVATE cxx_std_20)
target_compile_options(client PRIVATE
    -Wall
    -Wfatal-errors
    -Wextra
    -pedantic
    -pedantic-errors
    -Wno-unused-parameter)
