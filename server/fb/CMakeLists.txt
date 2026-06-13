cmake_minimum_required(VERSION 3.28)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(fb,
        VERSION 1.0.0
        DESCRIPTION "2D mmorpg common library"
        HOMEPAGE_URL "https://github.com/boyism80/fb"
        LANGUAGES CXX
)

IF(WIN32)
  set(ROOT_DIR ${CMAKE_SOURCE_DIR})
ELSE()
  set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
ENDIF(WIN32)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(NOMINMAX)

file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
ADD_LIBRARY(fb ${SOURCE_FILES})

include(GNUInstallDirs)
include_directories("${ROOT_DIR}")
include_directories("${ROOT_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
IF(WIN32)
  include_directories("${ROOT_DIR}/dependency/include")
  include_directories("${ROOT_DIR}/dependency/include/zlib")
  set(CMAKE_CXX_FLAGS "/source-charset:utf-8 /EHsc")
  set_property(TARGET fb PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  cmake_path(APPEND DEPENDENCY_LIBDIR ${ROOT_DIR} dependency lib fb$<$<CONFIG:Debug>:d>.lib*)
  string(REGEX REPLACE "/" "\\\\" DEPENDENCY_LIBDIR ${DEPENDENCY_LIBDIR})
  add_custom_command(TARGET fb POST_BUILD COMMAND XCOPY /s /i $(OutputPath)fb.lib ${DEPENDENCY_LIBDIR} /y)
  
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_compile_definitions(KINGDOM_OF_THE_WIND)
  add_compile_definitions(AMQP_STATIC)
ELSE()
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -fsanitize=undefined -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=undefined")
  add_compile_definitions(NCURSES_NOMACROS)
ENDIF(WIN32)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


install(TARGETS fb DESTINATION ${CMAKE_INSTALL_LIBDIR})