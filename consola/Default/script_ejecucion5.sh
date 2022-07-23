#!/bin/bash -x
{ ./consola ./../tests/TLB_1 300 &
{ sleep 0.05; ./consola ./../tests/TLB_2 300; } & } 