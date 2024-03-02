import sys
import os
import subprocess
import platform
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from pathlib import Path

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']

        self.build_temp = os.path.join(os.path.abspath(self.build_temp), 'build')
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

long_description = (Path(__file__).parent / "README.md").read_text()

setup(
    name='roboflex',
    version='0.1.33',
    description='Roboflex Core Library: a C++20 and python library for distributed robotics and automation.',
    url="https://github.com/flexrobotics/roboflex",
    author='Colin Prepscius',
    author_email='colinprepscius@gmail.com',
    long_description=long_description,
    long_description_content_type='text/markdown',
    classifiers = [
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Software Development :: Embedded Systems",
        "Framework :: Robot Framework",
        "Framework :: Robot Framework :: Library",
        "Framework :: Robot Framework :: Tool",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
    ],
    keywords = ["robotics", "middleware", "flexbuffers", "python", "c++", "c++20"],
    license = "MIT",
    python_requires='>=3.6',
    install_requires=['numpy'],
    ext_modules=[CMakeExtension('roboflex/roboflex_core_python_ext')],
    cmdclass=dict(build_ext=CMakeBuild),
    py_modules=['dynoflex', 'flexbuffers', 'flextensors', '__init__'],
    packages=['roboflex'],
    package_dir={'roboflex': 'python'}
)

