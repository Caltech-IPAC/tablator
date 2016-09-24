#!/bin/bash

for table in test/bad_ipac_tables/* test/bad_votables/*; do
    build/tablator $table bad_table.tbl 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table"
    else
        echo "PASS: $table"
    fi
done

for table in test/multi test/multi.csv test/multi.tsv test/fits_medium.fits test/*.tbl test/*.json5 test/*.xml; do
    for ending in tbl hdf5 xml csv tsv fits html json json5; do
        if [ $ending == "fits" ]; then
            build/tablator $table --stream-intermediate test.$ending
        else
            build/tablator $table test.$ending
        fi
        if [ $ending == "hdf5" ] || [ $ending == "xml" ] || [ $ending == "fits" ] || [ $ending == "json" ] || [ $ending == "tbl" ]; then
            build/tablator test.$ending temp.tbl
        fi
        if [ $? -eq 0 ]; then
            echo "PASS: $table <-> $ending"
        else
            echo "FAIL: $table <-> $ending"
        fi
        rm -f test.$ending temp.tbl
    done
done
