#!/bin/bash -x
{ ./consola ./../tests/TLB_1 2048 &
{ sleep 0.05; ./consola ./../tests/TLB_2 2048; } & } 