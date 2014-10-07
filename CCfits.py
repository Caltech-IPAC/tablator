#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find CCFITS
    if conf.options.ccfits_dir:
        if not conf.options.ccfits_incdir:
            conf.options.ccfits_incdir=conf.options.ccfits_dir + "/include"
        if not conf.options.ccfits_libdir:
            conf.options.ccfits_libdir=conf.options.ccfits_dir + "/lib"

    if conf.options.ccfits_incdir:
        ccfits_incdir=[conf.options.ccfits_incdir]
    else:
        ccfits_incdir=[]
    if conf.options.ccfits_libdir:
        ccfits_libdir=[conf.options.ccfits_libdir]
    else:
        ccfits_libdir=[]

    if conf.options.ccfits_libs:
        ccfits_libs=conf.options.ccfits_libs.split()
    else:
        ccfits_libs=['CCfits']

    conf.check_cxx(msg="Checking for CCfits",
                  header_name='CCfits/CCfits',
                  includes=ccfits_incdir,
                  uselib_store='CCfits',
                  libpath=ccfits_libdir,
                  rpath=ccfits_libdir,
                  lib=ccfits_libs)

def options(opt):
    ccfits=opt.add_option_group('CCfits Options')
    ccfits.add_option('--ccfits-dir',
                   help='Base directory where ccfits is installed')
    ccfits.add_option('--ccfits-incdir',
                   help='Directory where ccfits include files are installed')
    ccfits.add_option('--ccfits-libdir',
                   help='Directory where ccfits library files are installed')
    ccfits.add_option('--ccfits-libs',
                   help='Names of the ccfits libraries without prefix or suffix\n'
                   '(e.g. "CCFITS")')
