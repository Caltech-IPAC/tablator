import os

def options(opt):
    opt.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx CCfits')

def configure(conf):
    conf.load('compiler_cxx gnu_dirs cxx11 hdf5_cxx CCfits')

def build(bld):
    default_flags=['-Wall', '-Wextra', '-Ofast']
    use_packages=['cxx11', 'hdf5', 'hdf5_cxx', 'CCfits']

    sources=['src/Table/Table.cxx',
             'src/Table/flatten_properties.cxx',
             'src/Table/read_fits.cxx',
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
             'src/Table/write_HDF5/write_HDF5.cxx',
             'src/Table/write_HDF5/write_HDF5_to_file.cxx']

    bld.shlib(source=sources,
              target='tablator',
              cxxflags=default_flags,
              use=use_packages
              )

