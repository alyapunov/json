cmake_minimum_required(VERSION 3.5)
project(json)

SET(DEFAULT_BUILD_TYPE "Debug")
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
ENDIF()
SET(EXPECTED_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
IF("${CMAKE_BUILD_TYPE}" IN_LIST EXPECTED_BUILD_TYPES)
    MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
ELSE()
    MESSAGE(WARNING "Unexcpected build type: ${CMAKE_BUILD_TYPE}")
ENDIF()

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
SET(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror")

set(SOURCE_FILES_JSON_ENCODE_TEST
        JsonEncoderTest.cpp
        JsonEncoder.h
        JsonEncoder.hpp
        Common.h
        Common.cpp
)

add_executable(jsonenc.test ${SOURCE_FILES_JSON_ENCODE_TEST})

set(SOURCE_FILES_JSON_DECODE_TEST
        JsonDecoderTest.cpp
        JsonDecoder.h
        JsonDecoder.hpp
        Common.h
        Common.cpp
)

add_executable(jsondec.test ${SOURCE_FILES_JSON_DECODE_TEST})

enable_testing()
add_test(NAME jsonenc.test COMMAND jsonenc.test)
add_test(NAME jsondec.test COMMAND jsondec.test)
