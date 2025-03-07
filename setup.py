
from pathlib import Path
from setuptools import setup, find_packages

source = Path(__file__).resolve().parent
version = (source / "VERSION").read_text().strip()
__version__ = version

setup(
    name="cunqa",
    version=version,
    packages=find_packages()
)