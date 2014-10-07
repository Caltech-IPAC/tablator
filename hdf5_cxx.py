#! /usr/bin/env python
# encoding: utf-8

def options(opt):
    hdf5=opt.add_option_group('HDF5 C/C++ Options')
    hdf5.add_option('--hdf5-dir',
                   help='Base directory where HDF5 C/C++ is installed')
    hdf5.add_option('--hdf5-incdir',
                   help='Directory where HDF5 C/C++ include files are installed')
    hdf5.add_option('--hdf5-libdir',
                   help='Directory where HDF5 C/C++ library files are installed')
    hdf5.add_option('--hdf5-c-libs',
                   help='Names of the HDF5 C libraries without prefix or suffix\n'
                   '(e.g. "hdf5")')
    hdf5.add_option('--hdf5-cxx-libs',
                   help='Names of the HDF5 C++ libraries without prefix or suffix\n'
                   '(e.g. "hdf5_cxx")')

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find HDF5 C
    if conf.options.hdf5_dir:
        if not conf.options.hdf5_incdir:
            conf.options.hdf5_incdir=conf.options.hdf5_dir + "/include"
        if not conf.options.hdf5_libdir:
            conf.options.hdf5_libdir=conf.options.hdf5_dir + "/lib"
    frag="#include <hdf5.h>\n" + "#include <hdf5_hl.h>\n" + 'int main()\n' \
        + "{H5TBget_table_info((hid_t)0,NULL,NULL,NULL);\n" \
        + "return H5Fopen(\"foo.h5\",H5F_ACC_RDONLY, H5P_DEFAULT);}\n"
    if conf.options.hdf5_incdir:
        hdf5_inc=[conf.options.hdf5_incdir]
    else:
        hdf5_inc=[]

    if conf.options.hdf5_c_libs:
        hdf5_c_libs=conf.options.hdf5_c_libs.split()
    else:
        hdf5_c_libs=['hdf5','hdf5_hl']

    conf.check_cxx(msg="Checking for HDF5 C bindings",
                  fragment=frag,
                  includes=hdf5_inc, uselib_store='hdf5',
                  libpath=[conf.options.hdf5_libdir],
                  rpath=[conf.options.hdf5_libdir],
                  lib=hdf5_c_libs)

    # Find HDF5 C++
    if conf.options.hdf5_dir:
        if not conf.options.hdf5_incdir:
            conf.options.hdf5_incdir=conf.options.hdf5_dir + "/include"
        if not conf.options.hdf5_libdir:
            conf.options.hdf5_libdir=conf.options.hdf5_dir + "/lib"
    frag="#include <H5Cpp.h>\n" + 'int main()\n' \
        + "{H5::Exception::dontPrint();}\n"
    if conf.options.hdf5_incdir:
        hdf5_inc=[conf.options.hdf5_incdir]
    else:
        hdf5_inc=[]

    if conf.options.hdf5_cxx_libs:
        hdf5_cxx_libs=conf.options.hdf5_cxx_libs.split()
    else:
        hdf5_cxx_libs=["hdf5_cpp"]

    conf.check_cxx(msg="Checking for HDF5 C++ bindings",
                  fragment=frag,
                  includes=hdf5_inc, uselib_store='hdf5_cxx',
                  libpath=[conf.options.hdf5_libdir],
                  rpath=[conf.options.hdf5_libdir],
                  lib=hdf5_cxx_libs+hdf5_c_libs)
