from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import subprocess
import platform
import sys
import os

LIBRARY_SUFFIX = 'so' if platform.system() == 'Linux' else 'pyd'

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir='.'):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        os.makedirs(self.build_temp, exist_ok=True)
        os.makedirs(self.build_lib, exist_ok=True)

        subprocess.check_call(
            [
                'cmake',
                '-DPYTHON_INCLUDE_DIR=$(python3 -c "import sysconfig; print(sysconfig.get_path(\'include\'))")',
                '-DPYTHON_LIBRARY=$(python3 -c "import sysconfig; print(sysconfig.get_config_var(\'LIBDIR\'))")',
                '-B',
                self.build_temp,
                '-DCMAKE_BUILD_TYPE=' + cfg,
                ext.sourcedir
            ] + cmake_args, cwd=self.build_temp
        )
        subprocess.check_call(['cmake', '--build', self.build_temp] + build_args, cwd=self.build_temp)


setup(
    name = 'bulkhasher',
    version = '0.0.5',
    description = 'Module for bulk hashing of files',
    long_description="Made on C",
    author = 'Alex Murkoff',
    author_email = '413x1nkp@gmail.com',
    packages=['bulkhasher'],
    package_dir={'': '.'},
    package_data={
        'bulkhasher': [
            f'bulkhasher.{LIBRARY_SUFFIX}'
        ]
    },
    ext_modules=[CMakeExtension('bulkhasher')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
