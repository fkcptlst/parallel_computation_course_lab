#!/bin/bash

# compile
g++ -o serial.o serial.cpp
g++ -pthread -o parallel.o parallel.cpp