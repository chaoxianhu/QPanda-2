cmake_minimum_required(VERSION 3.1)
project(QPanda.Core.test)
file(GLOB_RECURSE QPandaCore_TEST_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE QPandaCore_TEST_CPP ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )
add_executable(${PROJECT_NAME} ${QPandaCore_TEST_H} ${QPandaCore_TEST_CPP})
target_link_libraries(${PROJECT_NAME} ${LIB_QPANDA}  ${LIB_GTEST})
