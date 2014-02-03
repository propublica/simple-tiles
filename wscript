#!/usr/bin/env python


def options(opt):
    opt.load('compiler_c')


def configure(conf):
    conf.load('compiler_c')
    conf.check_cfg(package='pangocairo', args=['--cflags', '--libs'], uselib_store='CAIRO')
    conf.check_cfg(path='gdal-config', args=['--cflags'], package='',
                   uselib_store='GDAL')
    conf.check_cfg(path='gdal-config', args=['--libs'], package='',
                   uselib_store='GDAL')
    conf.env.append_unique('CFLAGS', ['-std=c99', '-Wall', '-Wextra'])
    if conf.env['DEBUG'] is not None:
        conf.env.append_unique('CFLAGS', ['-g'])


def build(bld):
    sources = bld.path.ant_glob(['src/*.c'])
    bld.shlib(
        features='c cshlib',
        source=sources,
        target='simple-tiles',
        uselib='CAIRO GDAL'
    )

    bld.stlib(
        features='c cstlib',
        source=sources,
        target='simple-tiles',
        uselib='CAIRO GDAL'
    )

    bld.recurse('test')


def test(ctx):
    from waflib import Options
    Options.commands = ['build'] + Options.commands
    ctx.recurse('test', name='test')
