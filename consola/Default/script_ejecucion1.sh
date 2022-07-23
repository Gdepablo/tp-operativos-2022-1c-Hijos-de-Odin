#!/bin/bash -x
{ ./consola ./../tests/BASE_1 300 &
{ sleep 0.05; ./consola ./../tests/BASE_2 300; } & 
{ sleep 0.1; ./consola ./../tests/BASE_2 300;} }