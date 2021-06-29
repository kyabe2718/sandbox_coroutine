cmake_minimum_required(VERSION 3.19)
project(sandbox_coroutine)

#set(CMAKE_CXX_COMPILER g++-10)
#set(CMAKE_C_COMPILER gcc-10)
#add_compile_options(-std=c++20 -fcoroutines)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)
add_compile_options(-std=c++20 -fcoroutines-ts -stdlib=libc++)

set(CMAKE_CXX_FLAGS "-O0 -g3")

add_library(coro INTERFACE)
target_include_directories(coro INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

file(GLOB SANDBOX_SOURCES ${CMAKE_CURRENT_LIST_DIR}/sandbox/*.cpp)
foreach (SOURCE ${SANDBOX_SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_executable(sandbox_${NAME} ${SOURCE})
    target_link_libraries(sandbox_${NAME} coro)
endforeach ()

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
add_executable(test ${TEST_SOURCES})
target_link_libraries(test coro)
target_include_directories(test PRIVATE external/doctest/)
