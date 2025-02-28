def options(opt):
    opt.load('compiler_cxx gnu_dirs cxx14 hdf5_cxx cfitsio CCfits boost json5_parser sqlite3 vsqlitepp')
    opt.add_option('--debug', help='Include debug symbols and turn ' +
                                   'compiler optimizations off',
                   action='store_true', default=False, dest='debug')

def configure(conf):
    conf.load('compiler_cxx gnu_dirs cxx14 hdf5_cxx cfitsio CCfits boost json5_parser sqlite3 vsqlitepp')
    conf.check_boost(lib='filesystem system program_options regex')

def build(bld):

    if bld.options.debug:
        default_flags=['-Wall', '-Wextra', '-g']
    else:
        default_flags=['-Wall', '-Wextra', '-g', '-Ofast', '-fno-finite-math-only', '-DNDEBUG']

    use_packages=['cxx14', 'hdf5', 'hdf5_cxx', 'cfitsio', 'CCfits', 'BOOST',
                  'json5_parser', 'sqlite3', 'vsqlitepp']

    sources=['src/fits_keyword_ucd_mapping.cxx',
             'src/Data_Type_Adjuster.cxx',
             'src/Format/set_from_extension.cxx',
             'src/Format/content_type.cxx',
             'src/Row/set_null.cxx',
             'src/H5_to_Data_Type.cxx',
             'src/quote_sql_string.cxx',
             'src/Table/Table.cxx',
             'src/Table/flatten_properties.cxx',
             'src/Table/extract_column_values_as_strings.cxx',
             'src/Table/read_unknown/read_unknown.cxx',
             'src/Table/read_unknown/is_fits.cxx',
             'src/Table/read_unknown/is_ipac_table.cxx',
             'src/Table/read_unknown/is_json5.cxx',
             'src/Table/read_unknown/is_votable.cxx',
             'src/Table/read_fits.cxx',
             'src/Table/read_hdf5/read_hdf5.cxx',
             'src/Table/read_hdf5/read_metadata.cxx',
             'src/Table/read_hdf5/read_column_metadata/read_column_metadata.cxx',
             'src/Table/read_hdf5/read_column_metadata/is_columns_valid.cxx',
             'src/Table/read_json5.cxx',
             'src/Table/read_ipac_table/read_ipac_table.cxx',
             'src/Table/read_ipac_table/read_ipac_header.cxx',
             'src/Table/read_ipac_table/shrink_ipac_string_columns_to_fit.cxx',
             'src/Table/read_ipac_table/create_types_from_ipac_headers/create_types_from_ipac_headers.cxx',
             'src/Table/read_ipac_table/create_types_from_ipac_headers/append_ipac_data_member.cxx',
             'src/Table/read_dsv/read_dsv.cxx',
             'src/Table/read_dsv/parse_dsv/parse_dsv.cxx',
             'src/Table/read_dsv/parse_dsv/DSV_Parser.cxx',
             'src/Table/read_dsv/read_dsv_rows.cxx',
             'src/Table/read_dsv/set_column_info/set_column_info.cxx',
             'src/Table/read_dsv/set_column_info/get_best_data_type.cxx',
             'src/Table/write.cxx',
             'src/Table/write_dsv.cxx',
             'src/Table/write_sql_create_table.cxx',
             'src/Table/write_sql_inserts.cxx',
             'src/Table/write_sql_insert.cxx',
             'src/Table/write_sqlite_db.cxx',
             'src/Table/write_fits.cxx',
             'src/Table/write_html.cxx',
             'src/Table/write_tabledata/write_tabledata.cxx',
             'src/Table/write_tabledata/decode_links.cxx',
             'src/Table/generate_property_tree/generate_property_tree.cxx',
             'src/Table/generate_property_tree/add_to_property_tree.cxx',
             'src/Table/write_hdf5/write_hdf5.cxx',
             'src/Table/write_hdf5/write_hdf5_to_H5File/write_hdf5_to_H5File.cxx',
             'src/Table/write_hdf5/write_hdf5_to_H5File/write_hdf5_attributes.cxx',
             'src/Table/write_hdf5/write_hdf5_to_H5File/write_hdf5_columns.cxx',
             'src/Ipac_Table_Writer/internals.cxx',
             'src/Ipac_Table_Writer/Ipac_Table_Writer.cxx',
             'src/Utils/Table_Utils/insert_ascii_in_row.cxx',
             'src/Utils/Table_Utils/append_column.cxx',
             'src/ptree_readers/ptree_readers.cxx',
             'src/ptree_readers/Utils.cxx',
             'src/ptree_readers/extract_attributes.cxx',
             'src/ptree_readers/read_property_tree_as_votable.cxx',
             'src/ptree_readers/read_property.cxx',
             'src/ptree_readers/read_group_element.cxx',
             'src/ptree_readers/read_resource_element/read_resource_element.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_table_element.cxx',
             'src/ptree_readers/read_resource_element/read_field/read_field.cxx',
             'src/ptree_readers/read_resource_element/read_field/read_values.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_data_element.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_tabledata/read_tabledata.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_tabledata/count_elements.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_binary2/read_binary2.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_binary2/decode_base64_stream.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_binary2/compute_column_array_sizes.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_binary2/append_data_from_stream/append_data_from_stream.cxx',
             'src/ptree_readers/read_resource_element/read_table_element/read_data_element/read_binary2/append_data_from_stream/insert_swapped.cxx',
             'src/Ascii_Writer/Ascii_Writer.cxx',
             'src/Decimal_String_Trimmer/Decimal_String_Trimmer.cxx'
	     ]

    bld.shlib(source=sources,
              target='tablator',
              name='libtablator_sh',
              cxxflags=default_flags,
              install_path=bld.env.LIBDIR,
              use=use_packages,
              vnum='4.0.3'
              )

    bld.stlib(source=sources,
              target='tablator',
              name='libtablator_st',
              cxxflags=default_flags,
              install_path=bld.env.LIBDIR,
              use=use_packages
              )

    bld.program(source=['src/tablator/main.cxx'],
                target='tablator',
                cxxflags=default_flags,
                use=use_packages + ['libtablator_st']
                )

    # install headers
    bld.install_files(bld.env.INCLUDEDIR + '/tablator',
                      bld.path.ant_glob("src/**/*.hxx"),
                      cwd=bld.path.find_dir('src'), relative_trick=True)

    bld.install_files(bld.env.MANDIR + '/man1',
                      ['doc/tablator.1'])
