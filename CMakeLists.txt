cmake_minimum_required(VERSION 3.28)

IF (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
ENDIF (POLICY CMP0048)

project(fb
        VERSION 1.0.0
        DESCRIPTION "2D mmorpg server"
        HOMEPAGE_URL "https://github.com/boyism80/fb"
        LANGUAGES CXX
)

# Defined before any header. Without it, windows.h includes winsock.h and
# the wsock32 import library collides with ws2_32.
if(WIN32)
  # winsock2.h warns, and then guesses, when the target Windows version is unset.
  add_compile_definitions(WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0601)
endif()

add_subdirectory(server/fb)
add_subdirectory(server/gateway)
add_subdirectory(server/login)
add_subdirectory(server/game)
add_subdirectory(server/bot)

add_dependencies(gateway fb)
add_dependencies(login fb)
add_dependencies(game fb)
add_dependencies(bot fb)