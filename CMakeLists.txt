cmake_minimum_required(VERSION 3.28)
project(Cpp-Reflection)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 20)



add_library(Reflection STATIC)

set_target_properties(Reflection PROPERTIES LINKER_LANGUAGE CXX)

target_sources(Reflection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/Reflection.h ${CMAKE_CURRENT_SOURCE_DIR}/src/Reflection.cpp)
target_include_directories(Reflection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)


add_executable(Reflection_Test)

target_sources(Reflection_Test PUBLIC ${CMAKE_SOURCE_DIR}/test.cpp)
target_link_libraries(Reflection_Test PUBLIC Reflection)