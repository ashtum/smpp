find_package(Boost 1.81 COMPONENTS unit_test_framework REQUIRED)

add_executable(unit_test
    main.cpp pdu_test.cpp
    serialization_utils_test.cpp
    session_test.cpp)

target_link_libraries(unit_test smpp Boost::unit_test_framework)
target_compile_options(unit_test PRIVATE
    -Wall
    -Wfatal-errors
    -Wextra -pedantic
    -pedantic-errors
    -Wno-unused-parameter)
target_compile_features(unit_test PRIVATE cxx_std_20)
