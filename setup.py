from setuptools import setup, Distribution
import platform

LIBRARY_SUFFIX = 'so' if platform.system() == 'Linux' else 'pyd'

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name = 'bulkhasher',
    version = '0.0.2',
    description = 'Module for bulk hashing of files',
    url = None,
    author = 'Alex Murkoff',
    author_email = '413x1nkp@gmail.com',
    packages=['bulkhasher'],
    package_dir={'': '.'},
    package_data={
        'bulkhasher': [
            f'bulkhasher.{LIBRARY_SUFFIX}'
        ]
    },
    distclass=BinaryDistribution
)
