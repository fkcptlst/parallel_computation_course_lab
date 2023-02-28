#!/bin/bash

# compile
g++ -o lab1_montecarlo_serial.o lab1_montecarlo_serial.cpp
g++ -pthread -o lab1_montecarlo_parallel.o lab1_montecarlo_parallel.cpp