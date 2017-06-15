#!/bin/bash

for table in test/bad_ipac_tables/* test/bad_votables/*; do
    build/tablator $table bad_table.hdf5 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table"
    else
        echo "PASS: $table"
    fi
done

for table in test/multi test/multi.csv test/multi.tsv test/fits_medium.fits test/*.tbl test/*.json5 test/*.xml test/*.unk; do
    for ending in tbl hdf5 xml csv tsv fits html json json5 postgres sqlite oracle db; do
        if [ $ending == "fits" ]; then
            build/tablator --stream-intermediate=yes $table test.$ending
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

build/tablator --input-format=hdf5 --output-format=ipac_table test/h5_as_csv.csv temp.h5
build/tablator --input-format=ipac_table temp.h5 temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Explicit format specified"
else
    echo "FAIL: Explicit format specified"
fi
rm -f temp.tbl temp.h5

build/tablator --output-format=votable test/recursive_param.xml - | diff - test/recursive_param.xml

if [ $? -eq 0 ]; then
    echo "PASS: recursive param tabledata"
else
    echo "FAIL: recursive param tabledata"
fi

build/tablator --output-format=votable test/recursive_param_binary2.xml  - | diff - test/recursive_param.xml
if [ $? -eq 0 ]; then
    echo "PASS: recursive param binary2"
else
    echo "FAIL: recursive param binary2"
fi

./build/tablator --output-format=csv test/al.csv - | diff -w - test/al.csv
if [ $? -eq 0 ]; then
    echo "PASS: CSV implicit float"
else
    echo "FAIL: CSV implicit float"
fi

./build/tablator --output-format=html test/multi.tbl - | diff -w - test/multi.html
if [ $? -eq 0 ]; then
    echo "PASS: HTML retain links"
else
    echo "FAIL: HTML retain links"
fi

./build/tablator --output-format=postgres test/multi.tbl - | diff -w - test/multi.postgres
if [ $? -eq 0 ]; then
    echo "PASS: Postgres output"
else
    echo "FAIL: Postgres output"
fi

./build/tablator --output-format=oracle test/multi.tbl - | diff -w - test/multi.oracle
if [ $? -eq 0 ]; then
    echo "PASS: Oracle output"
else
    echo "FAIL: Oracle output"
fi

./build/tablator --output-format=sqlite test/multi.tbl - | diff -w - test/multi.sqlite
if [ $? -eq 0 ]; then
    echo "PASS: SQLite output"
else
    echo "FAIL: SQLite output"
fi

./build/tablator --output-format=text test/empty_ipac_table - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty IPAC Table"
else
    echo "PASS: Empty IPAC Table"
fi

./build/tablator --output-format=text --input-format=csv test/empty - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty IPAC Table"
else
    echo "PASS: Empty IPAC Table"
fi

./build/tablator --output-format=text --input-format=votable test/empty_votable - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty VOTable"
else
    echo "PASS: Empty VOTable"
fi
