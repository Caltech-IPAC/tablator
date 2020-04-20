#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find Json5_Parser
    if conf.options.json5_parser_dir:
        if not conf.options.json5_parser_incdir:
            conf.options.json5_parser_incdir=conf.options.json5_parser_dir + "/include"
        if not conf.options.json5_parser_libdir:
            conf.options.json5_parser_libdir=conf.options.json5_parser_dir + "/lib"
    frag="#include \"json5_parser.h\"\n" + 'int main()\n' \
        + "{json5_parser::Value v1( true );}\n"
    if conf.options.json5_parser_incdir:
        json5_parser_inc=[conf.options.json5_parser_incdir]
    else:
        json5_parser_inc=[]
    if conf.options.json5_parser_libdir:
        json5_parser_libdir=[conf.options.json5_parser_libdir]
    else:
        json5_parser_libdir=[]
    if conf.options.json5_parser_libs:
        json5_parser_libs=conf.options.json5_parser_libs.split()
    else:
        json5_parser_libs=["json5_parser"]

    conf.check_cxx(msg="Checking for Json5_Parser",
                  fragment=frag,
                  includes=json5_parser_inc, uselib_store='json5_parser',
                  libpath=json5_parser_libdir,
                  rpath=json5_parser_libdir,
                  lib=json5_parser_libs)

def options(opt):
    json5_parser=opt.add_option_group('json5_parser Options')
    json5_parser.add_option('--json5_parser-dir',
                   help='Base directory where json5_parser is installed')
    json5_parser.add_option('--json5_parser-incdir',
                   help='Directory where json5_parser include files are installed')
    json5_parser.add_option('--json5_parser-libdir',
                   help='Directory where json5_parser library files are installed')
    json5_parser.add_option('--json5_parser-libs',
                   help='Names of the json5_parser libraries without prefix or suffix\n'
                   '(e.g. "json5_parser"')

