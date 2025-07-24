/// On Debian, Compile with
/// g++ extract_ra_dec.cxx -o extract_ra_dec -I/usr/include/hdf5/serial/ -std=c++11
/// -ltablator -lboost_filesystem -lboost_system

// JTODO Update this

#include <tablator/Table.hxx>

int main(int argc, char *argv[]) {
    /// Read in the table
    tablator::Table input_table(argv[1]);

    /// Find the indices for ra and dec
    size_t ra_index(-1), dec_index(-1);
    for (size_t column = 0; column < input_table.columns.size(); ++column) {
        if (input_table.columns[column].name == "ra") ra_index = column;
        if (input_table.columns[column].name == "dec") dec_index = column;
    }
    if (ra_index == -1 || dec_index == -1) {
        std::cerr << "Missing ra or dec\n" << std::flush;
        exit(1);
    }

    /// Create a Table structure for output with entries only for ra and
    /// dec.  Note that output_table.columns will have an entry for
    /// nulls.  So 'ra' will be in index 1, and 'dec' will be in index
    /// 2.
    tablator::Table output_table(std::vector<tablator::Column>(
            {input_table.columns[ra_index], input_table.columns[dec_index]}));

    /// Create a temporary Row to store what will eventually go into the
    /// output table.
    tablator::Row output_row(output_table.row_size());

    /// Copy the two columns from the input table to the output table.
    for (size_t row_offset = 0; row_offset < input_table.data.size();
         row_offset += input_table.row_size()) {
        output_row.set_zero();
        /// Copy ra
        if (input_table.is_null(row_offset, ra_index)) {
            /// Setting a column to be null in a Row is a bit clunky
            output_row.set_null(output_table.columns[1].type,
                                output_table.columns[1].array_size, 1,
                                output_table.offsets[1], output_table.offsets[2]);
        } else {
            /// Copy the data over directly.
            auto input_row_ptr = input_table.data.begin() + row_offset;
            output_row.insert(input_row_ptr + input_table.offsets[ra_index],
                              input_row_ptr + input_table.offsets[ra_index + 1],
                              output_table.offsets[1]);
        }
        /// Copy dec
        if (input_table.is_null(row_offset, dec_index)) {
            output_row.set_null(output_table.columns[2].type,
                                output_table.columns[2].array_size, 2,
                                output_table.offsets[2], output_table.offsets[3]);
        } else {
            auto input_row_ptr = input_table.data.begin() + row_offset;
            output_row.insert(input_row_ptr + input_table.offsets[dec_index],
                              input_row_ptr + input_table.offsets[dec_index + 1],
                              output_table.offsets[2]);
        }
        std::cout << "table\n";
        output_table.append_row(output_row);
    }
    output_table.write_output(argv[2]);
}
