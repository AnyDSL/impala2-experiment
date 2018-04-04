cmake_minimum_required(VERSION 3.10)
project(impala VERSION 2.0 DESCRIPTION "the higher-order intermediate representation")

option(BUILD_SHARED_LIBS "Build libimpala as shared library (so/dll)" ON)
option(BUILD_TESTS "Build gtest-based test program" ON)
set(THORIN2_DIR "../thorin2" CACHE FILEPATH "thorin2 directory")

# libimpala2

add_library(libimpala
    impala/lexer.h
    impala/lexer.cpp
)

set_target_properties(libimpala PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
set_target_properties(libimpala PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(libimpala PROPERTIES SOVERSION 2)
target_include_directories(libimpala PRIVATE . ${THORIN2_DIR}/ ${THORIN2_DIR}/build/half-src/include)

# impala

add_executable(impala
    driver/main.cpp
)
set_target_properties(impala PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_include_directories(impala PRIVATE . ${THORIN2_DIR}/ ${THORIN2_DIR}/build/half-src/include)
target_link_libraries(impala libimpala)

# impala-gtest

if(BUILD_TESTS)
    include(CTest)
    include(GoogleTest)

    # download and unpack gtest at configure time
    configure_file(CMakeLists.txt.gtest.in googletest-download/CMakeLists.txt)

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build.
    # This defines the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build)

    add_executable(impala-gtest
        test/lexer.cpp
        test/main.cpp
    )
    set_target_properties(impala-gtest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    target_include_directories(impala-gtest PRIVATE . ${THORIN2_DIR}/ ${THORIN2_DIR}/build/half-src/include)
    target_link_libraries(impala-gtest libimpala gtest_main)
    gtest_discover_tests(impala-gtest)
endif()
