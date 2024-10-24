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

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g)
add_definitions(-DREDISCPP_HEADER_ONLY)

include_directories("${PROJECT_SOURCE_DIR}/..")
include_directories("${PROJECT_SOURCE_DIR}/../plugin/linux")
include_directories("${PROJECT_SOURCE_DIR}/../include")
include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

ADD_LIBRARY(fb ${SOURCE_FILES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
install(TARGETS fb DESTINATION /usr/local/lib)