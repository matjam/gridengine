# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

############ VCPKG CONFIGURATION

if(WIN32)
    message("************************************")
    message("Building for Windows ... you mad man")
    message("************************************")
    set(CMAKE_TOOLCHAIN_FILE "C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake")
    include_directories(SYSTEM "C:/dev/vcpkg/installed/x64-windows/include")
elseif(APPLE)
    message("************************************")
    message("building for MacOS ... cool")
    message("************************************")
    set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    include_directories(SYSTEM "/usr/local/vcpkg/installed/x64-mac/include")
    set(MAKE_INSTALL_NAME_DIR @rpath)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    message("************************************")
    message("building for Linux ... fanboi")
    message("************************************")
    set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    include_directories(SYSTEM "/usr/local/vcpkg/installed/x64-linux/include")
else()
    message( FATAL_ERROR "Unsupported platform." )
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
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*,boost-*,misc-*,modernize-*,performance-*,portability-*,-modernize-avoid-bind,-modernize-use-trailing-return-type")

file(GLOB RESOURCE_DATA data/*)
file(GLOB RESOURCE_SCRIPTS data/scripts/*)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# spdlog uses fmt, but I also want to use fmt.
add_definitions(-DSPDLOG_FMT_EXTERNAL)

file(GLOB gridengine_sources source/*.cpp source/engine/*.cpp)

include_directories(external/sol2/single/include)

# find all packages
include(FindLua)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Lua REQUIRED)
find_package(Freetype REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS window system audio graphics)

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

target_include_directories(gridengine PRIVATE engine/source)
target_include_directories(gridengine PRIVATE external/SelbaWard/src)
target_include_directories(gridengine PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(gridengine PRIVATE ${FREETYPE_LIBRARIES})
target_link_libraries(gridengine PRIVATE sfml-window sfml-system sfml-graphics sfml-audio)
target_link_libraries(gridengine PRIVATE spdlog::spdlog)
target_link_libraries(gridengine PRIVATE fmt::fmt)
target_link_libraries(gridengine PRIVATE Boost::filesystem)
target_link_libraries(gridengine PRIVATE FLAC OpenAL OpenGL)
target_link_libraries(gridengine PRIVATE ${LUA_LIBRARIES})
# target_link_libraries(gridengine PRIVATE selbaward)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    target_link_libraries(gridengine PRIVATE X11 dl UDev GL)
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
