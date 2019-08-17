#!/bin/bash
set -e -x
qmake orion.pro
make -j4
