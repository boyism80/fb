cmake_minimum_required(VERSION 3.28)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(fb,
        VERSION 1.0.0
        DESCRIPTION "2D mmorpg game server"
        HOMEPAGE_URL "https://github.com/boyism80/fb"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g)
add_definitions(-DREDISCPP_HEADER_ONLY)

include_directories("${PROJECT_SOURCE_DIR}/..")
include_directories("${PROJECT_SOURCE_DIR}/../plugin/linux")
include_directories("${PROJECT_SOURCE_DIR}/../include")
include_directories("${PROJECT_SOURCE_DIR}/../lib/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("/usr/include/mysql")

add_compile_definitions(ENABLE_MYSQL)

file(GLOB_RECURSE CORE_FILES "${PROJECT_SOURCE_DIR}/../core/*.cpp")
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/lib/*.cpp")

add_executable(app ${CORE_FILES} ${SOURCE_FILES} main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(app fb jsoncpp z pthread crypto lua5.3 boost_program_options cpp_redis tacopie rabbitmq cpp-terminal cpp-terminal-private)