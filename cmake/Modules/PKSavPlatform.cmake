#
# Copyright (c) 2015-2016 Nicholas Corgan (n.corgan@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)
#

# Set build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(CMAKE_COMPILER_IS_GNUC)
    SET(PKSAV_GCC TRUE)
ELSEIF(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    SET(PKSAV_CLANG TRUE)
ENDIF(CMAKE_COMPILER_IS_GNUC)

IF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(PKSAV_64BIT TRUE)
ELSE()
    SET(PKSAV_32BIT TRUE)
ENDIF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)

IF(PKSAV_GCC OR PKSAV_CLANG)
    SET(PKSAV_C_FLAGS "-O3 -std=gnu99 -Wall -Wextra -fvisibility=hidden")
ELSEIF(MSVC)
    ADD_DEFINITIONS(/MP)                       # Multi-threaded build
    ADD_DEFINITIONS(/EHsc)                     # Exception handling
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS) # Ignore deprecation warnings
ENDIF(PKSAV_GCC OR PKSAV_CLANG)

# Checks for required headers
INCLUDE(CheckIncludeFile)
SET(CMAKE_REQUIRED_FLAGS "${PKSAV_C_FLAGS}")
CHECK_INCLUDE_FILE(stdbool.h HAVE_STDBOOL_H)

# Set compiler name for CMake display
IF(MSVC)
    IF(MSVC12)
        SET(PKSAV_COMPILER_NAME "Microsoft Visual Studio 2013")
    ELSEIF(MSVC14)
        SET(PKSAV_COMPILER_NAME "Microsoft Visual Studio 2015")
    ELSE()
        SET(PKSAV_COMPILER_NAME "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    ENDIF(MSVC12)
ELSEIF(MINGW)
    SET(PKSAV_COMPILER_NAME "MinGW-${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
ELSE()
    SET(PKSAV_COMPILER_NAME "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
ENDIF(MSVC)

MESSAGE(STATUS "")
