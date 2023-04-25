cmake_minimum_required(VERSION 3.22)
project(cserial)

set(CMAKE_CXX_STANDARD 17)

IF (WIN32)
    message("cserial WIN32")
    add_library(cserial SHARED
            cserial/impl_win32.c
            cserial/common.c)
    target_include_directories(cserial PUBLIC win32)
ELSE()
    message("cserial POSIX")
    add_library(cserial SHARED
            cserial/impl_posix.c
            cserial/common.c)
    target_include_directories(cserial PUBLIC posix)
ENDIF()

add_executable(test_posix test/test.c)
target_link_libraries(test_posix cserial)
