#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    # Find sqlite3
    if conf.options.sqlite3_dir:
        if not conf.options.sqlite3_incdir:
            conf.options.sqlite3_incdir=conf.options.sqlite3_dir + "/include"
        if not conf.options.sqlite3_libdir:
            conf.options.sqlite3_libdir=conf.options.sqlite3_dir + "/lib"

    sqlite3_incdir=[]
    if conf.options.sqlite3_incdir:
        sqlite3_incdir=[conf.options.sqlite3_incdir]
        
    sqlite3_libdir=[]
    if conf.options.sqlite3_libdir:
        sqlite3_libdir=[conf.options.sqlite3_libdir]

    sqlite3_libs=['sqlite3']
    if conf.options.sqlite3_libs:
        sqlite3_libs=conf.options.sqlite3_libs.split()

    try:
        conf.check_cxx(msg='Checking for SQLite',
                       header_name='sqlite3.h',
                       includes=sqlite3_incdir,
                       uselib_store='sqlite3',
                       libpath=sqlite3_libdir,
                       rpath=sqlite3_libdir,
                       lib=sqlite3_libs)
    except conf.errors.ConfigurationError:
        conf.fatal("Could not find sqlite3 libraries")
        
def options(opt):
    sqlite3=opt.add_option_group('SQLITE3 Options')
    sqlite3.add_option('--sqlite3-dir',
                   help='Base directory where sqlite3 is installed')
    sqlite3.add_option('--sqlite3-incdir',
                   help='Directory where sqlite3 include files are installed')
    sqlite3.add_option('--sqlite3-libdir',
                   help='Directory where sqlite3 library files are installed')
    sqlite3.add_option('--sqlite3-libs',
                   help='Names of the sqlite3 libraries without prefix or suffix\n'
                   '(e.g. "sqlite3")')
