import os

def options(opt):
    opt.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx cfitsio CCfits boost')

def configure(conf):
    conf.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx cfitsio CCfits boost')
    conf.check_boost(lib='filesystem system')

def build(bld):
    default_flags=['-Wall', '-Wextra', '-Ofast']
    # default_flags=['-Wall', '-Wextra', '-g']
    use_packages=['cxx11', 'hdf5', 'hdf5_cxx', 'cfitsio', 'CCfits', 'BOOST']

    sources=['src/fits_keyword_mapping.cxx',
             'src/Format/set_from_extension.cxx',
             'src/Table/Table.cxx',
             'src/Table/flatten_properties.cxx',
             'src/Table/read_fits.cxx',
             'src/Table/read_hdf5.cxx',
             'src/Table/write_output.cxx',
             'src/Table/write_csv_tsv.cxx',
             'src/Table/write_fits.cxx',
             'src/Table/write_html.cxx',
             'src/Table/put_table_in_property_tree.cxx',
             'src/Table/write_votable/write_votable.cxx',
             'src/Table/write_votable/Field_Properties_to_xml.cxx',
             'src/Table/write_ipac_table/write_ipac_table.cxx',
             'src/Table/write_ipac_table/write_ipac_table_header.cxx',
             'src/Table/write_ipac_table/write_element_type.cxx',
             'src/Table/write_hdf5/write_hdf5.cxx',
             'src/Table/write_hdf5/write_hdf5_to_file.cxx']

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
