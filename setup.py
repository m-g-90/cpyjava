from distutils.core import setup, Extension
from distutils.spawn import find_executable
from distutils.util import get_platform
import os


try:
    import pip

    deps = ""

    for dep in deps.split(' '):
        try:
            if len(dep)>0:
                pip.main(['install', '--user', dep])
        except:
            pass
except:
    pass

sourcedir = os.path.dirname(os.path.realpath(__file__))


def findJavaFolders():
    java_folders = []
    try:
        tmp = os.environ['JAVA_HOME']
        if tmp is not None:
            if isinstance(tmp, str) and len(tmp) and os.path.isdir(tmp):
                java_folders.append(tmp)
    except:
        pass

    executables = ['java','java.exe']
    for executable in executables:
        try:
            path = find_executable(executable)
            if path is not None:
                java_folders.append(os.path.dirname(os.path.abspath(path)))
                java_folders.append(os.path.dirname(java_folders[-1]))
        except:
            pass

        try:
            path = find_executable(executable)
            if path is not None:
                import subprocess
                output = subprocess.run([path,'-server','-verbose:class'],stderr=subprocess.STDOUT,stdout=subprocess.PIPE,check=False).stdout
                import re
                path = re.match(r".*\[Loaded java\.lang\.Object from (?P<path>.*?)rt\.jar\].*",str(output)).group("path")
                if path is not None:
                    java_folders.append(os.path.dirname(os.path.abspath(path)))
                    java_folders.append(os.path.dirname(java_folders[-1]))
        except Exception as ex:
            print("Attempt to extract jvm path failed for "+executable+": "+repr(ex))

    return java_folders


def findJavaLibrary():
    if get_platform().startswith("win"):
        extension = '.dll'
    elif get_platform().startswith("linux"):
        extension = '.so'
    else:
        raise Exception('JVM search failed: unknown operating system.')

    subpaths = [
        os.path.join('jre','bin','server','jvm'),
        os.path.join('jre', 'bin', 'jvm'),
        os.path.join('jre', 'lib', 'jvm'),
        os.path.join('bin', 'server', 'jvm'),
        os.path.join('bin', 'jvm'),
        os.path.join('lib', 'jvm')
    ]

    java_folders = findJavaFolders()
    for prefix in java_folders:
        for mid in subpaths:
            if os.path.isfile(os.path.join(prefix,mid+extension)):
                return os.path.join(prefix,mid+extension)

    raise Exception('JVM search failed: no jvm'+extension+' found. (Searchpath: '+(';'.join(java_folders))+')')

define_macros = [('PYJAVA_SETUP_PY', '1',),('PYJAVA_EXPORT','1'),('PYJAVA_JVM_LOADLIBRARY','1')]
try:
    jvmfile = "\"\""+findJavaLibrary().replace("\\",'\\\\')+"\"\""
    define_macros.append(('PYJAVA_JVM_LOCATIONHINT',jvmfile))
except:
    pass

cpyjava_module = Extension('cpyjava',
                    define_macros = define_macros,
                    include_dirs = [os.path.join(sourcedir,'src')],
                    libraries = [],
                    library_dirs = [],
                    sources = [os.path.join(os.path.join(os.path.join(sourcedir,'src'),'pyjava'),x) for x in os.listdir(os.path.join(os.path.join(sourcedir,'src'),'pyjava')) if x.endswith(".c")])

setup (name = 'cpyjava',
       version = '0.6.3',
       description = 'python extension to use java objects',
       author = 'Marc Greim',
       author_email = '',
       url = 'https://github.com/m-g-90/cpyjava',
       long_description = '''

python extension to use java objects.

License: GNU Lesser General Public License v3.0

''',
       ext_modules = [cpyjava_module])
