#!/bin/sh -xe

rm -f FIFO
mkfifo FIFO
./xrso12832 -r $* -o FIFO &
sleep 1
dieharder -a -g 201 -f FIFO

