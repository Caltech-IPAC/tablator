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


###########################################################

for table in test/bad_ipac_tables/* test/bad_votables/*; do
    ${tablator_bin} $table bad_table.hdf5 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "FAIL: $table to hdf5"
    else
        echo "PASS: $table to hdf5"
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

${tablator_bin} --row-id=1 --column-to-extract chars2 --type char test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: extract_value() is not supported for columns of type char"
else
    echo "PASS: extract_value() is not supported for columns of type char"
    rm -f temp.txt
fi

${tablator_bin} --column-to-extract chars --type char test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: Extract_column() is not supported for columns of type char"
else
    echo "PASS: Extract_column() is not supported for columns of type char"
    rm -f temp.txt

fi


${tablator_bin} --static=1 --row-list="0 2 3 29 47" test/multi temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: invalid row_id"
else
    echo "PASS: invalid row_id"
    rm -f temp.tbl
fi

${tablator_bin}  test/back_and_forth_tables/bad_column_name1.vot temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write IPAC table with invalid column name correctly resulted in error"
else
    echo "PASS: attempt to write IPAC table with invalid column name correctly resulted in error"
    rm -f temp.tbl

fi

${tablator_bin}  test/back_and_forth_tables/bad_column_name2.vot temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write IPAC table with invalid column name correctly resulted in error"
else
    echo "PASS: attempt to write IPAC table with invalid column name correctly resulted in error"
    rm -f temp.tbl

fi

${tablator_bin}  test/back_and_forth_tables/bad_unit_name.xml temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write IPAC table with invalid unit name correctly resulted in error"
else
    echo "PASS: attempt to write IPAC table with invalid unit name correctly resulted in error"
    rm -f temp.tbl

fi

${tablator_bin} --column-names "" --exclude-cols 0  test/back_and_forth_tables/multi_row_0123_col_531.tbl temp.txt 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write IPAC subtable with no columns correctly resulted in error."
    rm -f temp.txt
else
    echo "PASS: attempt to write IPAC subtable with no columns correctly resulted in error."
fi

${tablator_bin} --column-names "object htm7 dec" --exclude-cols 1  test/back_and_forth_tables/multi_row_0123_col_531.tbl temp.txt 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write IPAC subtable with no columns via exclude-cols correctly resulted in error."
else
    echo "PASS: attempt to write IPAC subtable with no columns via exclude-cols correctly resulted in error."
    rm -f temp.txt
fi

${tablator_bin} build/tablator --output-format votable --column-names "ulong_int1" --exclude-cols 1 --idx-lookup 0 test/back_and_forth_tables/two_row_large_ulong_array.vot temp.vot 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to write subtable in votable format correctly resulted in error."
else
    echo "PASS: attempt to write subtable in votable format correctly resulted in error."
    rm -f temp.vot
fi

${tablator_bin}  test/back_and_forth_tables/no_results_resource.vot temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: attempt to read votable with no 'results' resource correctly resulted in error"
else
    echo "PASS: attempt to read votable with no 'results' resource correctly resulted in error"
    rm -f temp.tbl

fi

${tablator_bin}  --trim-decimal-runs=3:10 --row-count=3 --start-row 1 test/back_and_forth_tables/multi_untrimmed.tbl temp.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: invalid argument to trim-decimal-runs"
else
    echo "PASS: invalid argument to trim-decimal-runs"
    rm -f temp.tbl
fi


${tablator_bin} --combine-tables=1 test/multi test/int_types.vot temp.vot 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: combine tables whose column names are not distinct"

else
    echo "PASS: combine tables whose column names are not distinct"
    rm -f temp.tbl
    rm -f temp.vot
fi

${tablator_bin} --append-rows=1 test/back_and_forth_tables/two_row_float_array.vot test/back_and_forth_tables/two_row_double_array.vot temp.json5 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: combine tables whose corresponding columns are of different types"
else
    echo "PASS: combine tables whose corresponding columns are of different types"
    rm -f temp.json5
fi

${tablator_bin} --append-rows=1 test/back_and_forth_tables/two_row_double_array.vot test/back_and_forth_tables/two_row_double_array_II.vot out.tbl 2> /dev/null
if [ $? -eq 0 ]; then
    echo "FAIL: combine tables whose corresponding offsets don't match"
else
    echo "PASS: combine tables whose corresponding offsets don't match"
    rm -f temp.json5
fi


###########################################################

STREAM_INTERMEDIATE=""
for table in test/multi test/dos_ending.csv test/multi.csv test/multi.tsv test/fits_medium.fits test/*.tbl test/*.json5 test/*.xml test/upload_table.vo test/*.unk; do
    STREAM_INTERMEDIATE=""
    if [ $table != "test/fits_medium.fits" ] && [ $table != "test/multi_h5.unk" ] && [ $table != "test/multi_tsv.unk" ]; then
        STREAM_INTERMEDIATE="--stream-intermediate=yes"
    fi
    for ending in tbl hdf5 xml csv tsv fits html json json5 postgres sqlite oracle db; do
        # echo "table: $table, ending: $ending"
        if [ $ending == "db" ]; then
            ${tablator_bin} $table test.$ending
        else
            ${tablator_bin} $STREAM_INTERMEDIATE $table test.$ending
        fi

        if [ $? -eq 0 ]; then
            echo "PASS: $table -> $ending I"
        else
            echo "FAIL: $table -> $ending I"
        fi

        # optionally convert result to IPAC_TABLE format
        if [ $ending == "hdf5" ] || [ $ending == "fits" ]; then
            ${tablator_bin} test.$ending temp.tbl
        elif [ $ending == "xml" ] || [ $ending == "json" ] || [ $ending == "tbl" ]; then
            ${tablator_bin} $STREAM_INTERMEDIATE test.$ending temp.tbl
        fi

        let output=$?
        if [[ output -eq 0 ]]; then
            echo "PASS: $table -> $ending II (-> tbl) "
        else
            echo "FAIL: $table -> $ending II (-> tbl) : $output"
        fi

        rm -f test.$ending temp.tbl
    done
done

###########################################################
# Test clobbering when converting to .db.
touch temp.db
${tablator_bin} test/int_types.json5 temp.db
if [ $? -eq 0 ]; then
    echo "PASS: clobbered existing temp.db"
    rm -f temp.db
else
    echo "FAIL: clobbered existing temp.db"
fi

###########################################################
# Test one-way conversions


${tablator_bin} --input-format=hdf5 --output-format=ipac_table test/h5_as_csv.csv temp.h5
${tablator_bin} --input-format=ipac_table temp.h5 temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Explicit format specified"
    rm -f temp.h5 temp.tbl
else
    echo "FAIL: Explicit format specified"
fi


${tablator_bin} --output-format=votable test/recursive_param.xml temp.vot && diff test/recursive_param.xml temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: recursive param tabledata"
    rm -f temp.vot
else
    echo "FAIL: recursive param tabledata"
fi

${tablator_bin} --output-format=votable test/recursive_param_binary2.xml temp.vot && diff test/recursive_param.xml temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: recursive param binary2"
    rm -f temp.vot
else
    echo "FAIL: recursive param binary2"
fi

${tablator_bin} --output-format=csv test/al.csv temp.csv && diff -w test/al.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: CSV implicit float"
    rm -f temp.csv
else
    echo "FAIL: CSV implicit float"
fi

${tablator_bin} --output-format=html test/multi.tbl temp.html && diff test/back_and_forth_tables/multi.html temp.html
if [ $? -eq 0 ]; then
    echo "PASS: HTML retain links"
    rm -f temp.html
else
    echo "FAIL: HTML retain links"
fi

${tablator_bin} --output-format=postgres test/multi.tbl - | diff -w test/back_and_forth_tables/multi.postgres -
if [ $? -eq 0 ]; then
    echo "PASS: Postgres output"
else
    echo "FAIL: Postgres output"
fi

${tablator_bin} --output-format=oracle test/multi.tbl - | diff -w test/back_and_forth_tables/multi.oracle -
if [ $? -eq 0 ]; then
    echo "PASS: Oracle output"
else
    echo "FAIL: Oracle output"
fi

${tablator_bin} --output-format=sqlite test/multi.tbl - | diff -w test/back_and_forth_tables/multi.sqlite -
if [ $? -eq 0 ]; then
    echo "PASS: SQLite output"
else
    echo "FAIL: SQLite output"
fi

${tablator_bin} --output-format=csv test/multi.tbl - | diff -w test/back_and_forth_tables/multi_unquoted.csv -
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

cat test/multi.csv | ${tablator_bin} --input-format=csv --output-format=csv - - | diff -w test/multi.csv -
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

${tablator_bin} --input-format=ipac_table --output-format=votable test/int_types.tbl temp.vot && diff -w test/int_types.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with large uint64 vals to VOTable"
else
    echo "FAIL: Convert IPAC Table with large uint64 vals to VOTable"
fi

${tablator_bin} --input-format=ipac_table --output-format=json5 test/int_types.tbl temp.json5 && diff -w test/int_types.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with large uint64 vals to JSON5"
else
    echo "FAIL: Convert IPAC Table with large uint64 vals to JSON5"
fi

${tablator_bin} --input-format=json5 --output-format=votable test/back_and_forth_tables/two_row_large_ulong_array.json5 temp.vot && diff -w test/back_and_forth_tables/two_row_large_ulong_array_from_json5.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with large uint64 vals array col to VOTable"
else
    echo "FAIL: Convert Json5 Table with large uint64 vals array col to VOTable"
fi

# JTODO: datatypes do not survive the round trip via votable.
${tablator_bin} --input-format=json5 --output-format=votable test/back_and_forth_tables/integer_type_arrays.json5 temp.vot && diff -w test/back_and_forth_tables/integer_type_arrays_from_json5.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted array cols to VOTable"
else
    echo "FAIL: Convert Json5 Table with assorted array cols to VOTable"
fi

${tablator_bin} --output-format=ipac_table test/back_and_forth_tables/two_row_array_with_nulls.vot temp.tbl && diff -w test/back_and_forth_tables/two_row_array_with_nulls_from_vot.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with null elements to IPAC Table"
else
    echo "FAIL: Convert VOTable with null elements to IPAC Table"
fi

#################################################################
# Test one-way conversions
#################################################################

${tablator_bin}  test/fits_medium.fits temp.vot && diff -w test/back_and_forth_tables/fits_medium.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert FITS file to VOTable"
else
    echo "FAIL: Convert FITS file to VOTable"
fi


${tablator_bin} test/int_types_with_duplicate_keys.json5 out.json5 && diff -w test/int_types.json5 out.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert duplicate keys in JSON5 format to array"
    rm -f out.json5
else
    echo "FAIL: Convert duplicate keys in JSON5 format to array"
fi


###########################################################
# Test round-trip conversions
###########################################################

${tablator_bin} --output-format=fits test/back_and_forth_tables/one_row_int_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/one_row_int_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type int to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type int to FITS and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_int_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_int_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type int to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type int to FITS and back"
fi

# JTODO: INFO section still does not survive the round trip via IPAC_TABLE.
${tablator_bin} test/back_and_forth_tables/two_row_int_array.json5 temp.tbl && ${tablator_bin} --input-format=ipac_table temp.tbl temp.json5 && diff -w test/back_and_forth_tables/two_row_int_array_via_ipac.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type int to IPAC Table and back"
    rm -f temp.json5
    rm -f temp.tbl
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type int to IPAC Table and back"
fi


${tablator_bin} --output-format=votable test/back_and_forth_tables/two_row_int_array.json5 temp.vot && ${tablator_bin} temp.vot temp.json5 && diff -w test/back_and_forth_tables/two_row_int_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type int to VOTABLE and back"
    rm -f temp.json5
    rm -f temp.vot
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type int to VOTABLE and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/one_row_uint_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/one_row_uint_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type uint to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type uint to FITS and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_uint_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_uint_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type uint to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type uint to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_long_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_long_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type long to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type long to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_ulong_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_large_ulong_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_large_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong with large values to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong with large values to FITS and back"
fi


${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.json5 temp.vot &&
${tablator_bin}  temp.vot temp.json5 && diff -w test/back_and_forth_tables/two_row_large_ulong_array_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type ulong with large values to VOTABLE and back"
    rm -f temp.json5
    rm -f temp.vot
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type ulong with large values to VOTABLE and back"
fi

${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.json5 &&
${tablator_bin}  temp.json5 temp.vot && diff -w test/back_and_forth_tables/two_row_large_ulong_array.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with single two-row array col of type ulong with large values to Json5 and back"
    rm -f temp.json5
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable with single two-row array col of type ulong with large values to Json5 and back"
fi

${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array.vot temp.fits &&
${tablator_bin}  temp.fits temp.vot && diff -w test/back_and_forth_tables/two_row_large_ulong_array.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with single two-row array col of type ulong with large values to FITS and back"
    rm -f temp.fits
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable with single two-row array col of type ulong with large values to FITS and back"
fi

${tablator_bin}  test/back_and_forth_tables/two_row_large_ulong_array_with_type.vot temp.fits &&
${tablator_bin}  temp.fits temp.vot && diff -w test/back_and_forth_tables/two_row_large_ulong_array_with_type.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with resource of type results and single two-row array col of type ulong with large values to FITS and back"
    rm -f temp.fits
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable with resource of type results and single two-row array col of type ulong with large values to FITS and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/one_row_bool_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/one_row_bool_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single one-row array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single one-row array col of type bool to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_bool_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_bool_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type bool to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/two_row_byte_array.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/two_row_byte_array.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type unsignedByte to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type unsignedByte to FITS and back"
fi

${tablator_bin} --output-format=ipac_table test/back_and_forth_tables/two_row_byte_array.json5 temp.tbl && ${tablator_bin} --input-format=ipac_table temp.tbl temp.json5 && diff -w test/back_and_forth_tables/two_row_byte_array_via_ipac.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single two-row array col of type unsignedByte to IPAC Table and back"
    rm -f temp.json5
    rm -f temp.tbl
else
    echo "FAIL: Convert Json5 Table with single two-row array col of type unsignedByte to IPAC Table and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/single_bool_col.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/single_bool_col.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with single multi-row non-array col of type bool to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with single multi-row non-array col of type bool to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/fits_medium_modified.vot out.fits &&  ${tablator_bin}  out.fits temp.vot && diff -w test/back_and_forth_tables/fits_medium_modified.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable translated from FITS to FITS and back"
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable translated from FITS to FITS and back"
fi

${tablator_bin} --output-format=fits test/back_and_forth_tables/fits_medium_modified_with_value.vot temp.fits &&  ${tablator_bin}  temp.fits temp.vot && diff -w test/back_and_forth_tables/fits_medium_modified_with_value.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with unorthodox INFO property value to FITS and back"
    rm -f temp.vot
    rm -f temp.fits
else
    echo "FAIL: Convert VOTable with unorthodox INFO property value to FITS and back"
fi

${tablator_bin} --output-format=json test/back_and_forth_tables/fits_medium_modified_with_value.vot temp.json &&  ${tablator_bin}  temp.json temp.vot && diff -w test/back_and_forth_tables/fits_medium_modified_with_value.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with unorthodox INFO property value to JSON and back"
    rm -f temp.vot
    rm -f temp.json
else
    echo "FAIL: Convert VOTable with unorthodox INFO property value to JSON and back"
fi


# JTODO: Field-level INFO section would still not survive the round trip via fits.
${tablator_bin} --output-format=fits test/back_and_forth_tables/small_integer_type_arrays.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/small_integer_type_arrays.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted small-valued int-type array cols to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with assorted small-valued int-type array cols to FITS and back"
fi



# JTODO: descriptions and field-level attrs still do not survive the round trip via FITS.
${tablator_bin} --output-format=fits test/back_and_forth_tables/desc_attrs_no_info.xml - | ${tablator_bin} --input-format=fits - temp.vot && diff -w test/back_and_forth_tables/desc_attrs_no_info_via_fits.xml temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with elements FITS can't handle to FITS and back"
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable with elements FITS can't handle to FITS and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/small_fits_unsupported_integer_type_arrays.json5 - | ${tablator_bin} --input-format=fits - temp.json5 && diff -w test/back_and_forth_tables/small_fits_unsupported_integer_type_arrays_via_fits.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with assorted small-valued array cols of types not supported by FITS to FITS and back"
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with assorted small-valued array cols of types not supported by FITS to FITS and back"
fi


${tablator_bin} --output-format=fits test/back_and_forth_tables/partial_euclid.vot - | ${tablator_bin} --input-format=fits - temp.vot && diff -w test/back_and_forth_tables/partial_euclid.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with array col of type supported by FITS to FITS and back"
    rm -f temp.vot
else
    echo "FAIL: Convert VOTable with array col of type supported by FITS to FITS and back"
fi


${tablator_bin} test/back_and_forth_tables/partial_euclid.fits temp.vot && ${tablator_bin} temp.vot temp.fits && diff -w test/back_and_forth_tables/partial_euclid.fits temp.fits
if [ $? -eq 0 ]; then
    echo "PASS: Convert FITS file with array col to VOTable and back"
    rm -f temp.vot
    rm -f temp.fits
else
    echo "FAIL: Convert FITS file with array col to VOTable and back"
fi

# TODO: comments don't survive round trip via IPAC format.
${tablator_bin} test/back_and_forth_tables/partial_euclid.fits temp.tbl && diff -w test/back_and_forth_tables/partial_euclid.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert FITS file with array col to IPAC Table"
    rm -f temp.tbl
else
    echo "FAIL: Convert FITS file with array col to IPAC Table"
fi



${tablator_bin} test/back_and_forth_tables/ipac_table_with_malformed_comment.tbl temp.json5 &&
${tablator_bin} temp.json5 temp.tbl && diff -w test/back_and_forth_tables/ipac_table_with_added_comments.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with malformed comment to Json5 and back"
    rm -f temp.json5
    rm -f temp.tbl
else
    echo "FAIL: Convert IPAC Table with malformed comment to Json5 and back"
fi

${tablator_bin} --output-format=json5 test/back_and_forth_tables/ipac_table_with_added_comments.tbl temp.out &&
${tablator_bin} --input-format=json5 temp.out temp.tbl && diff -w test/back_and_forth_tables/ipac_table_with_added_comments.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with comments to Json5 and back"
    rm -f temp.out
    rm -f temp.tbl
else
    echo "FAIL: Convert IPAC Table with comments to Json5 and back"
fi

${tablator_bin} --output-format=ipac_table test/back_and_forth_tables/json5_table_with_long_comment.json5 temp.out &&
${tablator_bin} --input-format=ipac_table temp.out temp.json5 && diff -w test/back_and_forth_tables/json5_table_with_long_comment.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert Json5 Table with long comment to IPAC Table and back"
    rm -f temp.out
    rm -f temp.json5
else
    echo "FAIL: Convert Json5 Table with long comment to IPAC Table and back"
fi

${tablator_bin} --output-format=json5 test/back_and_forth_tables/ipac_table_with_long_comment.tbl temp.out &&
${tablator_bin} --input-format=json5 temp.out temp.tbl && diff -w test/back_and_forth_tables/ipac_table_with_long_comment.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with long comment to Json5 and back"
    rm -f temp.out
    rm -f temp.tbl
else
    echo "FAIL: Convert IPAC Table with long comment to Json5 and back"
fi

${tablator_bin} --output-format=votable test/back_and_forth_tables/ipac_table_with_newlines.tbl temp.out &&
${tablator_bin} --input-format=votable temp.out temp.tbl && diff -w test/back_and_forth_tables/ipac_table_with_newlines.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with newlines to VOTable and back"
    rm -f temp.out
    rm -f temp.json5
else
    echo "FAIL: Convert IPAC Table with newlines to VOTable and back"
fi

${tablator_bin} --output-format=ipac_table test/back_and_forth_tables/col_descriptions_with_newlines.xml temp.out &&
${tablator_bin} --input-format=ipac_table temp.out temp.vot && diff -w test/back_and_forth_tables/col_descriptions_with_newlines.round_trip.xml temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with newlines in column descriptions to IPAC Table and back"
    rm -f temp.vot
    rm -f temp.tbl
else
    echo "FAIL: Convert VOTable with newlines in column descriptions to IPAC Table and back"
fi


${tablator_bin} --output-format=votable test/back_and_forth_tables/ipac_table_with_spaces.tbl temp.out &&
${tablator_bin} --input-format=votable temp.out temp.tbl && diff -w test/back_and_forth_tables/ipac_table_with_spaces.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Convert IPAC Table with spaces to VOTable and back"
    rm -f temp.out
    rm -f temp.json5
else
    echo "FAIL: Convert IPAC Table with spaces to VOTable and back"
fi

${tablator_bin} test/back_and_forth_tables/info_at_all_levels.xml temp.hdf5 &&
${tablator_bin} temp.hdf5 temp.xml && diff test/back_and_forth_tables/info_at_all_levels.xml temp.xml
if [ $? -eq 0 ]; then
    echo "PASS: Convert VOTable with info at all levels to hdf5 and back"
    rm -f temp.hdf5
    rm -f temp.xml
else
    echo "FAIL: Convert VOTable with info at all levels to hdf5 and back"
fi


${tablator_bin} test/back_and_forth_tables/multiple_group_example.vot temp.json5 &&
${tablator_bin} temp.json5 out.vot && diff test/back_and_forth_tables/multiple_group_example.vot out.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with multiple group metadata from VOTable to JSON5 and back"
    rm -f temp_json5
    rm -f out.vot
else
    echo "FAIL: Convert table with multiple group metadata from VOTable to JSON5 and back"
fi

${tablator_bin} test/back_and_forth_tables/multiple_resource_results_last.vot temp.json5 &&
${tablator_bin} temp.json5 out.vot && diff test/back_and_forth_tables/multiple_resource_results_last.vot out.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with multiple resource elements from VOTable to JSON5 and back"
    rm -f temp_json5
    rm -f out.vot
else
    echo "FAIL: Convert table with multiple resource elements from VOTable to JSON5 and back"
fi

${tablator_bin} test/back_and_forth_tables/multiple_resource_results_middle.vot temp.json5 &&
${tablator_bin} temp.json5 out.vot && diff test/back_and_forth_tables/multiple_resource_results_middle.vot out.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with resource element in the middle from VOTable to JSON5 and back"
    rm -f temp_json5
    rm -f out.vot
else
    echo "FAIL: Convert table with resource element in the middle from VOTable to JSON5 and back"
fi


${tablator_bin} test/back_and_forth_tables/meta_resource_with_options.vot temp.json5 &&
${tablator_bin} temp.json5 temp.vot &&
diff test/back_and_forth_tables/meta_resource_with_options.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with multiple options from VOTable to JSON5 and back"
    rm -f temp.json5
    rm -f temp.vot
else
    echo "FAIL: Convert table with multiple options from VOTable to JSON5 and back"
fi

${tablator_bin} test/back_and_forth_tables/meta_resource_with_options.json5 temp.vot &&
${tablator_bin} temp.vot temp.json5 &&
diff test/back_and_forth_tables/meta_resource_with_options.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with multiple options from JSON5 to VOTable and back"
    rm -f temp.json5
    rm -f temp.vot
else
    echo "FAIL: Convert table with multiple options from JSON5 to VOTable and back"
fi

${tablator_bin} test/back_and_forth_tables/multi_with_hex_and_comments.csv temp.tbl &&
${tablator_bin} temp.tbl temp.csv &&
diff test/back_and_forth_tables/multi_with_comments.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with hex and comments from csv to IPAC table and back"
    rm -f temp.tbl
    rm -f temp.csv
else
    echo "FAIL: Convert table with hex and comments from csv to IPAC table and back"
fi

${tablator_bin} test/back_and_forth_tables/multi_with_comments.csv temp.tbl &&
${tablator_bin} temp.tbl temp.csv &&
diff test/back_and_forth_tables/multi_with_comments.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with comments from csv to IPAC table and back"
    rm -f temp.tbl
    rm -f temp.csv
else
    echo "FAIL: Convert table with comments from csv to IPAC table and back"
fi


${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.csv temp.tbl &&
${tablator_bin} --write-null-string temp.tbl temp.csv &&
diff test/back_and_forth_tables/various_with_null_strings_rounded.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from csv to IPAC table and back"
    rm -f temp.tbl
    rm -f temp.csv
else
    echo "FAIL: Convert table with null strings from csv to IPAC table and back"
fi

${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.tsv temp.tbl &&
${tablator_bin} --write-null-string temp.tbl temp.tsv &&
diff test/back_and_forth_tables/various_with_null_strings_rounded.tsv temp.tsv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from tsv to IPAC table and back"
    rm -f temp.tbl
    rm -f temp.tsv
else
    echo "FAIL: Convert table with null strings from tsv to IPAC table and back"
fi


${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.csv temp.tsv &&
${tablator_bin} --write-null-string temp.tsv temp.csv &&
diff test/back_and_forth_tables/various_with_null_strings_and_hex_rounded.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from csv to tsv and back, preserving null strings"
    rm -f temp.tsv
    rm -f temp.csv
else
    echo "FAIL: Convert table with null strings from csv to tsv and back, preserving null strings"
fi

${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.tsv temp.csv &&
${tablator_bin} --write-null-string temp.csv temp.tsv &&
diff test/back_and_forth_tables/various_with_null_strings_and_hex_rounded.tsv temp.tsv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from tsv to csv and back, preserving null strings"
    rm -f temp.tsv
    rm -f temp.csv
else
    echo "FAIL: Convert table with null strings from tsv to csv and back, preserving null strings"
fi


${tablator_bin} test/back_and_forth_tables/coosys_example.vot temp.fits &&
${tablator_bin} temp.fits temp.vot &&
diff test/back_and_forth_tables/coosys_example.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with coosys and timesys from vot to FITS and back"
    rm -f temp.vot temp.fits
else
    echo "FAIL: Convert table with coosys and timesys from vot to FITS and back"
fi



#################################################################
# not straight conversions
#################################################################

${tablator_bin} --static=1 --row-list="0 2 3" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_023.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with selected rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with selected rows in IPAC Table format"
fi

${tablator_bin} --skip-comments 1 --row-list="0 2 3" test/multi temp.tbl && diff test/multi_row_023_no_headers.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with selected rows and no headers in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with selected rows and no headers in IPAC Table format"
fi

${tablator_bin} --static=1 --skip-comments 1 --row-list="0 2 3" test/multi temp.tbl && diff test/multi_row_023_no_headers.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with static=1 and selected rows and no headers in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with static=1 and selected rows and no headers in IPAC Table format"
fi


${tablator_bin} --static=1 --row-list="0 2 1 2 0" test/back_and_forth_tables/multiple_row.json5 temp.tbl && diff test/back_and_forth_tables/multiple_row_02120.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with repeated selected rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with repeated selected rows in IPAC Table format"
fi

${tablator_bin}  --row-count=3 --start-row 1 test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_123.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with consecutive rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with consecutive rows in IPAC Table format"
fi

${tablator_bin}  --row-count=30 --start-row 1 test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_123.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with reduced number of consecutive rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with reduced number of consecutive rows in IPAC Table format"
fi


${tablator_bin}  --row-count=3 --start-row 1 --column-ids "1 3 5" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_123_col_135.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with consecutive rows and alternate columns in IPAC Table format"
    rm -f temp_file
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with consecutive rows and alternate columns in IPAC Table format"
fi

${tablator_bin}  --row-count=30 --start-row 1 --column-ids "0 1 3 5" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_123_col_135.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with selected columns and reduced number of consecutive rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with selected columns and reduced number of consecutive rows in IPAC Table format"
fi

${tablator_bin}  --row-count=30 --start-row 1 --column-ids "0 5 3 0 27 1 0" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_123_col_531.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with selected columns out of order and reduced number of consecutive rows in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with selected columns out of order and reduced number of consecutive rows in IPAC Table format"
fi

${tablator_bin}   --column-ids "0 5 3 0 27 1 0" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_0123_col_531.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with selected columns out of order and no row restriction in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with selected columns out of order and no row restriction in IPAC Table format"
fi


${tablator_bin} --skip-comments 0  --column-names "htm7 dec object" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_0123_col_531.tbl temp.tbl

if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with named columns out of order and no row restriction in IPAC Table format"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with named columns out of order and no row restriction in IPAC Table format"
fi

${tablator_bin} --idx-lookup 0  --column-names "htm7 dec object" test/multi temp.tbl && diff test/back_and_forth_tables/multi_row_0123_col_531.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with named columns out of order and no row restriction in IPAC Table format and explicit !ids_only"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with named columns out of order and no row restriction in IPAC Table format and explicit !ids_only"
fi

# skip-comments

${tablator_bin}  --skip-comments 1  --column-names "Index ra dec" --exclude-cols 0 test/back_and_forth_tables/partial_auriga.tbl temp.tbl && diff -w test/back_and_forth_tables/partial_auriga_no_flux.tbl temp.tbl

if [ $? -eq 0 ]; then
    echo "PASS: Extract columns, skipping headers"
    rm -f temp.tbl
else
    echo "FAIL: Extract columns, skipping headers"
fi


# extract single value
${tablator_bin}  --row-id=1 --column-to-extract big_uint64s --type UINT64_LE test/back_and_forth_tables/integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint64_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint64 value"
    rm -f temp.txt
else
    echo "FAIL: Extract uint64 value"
fi

${tablator_bin}  --row-id=1 --column-to-extract small_int32s --type INT32_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_int32_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract int32 value"
    rm -f temp.txt
else
    echo "FAIL: Extract int32 value"
fi

${tablator_bin}  --row-id=0 --column-to-extract small_uint32s --type UINT32_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint32_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint32 value"
    rm -f temp.txt
else
    echo "FAIL: Extract uint32 value"
fi

${tablator_bin} --row-id=1 --column-to-extract uints --type UINT32_LE test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt && diff test/back_and_forth_tables/extracted_null_long_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract null long value"
    rm -f temp.txt
else
    echo "FAIL: Extract null long value"
fi



${tablator_bin}  --row-id=1 --column-to-extract ubools --type UINT8_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_ubool_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint8 value"
    rm -f temp.txt
else
    echo "FAIL: Extract uint8 value"
fi

${tablator_bin}  --row-id=3 --column-to-extract ra --type FLOAT64_LE test/multi temp.txt && diff test/double_from_multi.txt temp.txt && diff test/back_and_forth_tables/extracted_double_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract double value"
    rm -f temp.txt
else
    echo "FAIL: Extract double value"
fi


# extract single value as string
${tablator_bin}  --row-id=1 --column-to-extract big_uint64s --as-string 1 test/back_and_forth_tables/integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint64_val_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint64 value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract uint64 value as string"
fi

${tablator_bin}  --row-id=1 --column-to-extract small_int32s --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_int32_val_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract int32 value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract int32 value as string"
fi

${tablator_bin}  --row-id=0 --column-to-extract small_uint32s --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint32_val_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint32 value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract uint32 value as string"
fi

${tablator_bin}  --row-id=1 --column-to-extract ubools --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_ubool_val_as_string.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint8 value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract uint8 value as string"
fi

${tablator_bin}  --row-id=3 --column-to-extract ra --as-string 1  test/multi temp.txt && diff test/back_and_forth_tables/extracted_double_val_precision.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract double value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract double value as string"
fi

${tablator_bin} --row-id=1 --column-to-extract chars2 --as-string 1 test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt && diff test/back_and_forth_tables/extracted_char_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract char value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract char value as string"
fi

${tablator_bin} --row-id=1 --column-to-extract chars --as-string 1 test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt && diff test/back_and_forth_tables/extracted_null_char_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract null char value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract null char value as string"
fi

${tablator_bin} --row-id=1 --column-to-extract uints --as-string 1 test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt && diff test/back_and_forth_tables/extracted_null_char_val.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract null long value as string"
    rm -f temp.txt
else
    echo "FAIL: Extract null long value as string"
fi



# extract column
${tablator_bin} --column-to-extract big_uint64s --type UINT64_LE test/back_and_forth_tables/integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint64_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint64 column"
    rm -f temp.txt
else
    echo "FAIL: Extract uint64 column"
fi

${tablator_bin}  --column-to-extract small_int32s --type INT32_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_int32_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract int32 column"
    rm -f temp.txt
else
    echo "FAIL: Extract int32 column"
fi

${tablator_bin}  --column-to-extract small_uint32s --type UINT32_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint32_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint32 column"
    rm -f temp.txt
else
    echo "FAIL: Extract uint32 column"
fi

${tablator_bin}  --column-to-extract ubools --type UINT8_LE test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_ubool_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint8 column"
    rm -f temp.txt
else
    echo "FAIL: Extract uint8 column"
fi

${tablator_bin}  --column-to-extract ra --type FLOAT64_LE test/multi temp.txt && diff test/back_and_forth_tables/extracted_double_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract double column"
    rm -f temp.txt
else
    echo "FAIL: Extract double column"
fi


#extract column as string
${tablator_bin} --column-to-extract shtm3 --as-string 1 test/multi temp_file && diff test/multi_shtm3_col temp_file
if [ $? -eq 0 ]; then
    echo "PASS: Extract int16 column as strings"
    rm -f temp_file
else
    echo "FAIL: Extract int16 column as strings"
fi


${tablator_bin} --column-to-extract big_uint64s --as-string 1 test/back_and_forth_tables/integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint64_col_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint64 column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract uint64 column as strings"
fi

${tablator_bin} --column-to-extract chars --as-string 1 test/back_and_forth_tables/two_row_array_with_nulls.vot temp.txt && diff test/back_and_forth_tables/extracted_char_col.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract char column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract char column as strings"
fi

${tablator_bin}  --column-to-extract small_int32s --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_int32_col_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract int32 column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract int32 column as strings"
fi

${tablator_bin}  --column-to-extract small_uint32s --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_uint32_col_str.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint32 column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract uint32 column as strings"
fi

${tablator_bin}  --column-to-extract ubools --as-string 1 test/back_and_forth_tables/small_integer_type_arrays.json5 temp.txt && diff test/back_and_forth_tables/extracted_ubool_col_hex.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract uint8 column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract uint8 column as strings"
fi

${tablator_bin}  --column-to-extract ra --as-string 1 test/multi temp.txt && diff test/back_and_forth_tables/extracted_double_col_precision.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Extract double column as strings"
    rm -f temp.txt
else
    echo "FAIL: Extract double column as strings"
fi

${tablator_bin} --column-names "ra dec SSO" --idx-lookup 1 test/multi temp.txt && diff test/back_and_forth_tables/multi_include_col_ids.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Lookup column names for inclusion"
    rm -f temp.txt
else
    echo "FAIL: Lookup column names for inclusion"
fi

${tablator_bin} --column-names "ra dec SSO" --exclude-cols 1 --idx-lookup 1 test/multi temp.txt && diff test/back_and_forth_tables/multi_exclude_col_ids.txt temp.txt
if [ $? -eq 0 ]; then
    echo "PASS: Lookup column names for exclusion"
    rm -f temp.txt
else
    echo "FAIL: Lookup column names for exclusion"
fi

${tablator_bin} --column-names "uints_10 uints_11"  test/back_and_forth_tables/expanded_array_cols.tbl temp.tbl && diff test/back_and_forth_tables/selected_array_cols.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: column headers for selected columns"
    rm -f temp.txt
else
    echo "FAIL: column headers for selected columns"
fi


${tablator_bin} test/back_and_forth_tables/group_example.vot temp.vot && diff test/back_and_forth_tables/group_example.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with group metadata"
    rm -f temp.vot
else
    echo "FAIL: Table with group metadata"
fi

${tablator_bin} test/back_and_forth_tables/coosys_example.vot temp.vot && diff test/back_and_forth_tables/coosys_example.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with coosys and timesys"
    rm -f temp.vot
else
    echo "FAIL: Table with coosys and timesys"
fi

${tablator_bin} test/back_and_forth_tables/multiple_group_example.vot temp.vot && diff test/back_and_forth_tables/multiple_group_example.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with multiple group metadata"
    rm -f temp.vot
else
    echo "FAIL: Table with  multiple group metadata"
fi




${tablator_bin} test/back_and_forth_tables/multiple_resource_results_first.vot temp.vot && diff test/back_and_forth_tables/multiple_resource_results_first_rearranged.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with results resource first"
    rm -f temp.vot
else
    echo "FAIL: Table with results resource first"
fi

${tablator_bin} test/back_and_forth_tables/multiple_resource_results_last.vot temp.vot && diff test/back_and_forth_tables/multiple_resource_results_last.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with results resource last"
    rm -f temp.vot
else
    echo "FAIL: Table with results resource last"
fi

${tablator_bin} test/back_and_forth_tables/multiple_resource_results_last.vot temp.json5 && diff test/back_and_forth_tables/multiple_resource_results_last.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with multiple resources to JSON5"
    rm -f temp.json5
else
    echo "FAIL: Convert table with multiple resources to JSON5"
fi


${tablator_bin} test/back_and_forth_tables/multiple_info_example.vot temp.vot && diff test/back_and_forth_tables/multiple_info_example_rearranged.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with multiple infos"
    rm -f temp.vot
else
    echo "FAIL: Table with multiple infos"
fi

${tablator_bin} test/back_and_forth_tables/multiple_info_example_rearranged.vot temp.vot && diff test/back_and_forth_tables/multiple_info_example_rearranged.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: Table with multiple infos already in order"
    rm -f temp_file
else
    echo "FAIL: Table with multiple infos already in order"
fi



${tablator_bin} test/back_and_forth_tables/meta_resource_with_options.vot temp.vot && diff test/back_and_forth_tables/meta_resource_with_options.vot temp.vot
if [ $? -eq 0 ]; then
    echo "PASS: VOTable with multiple options"
    rm -f temp.vot
else
    echo "FAIL: VOTable with multiple options"
fi

${tablator_bin} test/back_and_forth_tables/meta_resource_with_options.json5 temp.json5 && diff test/back_and_forth_tables/meta_resource_with_options.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: Json5 Table with multiple options"
    rm -f temp.json5
else
    echo "FAIL: Json5 Table with multiple options"
fi

${tablator_bin} test/back_and_forth_tables/multi_with_hex_and_comments.csv temp.csv&& diff test/back_and_forth_tables/multi_with_hex_and_comments.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: csv table with hex and comments"
    rm -f temp.csv
else
    echo "FAIL: csv table with hex and comments"
fi

${tablator_bin} test/back_and_forth_tables/multi_with_hex_and_comments_and_blank_line.csv temp.csv&& diff test/back_and_forth_tables/multi_with_hex_and_comments.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: csv table with hex, comments, and blank line"
    rm -f temp.csve
else
    echo "FAIL: csv table with hex, comments, and blank line"
fi

${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.csv temp.tbl &&
${tablator_bin} temp.tbl temp.csv &&
diff test/back_and_forth_tables/various_with_nulls.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from csv to IPAC table and back, losing null strings"
    rm -f temp.csv
else
    echo "FAIL: Convert table with null strings from csv to IPAC table and back, losing null strings"
fi

${tablator_bin} test/back_and_forth_tables/various_with_null_strings_and_hex.tsv temp.tbl &&
${tablator_bin} temp.tbl temp.tsv &&
diff test/back_and_forth_tables/various_with_nulls.tsv temp.tsv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with null strings from tsv to IPAC table and back, losing null strings"
    rm -f temp.tbl
    rm -f temp.tsv
else
    echo "FAIL: Convert table with null strings from tsv to IPAC table and back, losing null strings"
fi

${tablator_bin} test/back_and_forth_tables/various_with_quotes.csv temp.tbl &&
${tablator_bin} temp.tbl temp.csv &&
diff test/back_and_forth_tables/various_with_quotes.csv temp.csv
if [ $? -eq 0 ]; then
    echo "PASS: Convert table with single and double quotes from csv to IPAC table and back"
    rm -f temp.tbl
    rm -f temp.csv

else
    echo "FAIL: Convert table with single and double quotes from csv to IPAC table and back"
fi


${tablator_bin}  --trim-decimal-runs=0 --row-count=3 --start-row 1 test/back_and_forth_tables/multi_untrimmed.tbl temp.tbl && diff test/back_and_forth_tables/multi_row_123_untrimmed.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with consecutive rows in IPAC Table format, no trimming"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with consecutive rows in IPAC Table format, no trimming"
fi

${tablator_bin}  --trim-decimal-runs=1 --row-count=3 --start-row 1 test/back_and_forth_tables/multi_untrimmed.tbl temp.tbl && diff test/back_and_forth_tables/multi_row_123.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with consecutive rows in IPAC Table format, default trimming"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with consecutive rows in IPAC Table format, default trimming"
fi

${tablator_bin}  --trim-decimal-runs=1:10 --row-count=3 --start-row 1 test/back_and_forth_tables/multi_untrimmed.tbl temp.tbl && diff test/back_and_forth_tables/multi_row_123_untrimmed.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: Write subtable with consecutive rows in IPAC Table format, trim with N=10"
    rm -f temp.tbl
else
    echo "FAIL: Write subtable with consecutive rows in IPAC Table format, trim with N=10"
fi

#################################################################
# construct new tables
#################################################################

${tablator_bin} --counter-column=my_cntr test/back_and_forth_tables/two_row_array_with_nulls.vot temp.tbl && diff test/back_and_forth_tables/two_row_array_with_nulls_and_cntr.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: add counter column to vot and write as tbl"
    rm -f temp.tbl
else
    echo "FAIL: add counter column to vot and write as tbl"
fi

${tablator_bin} --combine-tables=1 test/back_and_forth_tables/two_row_array_with_nulls.vot test/back_and_forth_tables/two_row_large_ulong_array_from_json5.vot temp.tbl && diff test/back_and_forth_tables/combined_two_row_table.tbl temp.tbl
if [ $? -eq 0 ]; then
    echo "PASS: combine vot tables and write as tbl"
    rm -f temp.tbl
else
    echo "PASS: combine vot tables and write as tbl"
fi


${tablator_bin} --append-rows=1 test/back_and_forth_tables/two_row_int_array.json5 test/back_and_forth_tables/two_row_int_array_II.json5 temp.json5 && diff test/back_and_forth_tables/two_row_int_array_appended.json5 temp.json5
if [ $? -eq 0 ]; then
    echo "PASS: append json5 table"
    rm -f temp.json5
else
    echo "FAIL: append json5 table"
fi

