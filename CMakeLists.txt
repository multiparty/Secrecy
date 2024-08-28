cmake_minimum_required(VERSION 3.19)

project(Secrecy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3")

# Adding Dependencies
add_subdirectory(include/external-lib/sql-parser)

# Adding MPI Library
find_package(MPI REQUIRED)
if (MPI_FOUND)
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
else (MPI_FOUND)
    message(SEND_ERROR "This application cannot compile without MPI")
endif (MPI_FOUND)

# Using libsodium
find_package(PkgConfig REQUIRED)
pkg_check_modules(_LIBSODIUM REQUIRED libsodium)

find_path(SODIUM_INCLUDE_DIR sodium.h HINTS ${LIBSODIUM_INCLUDE_DIRS} /usr/local/include /opt/local/include /opt/include)
find_library(SODIUM_LIBRARY NAMES sodium HINTS ${LIBSODIUM_LIBRARY_DIRS} /usr/local/lib /opt/local/lib /opt/lib)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/external-lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/external-lib/sql-parser/src)

# SecrecyLib
file(GLOB Secrecy_SOURCES src/core/*.cpp src/core/wrappers/*.cpp src/common/*.cpp src/benchmarking/*.cpp src/planner/*.cpp)

set(TEST_FILE src/tests/c_api/test_join_sail.cpp)
# Define the executable using only the specified test file
get_filename_component(fileName ${TEST_FILE} NAME_WLE)
# Add the executable and link the necessary libraries
add_executable(${fileName} ${TEST_FILE} ${Secrecy_SOURCES})
target_link_libraries(${fileName} PUBLIC ${MPI_LIBRARIES} ${SODIUM_LIBRARY} src)
target_include_directories(${fileName} PUBLIC ${SODIUM_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/external-lib)