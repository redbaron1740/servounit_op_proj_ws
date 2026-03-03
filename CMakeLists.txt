cmake_minimum_required(VERSION 3.10)

#프로젝트 설정
project(Servounit_Test_Program VERSION 1.0 LANGUAGES CXX)

#C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일러 플래그 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


# 소스 파일
set(SOURCES
    src/main.cpp
)

add_executable(servounit_test_program ${SOURCES})

# Include directories
# Add the inc directory for comm_pcan.hpp and other headers
target_include_directories(servounit_test_program PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    /usr/include
)

# PCANBasic library (update the path as needed)
# If you have PCANBasic.h and libPCANBasic.so, set the include and link directories
target_link_libraries(servounit_test_program PRIVATE
    /usr/lib/libpcanbasic.so
    ncurses
    pthread
)
