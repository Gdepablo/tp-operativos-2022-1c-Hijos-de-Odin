#!/bin/bash -x
{ /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/INTEGRAL_1 2048 &
{ sleep 0.1; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/INTEGRAL_2 2048; } & 
{ sleep 0.2; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/INTEGRAL_3 2048;} 
{ sleep 0.3; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/INTEGRAL_4 2048;} 
{ sleep 0.4; /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/Default/consola /home/utnso/workspace/tp-2022-1c-Hijos-de-Odin/consola/tests/INTEGRAL_5 2048;} }