from setuptools.command import build_ext
from setuptools import setup, Extension
from pathlib import Path
import os

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


ext_module = PrebuiltExtension(os.environ['PREBUILT_BINARY'])

setup(
    name='bulkhasher',
    version='0.0.2',
    description='Module for bulk hashing of files',
    author='Alex Murkoff',
    author_email='413x1nkp@gmail.com',
    cmdclass={'build_ext': prebuilt_binary, 'build_py': prebuilt_binary},
    ext_modules=[ext_module]
    
)
