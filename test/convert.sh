#!/bin/bash

for table in test/bad_ipac_tables/*; do
    build/tablator $table bad_table.tbl 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table"
    else
        echo "PASS: $table"
    fi
done

for table in test/*.tbl; do
    echo "Testing conversion of ${table}"
    build/tablator $table test.tbl
    rm test.tbl
    build/tablator $table test.hdf5
    rm test.hdf5
    build/tablator $table test.xml
    rm test.xml
    build/tablator $table test.csv
    rm test.csv
    build/tablator $table test.tsv
    rm test.tsv
    build/tablator $table test.fits
    rm test.fits
    build/tablator $table test.html
    rm test.html
done
