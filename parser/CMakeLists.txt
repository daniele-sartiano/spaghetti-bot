cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

project(parser)

set(CMAKE_CXX_STANDARD 14)

#########################################################################
# Lexbor

ExternalProject_Add(
        Lexbor

        GIT_REPOSITORY "https://github.com/lexbor/lexbor.git"
        GIT_TAG "master"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""

        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/lexbor"
        CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_property(Lexbor install_dir)
include_directories(${install_dir}/include)
add_library(LexborLib UNKNOWN IMPORTED)
set_target_properties(LexborLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/liblexbor.so)
add_dependencies(LexborLib Lexbor)

#########################################################################
# Nats C Client

ExternalProject_Add(
        NatsCClient

        GIT_REPOSITORY "https://github.com/nats-io/nats.c.git"
        GIT_TAG "master"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""

        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/natsc"
        CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_property(NatsCClient install_dir)
include_directories(${install_dir}/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/natsc/src)
add_library(NatsCClientLib UNKNOWN IMPORTED)
set_target_properties(NatsCClientLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libnats.so)
add_dependencies(NatsCClientLib NatsCClient)

#########################################################################
# GumboParser

ExternalProject_Add(
        GumboParser

        GIT_REPOSITORY "https://github.com/google/gumbo-parser.git"
        GIT_TAG "master"

        SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/gumbo-parser
        BINARY_DIR ${CMAKE_SOURCE_DIR}/3rdparty/gumbo-parser
        CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND ${MAKE}
        INSTALL_COMMAND make install
)

ExternalProject_Get_property(GumboParser install_dir)
include_directories(${install_dir}/include)
add_library(GumboParserLib UNKNOWN IMPORTED)
set_target_properties(GumboParserLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libgumbo.so)
add_dependencies(GumboParserLib GumboParser)
#########################################################################
# Common lib
include_directories(${CMAKE_SOURCE_DIR}/common/include)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
add_library(libcommon SHARED IMPORTED)
set_property(TARGET libcommon PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/common/libcommon.so")

#########################################################################
# Download RapidJSON
ExternalProject_Add(
        rapidjson
        PREFIX "vendor/rapidjson"
        GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
        GIT_TAG f54b0e47a08782a6131cc3d60f94d038fa6e0a51
        TIMEOUT 10
        CMAKE_ARGS
        -DRAPIDJSON_BUILD_TESTS=OFF
        -DRAPIDJSON_BUILD_DOC=OFF
        -DRAPIDJSON_BUILD_EXAMPLES=OFF
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
)

# Prepare RapidJSON (RapidJSON is a header-only library)
ExternalProject_Get_Property(rapidjson source_dir)
set(RAPIDJSON_INCLUDE_DIR ${source_dir}/include)
include_directories(${RAPIDJSON_INCLUDE_DIR})
#########################################################################

add_executable(parser main.cpp src/Parser.cpp src/Parser.h)
add_dependencies(parser rapidjson)

TARGET_LINK_LIBRARIES(parser LexborLib)
TARGET_LINK_LIBRARIES(parser GumboParserLib)
TARGET_LINK_LIBRARIES(parser NatsCClientLib)
TARGET_LINK_LIBRARIES(parser libcommon)
