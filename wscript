#!/usr/bin/env python
from waflib import Options
import os


def options(opt):
    opt.load('compiler_c')


def configure(conf):
    conf.load('compiler_c')
    conf.load("clang_compilation_database", tooldir="./tools/")
    conf.check_cc(lib='m', uselib_store='M', use='M')
    conf.check_cfg(package='pangocairo', args=['--cflags', '--libs'],
                   uselib_store='CAIRO')
    conf.check_cfg(path='gdal-config', args=['--cflags'], package='',
                   uselib_store='GDAL')
    conf.check_cfg(path='gdal-config', args=['--libs'], package='',
                   uselib_store='GDAL')

    conf.env.append_unique('CFLAGS', ['-std=c99', '-Wall', '-Wextra'])
    conf.define('_GNU_SOURCE', 1)  # for asprintf

    if 'DEBUG' in os.environ:
        conf.env.append_unique('CFLAGS', ['-g', '-fsanitize=address',
                                          '-fno-omit-frame-pointer'])
        conf.env.append_unique('LINKFLAGS', ['-fsanitize=address'])
        conf.define('DEBUG', 1)
    else:
        conf.env.append_unique('CFLAGS', ['-O3'])


def build(bld):
    sources = bld.path.ant_glob(['src/*.c'])
    kwargs = {
        'source': sources,
        'uselib': 'CAIRO GDAL M',
        'target': 'simple-tiles'
    }

    bld.shlib(**dict(list(kwargs.items()) + [('features', 'c cshlib')]))
    bld.stlib(**dict(list(kwargs.items()) + [('features', 'c cstlib')]))

    libs = []
    for k in ['LIB_GDAL', 'LIB_M']:
        if bld.env[k] != []:
            libs.append('-l' + ' -l'.join(bld.env[k]))

    includes = ' '.join(['-I' + ' -I'.join(bld.env[k]) for k
                         in ['INCLUDES_CAIRO', 'INCLUDES_GDAL']])

    bld(source='src/simple-tiles.pc.in', VERSION='0.6.0',
        LIBS=' '.join(libs), INCLUDES=includes)

    bld.install_files('${PREFIX}/include/simple-tiles',
                      bld.path.ant_glob(['src/*.h']))

    bld.recurse('test')


def test(ctx):
    Options.commands = ['build'] + Options.commands
    ctx.recurse('test', name='test')
