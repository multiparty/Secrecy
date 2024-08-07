# CMakeLists.txt in include/external-lib/nlohmann
cmake_minimum_required(VERSION 3.15)

project(nlohmann_json LANGUAGES CXX)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
