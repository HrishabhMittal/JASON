#/bin/bash

make
for i in ./tests/json-checker/*; do
    echo ----------------------------------------------
    echo ---------------------FILE---------------------
    echo checking on $i
    # cat $i
    echo ------------------CODE OUTPUT-----------------
    ./main $i
done
