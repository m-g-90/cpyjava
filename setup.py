from distutils.core import setup, Extension
import os

sourcedir = os.path.dirname(os.path.realpath(__file__))


cpyjava_module = Extension('cpyjava',
                    define_macros = [('PYJAVA_SETUP_PY', '1'),('PYJAVA_EXPORT','1')],
                    include_dirs = [os.path.join(sourcedir,'src')],
                    libraries = [],
                    library_dirs = [],
                    sources = [os.path.join(os.path.join(os.path.join(sourcedir,'src'),'pyjava'),x) for x in os.listdir(os.path.join(os.path.join(sourcedir,'src'),'pyjava')) if x.endswith(".c")])

setup (name = 'cpyjava',
       version = '0.3',
       description = 'python extension to use java objects',
       author = 'Marc Greim',
       author_email = '',
       url = 'https://github.com/m-g-90/cpyjava',
       long_description = '''

python extension to use java objects.

License: GNU Lesser General Public License v3.0

''',
       ext_modules = [cpyjava_module])