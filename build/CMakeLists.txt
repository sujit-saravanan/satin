cmake_minimum_required(VERSION 3.23)
project(satin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../output)


add_library(lexer STATIC ../modules/lexer/lexer.cpp)
target_include_directories(lexer PUBLIC ../modules/lexer/include)



add_executable(${PROJECT_NAME} ../source/main.cpp)

target_include_directories(lexer PUBLIC ../modules/pch/include)
target_precompile_headers(lexer PUBLIC ../modules/pch/include/pch.h)

target_link_libraries(${PROJECT_NAME} PUBLIC lexer)

