#!/bin/bash

if ! [ -d lib/math/Eigen ]; then
    echo "Eigen not installed at lib/math/Eigen."
    echo "Run ./fetch_eigen.sh to install it."
    exit 1
fi

g++ \
-std=c++17 \
-I lib/error/ \
-I lib/math/ \
-I lib/flightmodule \
-I lib/IMU/include/ \
-I lib/IMU/src/ \
lib/flightmodule/Scheduler.cpp \
lib/flightmodule/Router.cpp \
lib/IMU/src/FakeIMU.cpp \
src/main_posix.cpp
