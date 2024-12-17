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

add_subdirectory(lib)
add_subdirectory(gateway)
add_subdirectory(login)
add_subdirectory(game)

add_dependencies(gateway fb)
add_dependencies(login fb)
add_dependencies(game fb)