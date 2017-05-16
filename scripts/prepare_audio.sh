#!/bin/bash

for filename in ./Dataset/*; do
    sox "$filename" -r 16000 -c 1 "./Dataset_mono_16k/${filename##*/}"
done