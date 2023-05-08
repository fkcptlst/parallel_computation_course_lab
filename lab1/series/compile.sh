#!/bin/bash

# compile
g++ -o lab1_series_serial.o lab1_series_serial.cpp
g++ -pthread -o lab1_series_parallel.o lab1_series_parallel.cpp