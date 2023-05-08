#!/bin/bash

# compile
g++ -o lab1_integral_serial.o lab1_integral_serial.cpp
g++ -pthread -o lab1_integral_parallel.o lab1_integral_parallel.cpp