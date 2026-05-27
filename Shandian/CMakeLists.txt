cmake_minimum_required(VERSION 3.10)
project(Shandian LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -fvisibility=hidden -flto -Os")

add_library(Shandian SHARED
    main.cpp
    hooks.cpp
    tick.cpp
    cheat/combat.cpp
    cheat/esp.cpp
    cheat/minimap.cpp
    menu/menu.cpp
)

target_include_directories(Shandian PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR})
target_link_libraries(Shandian PRIVATE log m)
set_target_properties(Shandian PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
