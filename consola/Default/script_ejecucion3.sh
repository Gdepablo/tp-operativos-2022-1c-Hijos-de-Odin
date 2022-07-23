#!/bin/bash -x
{ ./consola ./../tests/SUSPE_1 300 &
{ sleep 0.05; ./consola ./../tests/SUSPE_2 300; } & 
{ sleep 0.1; /./consola ./../tests/SUSPE_3 300;} }