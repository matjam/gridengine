# CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# ########### VCPKG CONFIGURATION
# Prefer VCPKG_ROOT env var, fall back to platform-specific defaults
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    elseif(WIN32)
        set(CMAKE_TOOLCHAIN_FILE "C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake")
    elseif(APPLE)
        set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    else()
        message(FATAL_ERROR "Unsupported platform. Set VCPKG_ROOT or CMAKE_TOOLCHAIN_FILE.")
    endif()
endif()

if(WIN32)
    message("************************************")
    message("Building for Windows ... you mad man")
    message("************************************")
elseif(APPLE)
    message("************************************")
    message("building for MacOS ... cool")
    message("************************************")
    set(MAKE_INSTALL_NAME_DIR @rpath)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    message("************************************")
    message("building for Linux ... fanboi")
    message("************************************")
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()

project(gridengine
    VERSION 0.1
    DESCRIPTION "gridengine"
    LANGUAGES CXX)

# Set version information
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*,boost-*,misc-*,modernize-*,performance-*,portability-*,-modernize-avoid-bind,-modernize-use-trailing-return-type")
set(CMAKE_CXX_STANDARD 20)

file(GLOB RESOURCE_DATA data/*)
file(GLOB RESOURCE_SCRIPTS data/scripts/*)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# spdlog uses fmt, but I also want to use fmt.
add_definitions(-DSPDLOG_FMT_EXTERNAL)

file(GLOB gridengine_sources source/game/*.cpp source/engine/*.cpp)

# find all packages
include(FindLua)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Lua REQUIRED)
find_package(Freetype REQUIRED)
find_package(SFML COMPONENTS Window System Audio Graphics CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)

# Find OS X libraries
if(APPLE)
    # Add Foundation
    find_library(FOUNDATION_LIBRARY Foundation)
    list(APPEND PROJECT_LIBRARIES ${FOUNDATION_LIBRARY})
    include_directories(source/osx)
    add_executable(gridengine MACOSX_BUNDLE source/osx/resourcePath.mm ${gridengine_sources} ${RESOURCE_DATA} ${RESOURCE_SCRIPTS})
    set_source_files_properties(${RESOURCE_DATA} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/data)
    set_source_files_properties(${RESOURCE_SCRIPTS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/data/scripts)
else()
    add_executable(gridengine ${gridengine_sources})
endif()

set_property(TARGET gridengine PROPERTY CXX_STANDARD 17)
set_property(TARGET gridengine PROPERTY C_STANDARD 11)

target_sources(gridengine PRIVATE ${gridengine_sources})

target_include_directories(gridengine PRIVATE source)
target_include_directories(gridengine PRIVATE source/engine)
target_include_directories(gridengine PRIVATE source/game)
target_include_directories(gridengine PRIVATE external/SelbaWard/src)
target_include_directories(gridengine PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(gridengine PRIVATE ${FREETYPE_LIBRARIES})
target_link_libraries(gridengine PRIVATE SFML::Window SFML::System SFML::Graphics SFML::Audio)
target_link_libraries(gridengine PRIVATE spdlog::spdlog)
target_link_libraries(gridengine PRIVATE fmt::fmt)
target_link_libraries(gridengine PRIVATE Boost::filesystem)
target_link_libraries(gridengine PRIVATE ${LUA_LIBRARIES})
target_link_libraries(gridengine PRIVATE sol2::sol2)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    target_link_libraries(gridengine PRIVATE dl)
endif()

if(APPLE)
    set_target_properties(gridengine PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(gridengine PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/source/osx/Info.plist)

    # Copy the assets and levels into the bundle
    set(BUNDLE_PATH ${CMAKE_CURRENT_BINARY_DIR}/gridengine.app)

    install(CODE "
		include(BundleUtilities)
        fixup_bundle(\"${BUNDLE_PATH}\" \"\" \"${LIBRARIES_DIR}\")
		" COMPONENT Runtime
    )
endif()

# cotire(gridengine)
