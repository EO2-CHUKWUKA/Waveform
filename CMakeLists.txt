cmake_minimum_required(VERSION 3.21)
project(power_coursework C)

set(CMAKE_C_STANDARD 99)

add_executable(power_coursework main.c InputOutput.h InputOutput.c Waveform.h Waveform.c)
