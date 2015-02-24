#!/usr/bin/env python
from waflib import Options
import sys, os


def options(opt):
    opt.load('compiler_c')


def configure(conf):
    conf.load('compiler_c')
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
        conf.env.append_unique('CFLAGS', ['-g'])
        conf.define('DEBUG', 1)
    else:
        conf.env.append_unique('CFLAGS', ['-O3'])

    if sys.platform.startswith('darwin'):
        conf.define('ST_APPLE', 1)
    elif sys.platform.startswith('linux'):
        conf.define('ST_LINUX', 1)
        conf.check_cc(lib='GL', uselib_store='GL', use='GL')
        conf.check_cc(lib='GLU', uselib_store='GL', use='GL')
        conf.check_cfg(package='osmesa', args=['--cflags', '--libs'],
                       uselib_store='GL')


def build(bld):
    sources = bld.path.ant_glob(['src/*.c'])
    kwargs = {
        'source': sources,
        'uselib': 'GL CAIRO GDAL M',
        'target': 'simple-tiles'
    }

    if sys.platform.startswith('darwin'):
        kwargs['framework'] = ["OpenGL"]

    bld.shlib(**dict(kwargs.items() + [('features', 'c cshlib')]))
    bld.stlib(**dict(kwargs.items() + [('features', 'c cstlib')]))

    libs = []
    for k in ['LIB_GDAL', 'LIB_M', 'LIB_GL', 'LIB_CAIRO']:
        if bld.env[k] != []:
            libs.append('-l' + ' -l'.join(bld.env[k]))

    includes = ' '.join(['-I' + ' -I'.join(bld.env[k]) for k in ['INCLUDES_CAIRO', 'INCLUDES_GDAL']])

    bld(source='src/simple-tiles.pc.in', VERSION='0.5.0',
        LIBS=' '.join(libs), INCLUDES=includes)

    bld.install_files('${PREFIX}/include/simple-tiles',
                      bld.path.ant_glob(['src/*.h']))

    bld.recurse('test')


def test(ctx):
    Options.commands = ['build'] + Options.commands
    ctx.recurse('test', name='test')
