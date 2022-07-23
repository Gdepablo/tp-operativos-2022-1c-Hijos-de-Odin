#!/bin/bash -x
{ ./consola ./../tests/INTEGRAL_1 2048 &
{ sleep 0.1; ./consola ./../tests/INTEGRAL_2 2048; } & 
{ sleep 0.2; ./consola ./../tests/INTEGRAL_3 2048;} 
{ sleep 0.3; ./consola ./../tests/INTEGRAL_4 2048;} 
{ sleep 0.4; ./consola ./../tests/INTEGRAL_5 2048;} }