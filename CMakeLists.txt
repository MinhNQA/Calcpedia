cmake_minimum_required(VERSION 3.10)
project(calcpedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(calcpedia
    src/main.cpp
    src/eval.cpp
    src/plot.cpp
    src/handbook.cpp
    src/explain.cpp
    src/repl.cpp
)

if(MSVC)
    target_compile_options(calcpedia PRIVATE /W4 /permissive-)
else()
    target_compile_options(calcpedia PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(calcpedia PRIVATE m)
endif()

install(TARGETS calcpedia RUNTIME DESTINATION bin)
