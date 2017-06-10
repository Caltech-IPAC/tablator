#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    conf.load('sqlite3')
    # Find vsqlitepp
    if conf.options.vsqlitepp_dir:
        if not conf.options.vsqlitepp_incdir:
            conf.options.vsqlitepp_incdir=conf.options.vsqlitepp_dir + "/include"
        if not conf.options.vsqlitepp_libdir:
            conf.options.vsqlitepp_libdir=conf.options.vsqlitepp_dir + "/lib"

    vsqlitepp_incdir=[]
    if conf.options.vsqlitepp_incdir:
        vsqlitepp_incdir=[conf.options.vsqlitepp_incdir]
        
    vsqlitepp_libdir=[]
    if conf.options.vsqlitepp_libdir:
        vsqlitepp_libdir=[conf.options.vsqlitepp_libdir]

    vsqlitepp_libs=['vsqlitepp']
    if conf.options.vsqlitepp_libs:
        vsqlitepp_libs=conf.options.vsqlitepp_libs.split()

    try:
        conf.check_cxx(msg='Checking for VSQLite++',
                       header_name='sqlite/connection.hpp',
                       includes=vsqlitepp_incdir,
                       uselib_store='vsqlitepp',
                       libpath=vsqlitepp_libdir,
                       rpath=vsqlitepp_libdir,
                       lib=vsqlitepp_libs,
                       use=['sqlite3'])
    except conf.errors.ConfigurationError:
        conf.fatal("Could not find vsqlite++ libraries")
        
def options(opt):
    vsqlitepp=opt.add_option_group('VSQLITE++ Options')
    vsqlitepp.add_option('--vsqlitepp-dir',
                   help='Base directory where vsqlite++ is installed')
    vsqlitepp.add_option('--vsqlitepp-incdir',
                   help='Directory where vsqlite++ include files are installed')
    vsqlitepp.add_option('--vsqlitepp-libdir',
                   help='Directory where vsqlite++ library files are installed')
    vsqlitepp.add_option('--vsqlitepp-libs',
                   help='Names of the vsqlitepp libraries without prefix or suffix\n'
                   '(e.g. "vsqlitepp")')
