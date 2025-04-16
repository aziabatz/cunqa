# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here.
import sys
import os
import shutil
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))


os.environ['INFO_PATH'] = '/mnt/netapp1/Store_CESGA/home/cesga/dexposito/'
os.environ['STORE'] = '/mnt/netapp1/Store_CESGA/home/cesga/dexposito/'
# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'CUNQA'
copyright = '2025, CESGA'
author = 'Marta Losada Estévez, Jorge Vázquez Pérez, Álvaro Carballido Costas, Daniel Expósito Patiño'
release = '0.1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['sphinx.ext.githubpages',  'sphinx.ext.doctest', 'sphinx.ext.autodoc', 'sphinx.ext.autosummary', 'sphinx_mdinclude', "nbsphinx", 'sphinx.ext.mathjax',] # 'myst_parser', 'sphinx_mdinclude', this ones were troublesome

autodoc_mock_imports = ['os', 'json', 'JSONDecodeError', 'load', 'logger', 'qiskit', 'dateutil', 'glob','argparse','logging', 'numpy', 'cunqa.qclient', 'QClient', 'cunqa.mappers', 'cunqa.logger', 'qmiotools', 'qiskit_aer', 'Pandoc']

templates_path = ['_templates']
exclude_patterns = []
pygments_style = 'sphinx'

nbsphinx_allow_errors = True

autodoc_default_flags = ['members']
autosummary_generate = True

# mdinclude_dir = '/path/to/dir/'
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme' 
html_static_path = ['_static']
# html_css_files = ['_static/custom.css',]
html_logo = "_static/logo_cesga_blanco.png"
html_favicon = "_static/favicon.ico"
html_theme_options = {
    'logo_only': False,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'vcs_pageview_mode': '',
    'flyout_display': 'hidden',
    'version_selector': True,
    'language_selector': True,
    # Toc options
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False
}

#Copy jupyter notebooks to folder docs/examples so nbsphinx can read them for our gallery (these will be ignored to avoid duplication)

def setup(app):
    print('Setup is being run')
    here = Path(__file__).resolve()
    project_root = here.parents[2]  # Goes from /docs/ to project root
    source_notebooks_dir = project_root / 'examples'
    dest_dir = here.parent / 'examples'  # This will be docs/examples

    dest_dir.mkdir(exist_ok=True)

    for notebook in source_notebooks_dir.glob('*.ipynb'):
        shutil.copy(notebook, dest_dir / notebook.name)
