cmake_minimum_required(VERSION 3.28)

IF (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
ENDIF (POLICY CMP0048)

IF(WIN32)
  set(DEST "bot")
ELSE()
  set(DEST "app")
ENDIF(WIN32)

project(fb,
        VERSION 1.0.0
        DESCRIPTION "2D mmorpg bot"
        HOMEPAGE_URL "https://github.com/boyism80/fb"
        LANGUAGES CXX
)

IF(WIN32)
  set(ROOT_DIR ${CMAKE_SOURCE_DIR})
ELSE()
  set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
ENDIF(WIN32)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DREDISCPP_HEADER_ONLY)
add_compile_definitions(BOT)

file(GLOB_RECURSE BOT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/lib/*.cpp")
file(GLOB_RECURSE GATEWAY_PROTOCOL_FILES "${ROOT_DIR}/gateway/lib/protocol/*.cpp")
file(GLOB_RECURSE LOGIN_PROTOCOL_FILES "${ROOT_DIR}/login/lib/protocol/*.cpp")
file(GLOB_RECURSE GAME_PROTOCOL_FILES "${ROOT_DIR}/game/lib/protocol/*.cpp")
add_executable(${DEST} ${BOT_SOURCE_FILES} ${GATEWAY_PROTOCOL_FILES} ${LOGIN_PROTOCOL_FILES} ${GAME_PROTOCOL_FILES} main.cpp)

include_directories("${ROOT_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}")

IF(WIN32)
  set(CMAKE_CXX_FLAGS "/source-charset:utf-8 /EHsc")
  string(REPLACE "/" "\\" DEPENDENCY_LIBDIR ${ROOT_DIR}/dependency/lib\\;${ROOT_DIR}/dependency/lib/boost\\;$(LibraryPath))
  set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES ${DEPENDENCY_LIBDIR})
  set(CMAKE_CXX_STANDARD_LIBRARIES "$(CMAKE_CXX_STANDARD_LIBRARIES) %(AdditionalDependencies) ")
  include_directories("${ROOT_DIR}/dependency/include")
  include_directories("${ROOT_DIR}/dependency/include/zlib")
  set_property(TARGET ${DEST} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_link_libraries(${DEST} fb$<$<CONFIG:Debug>:d>.lib
                                jsoncpp$<$<CONFIG:Debug>:d>.lib
                                librabbitmq.4$<$<CONFIG:Debug>:d>.lib
                                lua$<$<CONFIG:Debug>:d>.lib
                                zlib$<$<CONFIG:Debug>:d>.lib
                                flatbuffers$<$<CONFIG:Debug>:d>.lib
                                tacopie$<$<CONFIG:Debug>:d>.lib
                                cpp_redis$<$<CONFIG:Debug>:d>.lib
                                ws2_32)
ELSE()
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  add_compile_definitions(NCURSES_NOMACROS)
  find_package(Threads REQUIRED)
  target_link_libraries(${DEST} fb
                                jsoncpp
                                z
                                pthread
                                lua
                                boost_program_options
                                cpp_redis
                                tacopie
                                rabbitmq
                                ncursesw)
ENDIF(WIN32)