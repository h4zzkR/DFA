#cmake_minimum_required(VERSION 3.20)
#project(deus_ex_automata)
#
#set(CMAKE_CXX_STANDARD 17)
#
#include_directories(src)
#
#add_executable(deus_ex_automata
#        src/automata.cpp
#        src/automata.h
#        main.cpp)

cmake_minimum_required(VERSION 3.20)
project(deus_ex_automata LANGUAGES CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TEST "Build the unit tests" ON)

if(BUILD_TEST)
    add_subdirectory(src)
    # we cannot analyse results without gcov
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(FATAL_ERROR "Code coverage analysis requires gcov!")
    endif()

    enable_testing()
    add_subdirectory(test)
else()
    include_directories(src)

    add_executable(deus_ex_automata
            src/automata.cpp
            src/automata.h
            main.cpp)
endif()
