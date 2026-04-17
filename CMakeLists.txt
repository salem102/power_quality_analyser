cmake_minimum_required(VERSION 3.27)
project(power_analyser C)

set(CMAKE_C_STANDARD 99)

add_executable(power_analyser main.c waveform.c io.c)
target_link_libraries(power_analyser m)
