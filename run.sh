#/bin/bash

make
for i in ./tests/json-checker/*; do
    echo ----------------------------------------------
    echo ----------------------------------------------
    echo ------ FILE OUTPUT ------
    cat $i
    echo ------ CODE RESULT ------
    echo checking on $i
    ./main $i
done
