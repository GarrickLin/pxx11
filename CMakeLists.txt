cmake_minimum_required(VERSION 3.0)
project(cxx11)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

file( GLOB APP_SOURCES examples/*.cpp )
foreach( testsourcefile ${APP_SOURCES} )
    get_filename_component(testname ${testsourcefile} NAME_WE)   
    message(STATUS "add_executable ${testname} ${testsourcefile}")
    add_executable( ${testname} ${testsourcefile})
    # set_target_properties(${testname} PROPERTIES COMPILE_FLAGS "-save-temps")
endforeach( testsourcefile ${APP_SOURCES} )