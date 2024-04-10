#!/bin/bash

# Проверка количества аргументов
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <port> <-n/-des/-dh>"
    exit 1
fi

port=$1
option=$2

if [ "$option" == "-n" ]; then
    ./build/server $port 0
elif [ "$option" == "-des" ]; then
    ./build/server $port 1
elif [ "$option" == "-dh" ]; then
    ./build/server $port 2
else
    echo "Usage: $0 <port> [-n/-des/-dh]"
fi