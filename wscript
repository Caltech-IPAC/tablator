import os

def options(opt):
    opt.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx cfitsio CCfits boost json5_parser')

def configure(conf):
    conf.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx cfitsio CCfits boost json5_parser')
    conf.check_boost(lib='filesystem system regex')

def build(bld):
    default_flags=['-Wall', '-Wextra', '-Ofast', '-DNDEBUG']
    # default_flags=['-Wall', '-Wextra', '-g']
    use_packages=['cxx11', 'hdf5', 'hdf5_cxx', 'cfitsio', 'CCfits', 'BOOST',
                  'json5_parser']

    sources=['src/fits_keyword_mapping.cxx',
             'src/Format/set_from_extension.cxx',
             'src/Row/set_null.cxx',
             'src/H5_to_Data_Type.cxx',
             'src/Table/Table.cxx',
             'src/Table/append_column.cxx',
             'src/Table/flatten_properties.cxx',
             'src/Table/insert_ascii_in_row.cxx',
             'src/Table/read_fits.cxx',
             'src/Table/read_hdf5.cxx',
             'src/Table/read_json5.cxx',
             'src/Table/read_ipac_table/read_ipac_table.cxx',
             'src/Table/read_ipac_table/read_ipac_header.cxx',
             'src/Table/read_ipac_table/shrink_ipac_string_columns_to_fit.cxx',
             'src/Table/read_ipac_table/create_types_from_ipac_headers/create_types_from_ipac_headers.cxx',
             'src/Table/read_ipac_table/create_types_from_ipac_headers/append_ipac_data_member.cxx',
             'src/Table/read_property_tree_as_votable/read_property_tree_as_votable.cxx',
             'src/Table/read_property_tree_as_votable/read_node_and_attributes.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_resource.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_table.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_field.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_data.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_tabledata/read_tabledata.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_tabledata/count_elements.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_binary2/read_binary2.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_binary2/decode_base64_stream.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_binary2/compute_column_array_sizes.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_binary2/append_data_from_stream/append_data_from_stream.cxx',
             'src/Table/read_property_tree_as_votable/read_resource/read_table/read_data/read_binary2/append_data_from_stream/insert_swapped.cxx',
             'src/Table/read_dsv/read_dsv.cxx',
             'src/Table/read_dsv/parse_dsv/parse_dsv.cxx',
             'src/Table/read_dsv/parse_dsv/DSV_Parser.cxx',
             'src/Table/read_dsv/read_dsv_rows.cxx',
             'src/Table/read_dsv/set_column_info/set_column_info.cxx',
             'src/Table/read_dsv/set_column_info/get_best_data_type.cxx',
             'src/Table/write_output.cxx',
             'src/Table/write_csv_tsv.cxx',
             'src/Table/write_fits.cxx',
             'src/Table/write_html.cxx',
             'src/Table/write_tabledata.cxx',
             'src/Table/generate_property_tree/generate_property_tree.cxx',
             'src/Table/generate_property_tree/Field_Properties_to_property_tree.cxx',
             'src/Table/write_ipac_table/write_ipac_table.cxx',
             'src/Table/write_ipac_table/write_ipac_table_header.cxx',
             'src/Table/write_ipac_table/to_ipac_string.cxx',
             'src/Table/write_ipac_table/get_column_width.cxx',
             'src/Table/write_hdf5/write_hdf5.cxx',
             'src/Table/write_hdf5/write_hdf5_to_file/write_hdf5_to_file.cxx',
             'src/Table/write_hdf5/write_hdf5_to_file/write_hdf5_attributes.cxx',
             'src/Table/write_type_as_ascii.cxx']

    bld.shlib(source=sources,
              target='tablator',
              name='libtablator_sh',
              cxxflags=default_flags,
              install_path=os.path.join(bld.env.PREFIX, 'lib'),
              use=use_packages
              )

    bld.stlib(source=sources,
              target='tablator',
              name='libtablator_st',
              cxxflags=default_flags,
              install_path=os.path.join(bld.env.PREFIX, 'lib'),
              use=use_packages
              )

    bld.program(source=['src/tablator.cxx'],
                target='tablator',
                cxxflags=default_flags,
                rpath=[bld.env.LIBDIR],
                use=use_packages + ['libtablator_st']
                )

    # install headers
    bld.install_files(bld.env.INCLUDEDIR + '/tablator',
                      bld.path.ant_glob('src/*.hxx'),
                      cwd=bld.path.find_dir('src'), relative_trick=True)
