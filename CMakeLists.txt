cmake_minimum_required(VERSION 3.20)
project(kvadraOS4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow REQUIRED)

add_executable(kvadraOS4 main.cpp
        file-filter.hpp
        naive-file-filter.hpp)
target_link_libraries(kvadraOS4 PRIVATE Crow::Crow)
