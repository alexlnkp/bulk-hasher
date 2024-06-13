from setuptools.command import build_ext as build_ext
from setuptools import setup, Extension
from pathlib import Path
import sysconfig
import os

ROOT_CWD = "/home/alex/coding/RVCC/"

# Implementation heavily based upon https://github.com/tim-mitchell/prebuilt_binaries
class PrebuiltExtension(Extension):
    def __init__(self, input_file, package=None):
        name = Path(input_file).stem
        if package is not None:
            name = f'{package}.{name}'
        if not os.path.exists(input_file):
            raise ValueError(f'Prebuilt extension file not found\n{input_file}')
        self.input_file = input_file
        super().__init__(name, ['no-source-needed.c'])

def find_shared_objects(build_lib_path):
    so_files = []
    lib_folder = Path(build_lib_path)
    if not lib_folder.exists():
        return so_files
    
    for so_file in lib_folder.rglob('*.so'):
        rel_path = so_file.relative_to(lib_folder).parts
        dest_dir = Path('lib', *rel_path[:-1])
        so_files.append((str(dest_dir), [str(so_file)]))
    
    return so_files

class prebuilt_binary(build_ext.build_ext):

    def run(self):
        for ext in self.extensions:
            fullname = self.get_ext_fullname(ext.name)
            filename = self.get_ext_filename(fullname)
            dest_filename = os.path.join(self.build_lib, filename)
            dest_folder = os.path.dirname(dest_filename)
            if not os.path.exists(dest_folder):
                os.makedirs(dest_folder)

            os.makedirs(Path(ext.input_file).parent.parent.joinpath(Path(dest_filename)).parent, exist_ok=True)
            os.rename(
                ext.input_file, Path(ext.input_file).parent.parent.joinpath(Path(dest_filename))
            )
        if self.inplace:
            self.copy_extensions_to_source()

class CustomPrebuiltBinary(prebuilt_binary):
    def run(self):
        super().run()

        build_lib = Path(ROOT_CWD).joinpath(Path(self.build_lib))
        so_files = list(build_lib.rglob('*.so'))
        
        data_files = []
        for so_file in so_files:
            data_files.append(('', [str(so_file)]))

        self.distribution.package_data['bulkhasher'] = data_files

build_lib_path = os.environ.get('PREBUILT_BINARY', '')
ext_module = PrebuiltExtension(build_lib_path)

setup(
    name='bulkhasher',
    version='0.0.2',
    description='Module for bulk hashing of files',
    author='Alex Murkoff',
    author_email='413x1nkp@gmail.com',
    cmdclass={'build_ext': CustomPrebuiltBinary, 'build_py': prebuilt_binary},
    ext_modules=[ext_module],
    data_files=[('lib/python3.12/site-packages', ['/home/alex/coding/RVCC/build/lib.linux-x86_64-cpython-312/bulkhasher.cpython-312-x86_64-linux-gnu.so'])],

    package_data={'': ['*.so']},
    include_package_data=True,

)
