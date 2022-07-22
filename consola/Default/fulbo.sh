#!/bin/bash -x
{ echo 1 &
{ sleep 0.05; echo 2; } & 
{ sleep 0.1; echo 3;} }