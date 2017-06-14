import sys

if not (sys.version_info[0] >= 2 and sys.version_info[1] >= 5):
  print 'You must be running Python 2.5 or greater'
  sys.exit(-1)

def exit_with_error(s):
  print s
  sys.exit(-1)

def check_import(pkgname, pkgver, pkglongname):
    try:
        mod = __import__(pkgname)
    except ImportError:
        exit_with_error(
            "Can't find a local %s Python installation."
            "Please read carefully the ``README`` file "
            "and remember that we need the %s package "
            "to compile and run." % (pkglongname, pkglongname) )
    else:
        if mod.__version__ < pkgver:
            exit_with_error(
                "You need %(pkgname)s %(pkgver)s or greater to run!"
                % {'pkgname': pkglongname, 'pkgver': pkgver} )

    print ( "* Found %(pkgname)s %(pkgver)s package installed."
            % {'pkgname': pkglongname, 'pkgver': mod.__version__} )
    globals()[pkgname] = mod

check_import('numpy', '1.0.4', 'numpy')

from distutils.core import setup, Extension
from Cython.Distutils import build_ext
import numpy
from numpy.distutils.misc_util import get_numpy_include_dirs

inc_dirs = ['jp_draw/']
inc_dirs.extend(get_numpy_include_dirs())

extensions = [
  Extension('jp_draw.jp_draw',
            include_dirs=inc_dirs+['/usr/local/include/cairo','/usr/local/cairo'],
            sources=['jp_draw/jp_draw.pyx', 'jp_draw/_jp_draw.cpp'],
            depends=['jp_draw/jp_draw.hpp', 'jp_draw/jp_jpeg.hpp'],
            language="c++",
            libraries=['cairo','jpeg']
            ),
];

setup(name='jp_draw',
      version='0.3.81',
      description='Drawing utilities',
      author='James Philbin, Relja Arandjelovic',
      author_email='philbinj@gmail.com, relja@robots.ox.ac.uk',
      packages=['jp_draw'],
      ext_modules=extensions,
      cmdclass = {'build_ext' : build_ext})
