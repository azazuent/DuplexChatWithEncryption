#!/bin/bash

# Проверка количества аргументов
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <hostname> <port> <-n/-des/-dh>"
    exit 1
fi

hostname=$1
port=$2
option=$3

if [ "$option" == "-n" ]; then
    ./build/client $hostname $port 0
elif [ "$option" == "-des" ]; then
    ./build/client $hostname $port 1
elif [ "$option" == "-dh" ]; then
    ./build/client $hostname $port 2
else
    echo "Usage: $0 <port> [-n/-des/-dh]"
fi