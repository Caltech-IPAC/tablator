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

    if conf.options.hdf5_dir:
        if not conf.options.hdf5_incdir:
            conf.options.hdf5_incdir=conf.options.hdf5_dir + "/include"
        if not conf.options.hdf5_libdir:
            conf.options.hdf5_libdir=conf.options.hdf5_dir + "/lib"
    
    if not conf.options.hdf5_incdir and not conf.options.hdf5_libdir \
       and not conf.options.hdf5_c_libs and not conf.options.hdf5_cxx_libs:
        hdf5_config=[[[],[],['hdf5'],['hdf5_cpp']],
                     [['/usr/include/hdf5/serial'], [],
                      ['hdf5_serial'],['hdf5_cpp']]]
    else:
        if conf.options.hdf5_incdir:
            hdf5_inc=[conf.options.hdf5_incdir]
        else:
            hdf5_inc=[]

        hdf5_c_libs=["hdf5"]
        if conf.options.hdf5_c_libs:
            hdf5_c_libs=conf.options.hdf5_c_libs.split()

        hdf5_cxx_libs=["hdf5_cpp"]
        if conf.options.hdf5_cxx_libs:
            hdf5_cxx_libs=conf.options.hdf5_c_libs.split()

        hdf5_config=[[hdf5_inc,[conf.options.hdf5_libdir],hdf5_c_libs,hdf5_cxx_libs]]
    
    # Find HDF5 C
    frag="#include <hdf5.h>\n" + 'int main()\n' \
        + "{return H5Fopen(\"foo.h5\",H5F_ACC_RDONLY, H5P_DEFAULT);}\n"
    
    found_hdf5_c=False
    for c in hdf5_config:
        try:
            conf.check_cxx(msg="Checking for HDF5 C bindings using: " + str(c),
                           fragment=frag,
                           includes=c[0], uselib_store='hdf5',
                           libpath=c[1],
                           rpath=c[1],
                           lib=c[2])
        except conf.errors.ConfigurationError:
            continue
        else:
            found_hdf5_c=True
            break
    if not found_hdf5_c:
        conf.fatal("Could not find HDF5 C libraries")
            
    # Find HDF5 C++
    frag="#include <H5Cpp.h>\n" + 'int main()\n' \
        + "{H5::Exception::dontPrint();}\n"

    found_hdf5_cxx=False
    for c in hdf5_config:
        try:
            conf.check_cxx(msg="Checking for HDF5 C++ bindings using: " + str(c),
                           fragment=frag,
                           includes=c[0], uselib_store='hdf5_cxx',
                           libpath=c[1],
                           rpath=c[1],
                           lib=c[2] + c[3])
        except conf.errors.ConfigurationError:
            continue
        else:
            found_hdf5_c=True
    if not found_hdf5_c:
        conf.fatal("Could not find HDF5 C++ libraries")
