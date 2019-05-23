#!/bin/bash

#############################################################################
## helper functions
#############################################################################

usage()
{
    script_name=$1
    echo "Usage: ${script_name} [-b path_to_tablator_binary]"
    echo ""
}


# Generate command line to exit the script without exiting the shell,
# whether source'd or run from screen or otherwise.
get_out()
{
   echo "return $1 2> /dev/null || exit $1"
}


#############################################################################
## main function
#############################################################################

tablator_bin="build/tablator"

OPTIND=1
while getopts ":b:h" opt; do
    case $opt in
        b ) tablator_bin=$OPTARG
            ;;
        * ) usage $0
            exit 1
            eval `get_out 1`
            ;;
    esac
done
shift $((OPTIND-1))



for table in test/bad_ipac_tables/* test/bad_votables/*; do
    ${tablator_bin} $table bad_table.hdf5 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table"
    else
        echo "PASS: $table"
    fi
done

###########################################################
# Expected errors

${tablator_bin} --input-format=json5 --output-format=ipac test/back_and_forth_tables/integer_type_arrays.json5 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Shouldn't be able to convert Json5 Table with array of large uint64 vals to IPAC format"
else
    echo "PASS: Attempt to convert Json5 Table with array of large uint64 vals to IPAC format correctly resulted in error"
fi


###########################################################

STREAM_INTERMEDIATE=""
for table in test/multi test/dos_ending.csv test/multi.csv test/multi.tsv test/fits_medium.fits test/*.tbl test/*.json5 test/*.xml test/upload_table.vo test/*.unk; do
    STREAM_INTERMEDIATE=""
    if [ $table != "test/fits_medium.fits" ] && [ $table != "test/multi_h5.unk" ] && [ $table != "test/multi_tsv.unk" ]; then
        STREAM_INTERMEDIATE="--stream-intermediate=yes"
    fi
    for ending in tbl hdf5 xml csv tsv fits html json json5 postgres sqlite oracle db; do
        if [ $ending == "db" ]; then
            ${tablator_bin} $table test.$ending
        else
            ${tablator_bin} $STREAM_INTERMEDIATE $table test.$ending
        fi
        if [ $ending == "hdf5" ] || [ $ending == "fits" ]; then
            ${tablator_bin} test.$ending temp.tbl
        elif [ $ending == "xml" ] || [ $ending == "json" ] || [ $ending == "tbl" ]; then
            ${tablator_bin} $STREAM_INTERMEDIATE test.$ending temp.tbl
        fi
        if [ $? -eq 0 ]; then
            echo "PASS: $table <-> $ending"
        else
            echo "FAIL: $table <-> $ending"
        fi
        rm -f test.$ending temp.tbl
    done
done

${tablator_bin} --input-format=hdf5 --output-format=ipac_table test/h5_as_csv.csv temp.h5
${tablator_bin} --input-format=ipac_table temp.h5 temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Explicit format specified"
else
    echo "FAIL: Explicit format specified"
fi
rm -f temp.tbl temp.h5

${tablator_bin} --output-format=votable test/recursive_param.xml - | diff - test/recursive_param.xml

if [ $? -eq 0 ]; then
    echo "PASS: recursive param tabledata"
else
    echo "FAIL: recursive param tabledata"
fi

${tablator_bin} --output-format=votable test/recursive_param_binary2.xml  - | diff - test/recursive_param.xml
if [ $? -eq 0 ]; then
    echo "PASS: recursive param binary2"
else
    echo "FAIL: recursive param binary2"
fi

${tablator_bin} --output-format=csv test/al.csv - | diff -w - test/al.csv
if [ $? -eq 0 ]; then
    echo "PASS: CSV implicit float"
else
    echo "FAIL: CSV implicit float"
fi

${tablator_bin} --output-format=html test/multi.tbl - | diff -w - test/multi.html
if [ $? -eq 0 ]; then
    echo "PASS: HTML retain links"
else
    echo "FAIL: HTML retain links"
fi

${tablator_bin} --output-format=postgres test/multi.tbl - | diff -w - test/multi.postgres
if [ $? -eq 0 ]; then
    echo "PASS: Postgres output"
else
    echo "FAIL: Postgres output"
fi

${tablator_bin} --output-format=oracle test/multi.tbl - | diff -w - test/multi.oracle
if [ $? -eq 0 ]; then
    echo "PASS: Oracle output"
else
    echo "FAIL: Oracle output"
fi

${tablator_bin} --output-format=sqlite test/multi.tbl - | diff -w - test/multi.sqlite
if [ $? -eq 0 ]; then
    echo "PASS: SQLite output"
else
    echo "FAIL: SQLite output"
fi

${tablator_bin} --output-format=csv test/multi.tbl - | diff -w - test/multi.csv
if [ $? -eq 0 ]; then
    echo "PASS: CSV output"
else
    echo "FAIL: CSV output"
fi

${tablator_bin} --output-format=text test/empty_ipac_table - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty IPAC Table"
else
    echo "PASS: Empty IPAC Table"
fi

${tablator_bin} --output-format=text --input-format=csv test/empty - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty IPAC Table"
else
    echo "PASS: Empty IPAC Table"
fi

${tablator_bin} --output-format=text --input-format=votable test/empty_votable - 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Empty VOTable"
else
    echo "PASS: Empty VOTable"
fi

cat test/multi.csv | ${tablator_bin} --input-format=csv --output-format=csv - - | diff -w - test/multi.csv
if [ $? -eq 0 ]; then
    echo "PASS: Read CSV From STDIN"
else
    echo "FAIL: Read CSV From STDIN"
fi

${tablator_bin} --input-format=tsv --output-format=text test/no_trailing_newline.tsv - > /dev/null
if [ $? -eq 0 ]; then
    echo "PASS: Read TSV with no trailing newline"
else
    echo "FAIL: Read TSV with no trailing newline"
fi

${tablator_bin} --input-format=ipac_table --output-format=votable test/int_types.tbl - | diff -w - test/int_types.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with large uint64 vals to VOTable"
else
    echo "FAIL: Convert IPAC Table with large uint64 vals to VOTable"
fi


${tablator_bin} --input-format=ipac_table --output-format=json5 test/int_types.tbl - | diff -w - test/int_types.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with large uint64 vals to JSON5"
else
    echo "FAIL: Convert IPAC Table with large uint64 vals to JSON5"
fi

${tablator_bin} --input-format=json5 --output-format=votable test/back_and_forth_tables/two_row_large_ulong_array.json5 - | diff -w - test/back_and_forth_tables/two_row_large_ulong_array_from_json5.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with large uint64 vals array col to VOTable"
else
    echo "FAIL: Convert Json5 Table with large uint64 vals array col to VOTable"
fi

# JTODO: DESCRIPTION section would not survive the round trip.
${tablator_bin} --input-format=json5 --output-format=votable test/back_and_forth_tables/integer_type_arrays.json5 - | diff -w - test/back_and_forth_tables/integer_type_arrays_from_json5.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted array cols to VOTable"
else
    echo "FAIL: Convert Json5 Table with assorted array cols to VOTable"
fi

# JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/one_row_int_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/one_row_int_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type int to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type int to FITS and back"
fi

# JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_int_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_int_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type int to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type int to FITS and back"
fi

# JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/one_row_uint_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/one_row_uint_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type uint to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type uint to FITS and back"
fi

# JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_uint_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_uint_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type uint to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type uint to FITS and back"
fi

${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_long_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_long_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type long to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type long to FITS and back"
fi

${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_ulong_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong to FITS and back"
fi

${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_large_ulong_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_large_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong with large values to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong with large values to FITS and back"
fi


${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.json5 temp.vot && ${tablator_bin}  temp.vot temp.json5 && diff -w  test/back_and_forth_tables/two_row_large_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong with large values to VOTABLE and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong with large values to VOTABLE and back"
fi

${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.json5 && ${tablator_bin}  temp.json5 temp.vot && diff -w  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with single two-row array col of type ulong with large values to Json5 and back"
    rm -f temp.json5
else
    echo "FAIL: Convert VOTable with single two-row array col of type ulong with large values to Json5 and back"
fi

${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.fits && ${tablator_bin}  temp.fits temp.vot && diff -w  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with single two-row array col of type ulong with large values to FITS and back"
    rm -f temp.fits
else
    echo "FAIL: Convert VOTable with single two-row array col of type ulong with large values to FITS and back"
fi


${tablator_bin} --output-format=fits  test/back_and_forth_tables/one_row_bool_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/one_row_bool_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type bool to FITS and back"
fi

${tablator_bin} --output-format=fits  test/back_and_forth_tables/two_row_bool_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/two_row_bool_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type bool to FITS and back"
fi

${tablator_bin} --output-format=fits  test/back_and_forth_tables/single_bool_col.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/single_bool_col.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single multi-row non-array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single multi-row non-array col of type bool to FITS and back"
fi

# JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/small_integer_type_arrays.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/small_integer_type_arrays.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted small-valued int-type array cols to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with assorted small-valued int-type array cols to FITS and back"
fi

# # JTODO: INFO section still does not survive the round trip.
${tablator_bin} --output-format=fits  test/back_and_forth_tables/small_fits_unsupported_integer_type_arrays.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w  test/back_and_forth_tables/small_fits_unsupported_integer_type_arrays_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted small-valued array cols of types not supported by FITS to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with assorted small-valued array cols of types not supported by FITS to FITS and back"
fi
