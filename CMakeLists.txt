cmake_minimum_required(VERSION 3.12)
project(shunting-yard)
option(BUILD_STATIC_LIBS "Build static libraries?" ON)

if(BUILD_STATIC_LIBS) 
    add_library(math_parser STATIC src/math_parser.cpp src/token_evaluators.cpp)
    target_compile_features(math_parser PUBLIC cxx_std_20)
endif(BUILD_STATIC_LIBS)