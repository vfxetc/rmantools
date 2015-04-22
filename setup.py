from subprocess import check_call

from setuptools import setup, find_packages
from distutils.command.build import build as default_build



class build(default_build):

    def run(self):
        check_call(['make'])
        default_build.run(self)


setup(
    name='rmantools',
    version='0.1.0b1',
    description='A collection of Python, SL, OSL, and C++ for RenderMan 19.',
    url='http://github.com/westernx/rmantools',
    
    packages=find_packages(exclude=['build*', 'tests*']),
    include_package_data=True,
    
    author='Mike Boers',
    author_email='rmantools@mikeboers.com',
    license='BSD-3',

    cmdclass={'build': build},

    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Natural Language :: English',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 2',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    
)