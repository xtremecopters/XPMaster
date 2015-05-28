#!/bin/bash

#export LANG=en_US.ISO-8859-1
cmake -D CMAKE_BUILD_TYPE=Debug .

# build project
make ${@}
