cmake_minimum_required (VERSION 2.8.12)
project (fixed-point-compiler)

# Needed to build on VS
if(CMAKE_COMPILER_IS_MSVC)
	set(gtest_force_shared_crt ON CACHE BOOL "ON" FORCE)
endif()

# This generates compile_commands.json for clang-check
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_FORMAT NAMES clang-format clang-format-3.6 clang-format-3.5 clang-format-3.4)

if (!CMAKE_COMPILER_IS_MSVC)
    find_program(CLANG_CHECK NAMES clang-check clang-check-3.6 clang-check-3.5 clang-check-3.4)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
        # For stacktraces:
        #add_definitions(-rdynamic -fstack-protector-all)
        add_definitions(-fstack-protector-all)
        # Enable maximum of Warnings :
        add_definitions(-Wall -Wextra -Wswitch-default -Wswitch-enum -Winit-self -Wformat-security -Wfloat-equal -Wcast-qual -Wconversion -Wlogical-op -Winline)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9")
            add_definitions (-Wfloat-conversion)
            add_definitions (-Wshadow)
        endif ()
endif()

set (SHARED_SRCS fixed_point_compiler.cc range.cc scale.cc operations.cc program.cc fixed_point_compiler.h)
set (TEST_SRCS test/simple_test.cc)
include_directories (.)

add_custom_target(format ALL
	COMMAND ${CLANG_FORMAT} -i -style Google ${SHARED_SRCS} ${TEST_SRCS}
	WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
	COMMENT "Running clang-format"
	VERBATIM)

if (!CMAKE_COMPILER_IS_MSVC)
	add_custom_target(check ALL
		COMMAND ${CLANG_CHECK} -analyze -p ${CMAKE_BINARY_DIR} ${SHARED_SRCS} ${TEST_SRCS}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		COMMENT "Running clang-check"
		VERBATIM)
endif()

add_executable(main main.cc ${SHARED_SRCS})

# add the subdirectory containing the CMakeLists.txt for the gtest library
add_subdirectory(googletest)

include_directories("${PROJECT_SOURCE_DIR}/googletest/include")

# simple test
cxx_test (simple_test gtest_main)

enable_testing()
