cmake_minimum_required(VERSION 3.0.0)
project(dublin VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(optimizer main.cpp src/tinyxml2.cpp hyls/hyls.cpp hyls/localsearch.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) 

include(CPack)
