cmake_minimum_required(VERSION 3.15.3 FATAL_ERROR)

project(htwatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# https://wiki.gentoo.org/wiki/GCC_optimization
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -march=native -Wall -Wextra -Wno-unused-parameter -ggdb -msse -msse2 -msse3 -msse4.1")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra -Wno-unused-parameter -g -ggdb -msse -msse2 -msse3 -msse4.1")
endif()

message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")


set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)

set(STAGED_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/stage)
message(STATUS "${PROJECT_NAME} staged install: ${STAGED_INSTALL_PREFIX}")


add_subdirectory(external/upstream)


find_library(numa numa)

add_compile_options(-DBOOST_STACKTRACE_USE_BACKTRACE)
add_compile_options(-DBOOST_STACKTRACE_LINK)
find_package(Backtrace REQUIRED)

if (Backtrace_FOUND)
    if (NOT Backtrace_LIBRARIES)
        message (STATUS "Fix Backtrace_LIBRARIES")
        set(Backtrace_LIBRARIES -lbacktrace)
    endif ()
    message(STATUS "Backtrace Backtrace_HEADER '${Backtrace_HEADER}', Backtrace_INCLUDE_DIRS '${Backtrace_INCLUDE_DIRS}', Backtrace_LIBRARIES '${Backtrace_LIBRARIES}', Backtrace_LIBRARY '${Backtrace_LIBRARY}', Backtrace_INCLUDE_DIR '${Backtrace_INCLUDE_DIR}'")
else ()
    message(STATUS "Backtrace Not Found")
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(htwatcher
        src/htwatcher.cpp
        )

add_dependencies(htwatcher
        dpdk_external
        )

target_link_libraries(htwatcher
        PRIVATE
        -rdynamic
        dpdk_headers
        -Wl,--whole-archive ${DPDK_LIBRARIES} -Wl,--no-whole-archive
        numa
        pcap
        Threads::Threads
        ${CMAKE_DL_LIBS}
        ${Backtrace_LIBRARIES}
        )
