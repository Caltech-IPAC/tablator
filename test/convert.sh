#!/bin/bash

for table in test/bad_ipac_tables/* test/bad_votables/*; do
    build/tablator $table bad_table.tbl 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table"
    else
        echo "PASS: $table"
    fi
done

for table in test/*.tbl; do
    for ending in tbl hdf5 xml csv tsv fits html; do
        build/tablator $table test.$ending
        if [ $? -eq 0 ]; then
            echo "PASS: $table -> $ending"
        else
            echo "FAIL: $table -> $ending"
        fi
        rm test.$ending
    done
done
