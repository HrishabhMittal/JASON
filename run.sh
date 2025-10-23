#!/bin/bash
make
time for file in ./tests/*; do
    ./main "$file"
done
time for file in ./tests/*; do 
    jq -e . < "$file" > /dev/null 2>&1
done
