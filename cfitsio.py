#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find cfitsio
    if conf.options.cfitsio_dir:
        if not conf.options.cfitsio_incdir:
            conf.options.cfitsio_incdir=conf.options.cfitsio_dir + "/include"
        if not conf.options.cfitsio_libdir:
            conf.options.cfitsio_libdir=conf.options.cfitsio_dir + "/lib"

    if conf.options.cfitsio_incdir:
        cfitsio_incdir=[conf.options.cfitsio_incdir]
    else:
        cfitsio_incdir=[]
    if conf.options.cfitsio_libdir:
        cfitsio_libdir=[conf.options.cfitsio_libdir]
    else:
        cfitsio_libdir=[]

    if conf.options.cfitsio_libs:
        cfitsio_libs=conf.options.cfitsio_libs.split()
    else:
        cfitsio_libs=['cfitsio']

    conf.check_cc(msg="Checking for CFITSIO",
                  header_name='fitsio.h',
                  includes=cfitsio_incdir,
                  uselib_store='cfitsio',
                  libpath=cfitsio_libdir,
                  rpath=cfitsio_libdir,
                  lib=cfitsio_libs)

def options(opt):
    cfitsio=opt.add_option_group('CFITSIO Options')
    cfitsio.add_option('--cfitsio-dir',
                   help='Base directory where cfitsio is installed')
    cfitsio.add_option('--cfitsio-incdir',
                   help='Directory where cfitsio include files are installed')
    cfitsio.add_option('--cfitsio-libdir',
                   help='Directory where cfitsio library files are installed')
    cfitsio.add_option('--cfitsio-libs',
                   help='Names of the cfitsio libraries without prefix or suffix\n'
                   '(e.g. "cfitsio")')
