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
    build/tablator $table test.tbl
    build/tablator $table test.hdf5
    build/tablator $table test.xml
    build/tablator $table test.csv
    build/tablator $table test.tsv
    rm test.fits
    build/tablator $table test.fits
    build/tablator $table test.html
done
