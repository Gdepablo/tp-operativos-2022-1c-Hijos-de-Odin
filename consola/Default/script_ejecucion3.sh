#!/bin/bash -x
{ /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/SUSPE_1 300 &
{ sleep 0.05; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/SUSPE_2 300; } & 
{ sleep 0.1; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/SUSPE_3 300;} }