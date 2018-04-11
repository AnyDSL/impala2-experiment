cmake_minimum_required(VERSION 3.10)
project(impala VERSION 2.0 DESCRIPTION "the higher-order intermediate representation")

option(BUILD_SHARED_LIBS "Build shared libraries (so/dll)" ON)
option(BUILD_TESTS "Build gtest-based test program" ON)

add_subdirectory(thorin2)

# target: library impala

add_library(impala
    impala/ast.h
    impala/compiler.h
    impala/lexer.cpp
    impala/parser.cpp
    impala/parser.h
    impala/print.cpp
    impala/print.h
    impala/token.cpp
    impala/token.h
    impala/sema/world.cpp
    impala/sema/world.h
)
set_target_properties(impala PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
set_target_properties(impala PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(impala PROPERTIES SOVERSION 2)
target_include_directories(impala PRIVATE . thorin2/ thorin2/half/include/)

# target: executable impala-bin

add_executable(impala-bin
    driver/main.cpp
)
set_target_properties(impala-bin PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
      OUTPUT_NAME impala
)
target_include_directories(impala-bin PRIVATE . thorin2/ thorin2/half/include/)
target_link_libraries(impala-bin impala thorin)

# target: executable impala-gtest

if(BUILD_TESTS)
    include(CTest)
    include(GoogleTest)

    add_executable(impala-gtest
        test/lexer.cpp
        test/main.cpp
    )
    set_target_properties(impala-gtest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    target_include_directories(impala-gtest PRIVATE . thorin2/ thorin2/half/include/)
    target_link_libraries(impala-gtest impala thorin gtest_main)
    gtest_discover_tests(impala-gtest TEST_PREFIX "impala.")
endif()
