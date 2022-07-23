#!/bin/bash -x
{ ./consola ./../tests/PLANI_1 300 &
{ sleep 0.05; ./consola ./../tests/PLANI_2 300; } & 
{ sleep 0.1;./consola ./../tests/PLANI_2 300;} }