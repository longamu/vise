#!/bin/sh
toExec=../../build/tests/eval_qe

time $toExec oxc1_5k s o
time $toExec oxc1_5k h o
time $toExec ox100k h o
time $toExec ox100k s o
time $toExec oxc1_5k s p
#time $toExec oxc1_5k h p
#time $toExec ox100k h p
time $toExec ox100k s p
