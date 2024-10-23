#!/bin/bash


if [ "$#" -le 0 ]; then
    echo "Usage: ./hex.sh <image>"
    exit 1
fi

okteta $1 >/dev/null 2>&1
