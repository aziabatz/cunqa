# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here.
import sys
import re
import os
import shutil
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'CUNQA'
copyright = '2025, CESGA'
author = 'Marta Losada Estévez, Jorge Vázquez Pérez, Álvaro Carballido Costas, Daniel Expósito Patiño'
release = '0.1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration


extensions = ['sphinx.ext.githubpages', 'sphinx.ext.doctest', 'sphinx.ext.autodoc', 'sphinx.ext.autosummary', 'sphinx_mdinclude', "nbsphinx", 'sphinx.ext.mathjax'] # 'myst_parser', 'sphinx_mdinclude', this ones were troublesome

autodoc_mock_imports = ['os', 'json', 'JSONDecodeError', 'load', 'logger', 'qiskit', 'dateutil', 'glob','argparse','logging', 'numpy', 'cunqa.qclient', 'QClient', 'cunqa.logger', 'qmiotools', 'qiskit_aer', 'Pandoc']


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
    'collapse_navigation': False,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False
}


# Auxiliary functions to convert extended markdown notes to basic markdown structures that mdinclude can read (in First Distributed Execution)
ADMONITION_MAP = {
    "NOTE": ("📘", "Note"),
    "WARNING": ("⚠️", "Warning"),
    "IMPORTANT": ("❗", "Important"),
    "TIP": ("💡", "Tip"),
    "CAUTION": ("🚧", "Caution"),
    "INFO": ("ℹ️", "Info"),
}

def convert_gfm_admonitions_to_simple(text):
    lines = text.splitlines()
    output = []
    in_admonition = False
    header = ""

    for line in lines:
        match = re.match(r"^>\s*\[!(\w+)\]\s*$", line.strip(), re.IGNORECASE)
        if match:
            admonition_type = match.group(1).upper()
            emoji, title = ADMONITION_MAP.get(admonition_type, ("💬", admonition_type.capitalize()))
            header = f"> {emoji} **{title}:**"
            output.append(header)
            in_admonition = True
        elif in_admonition:
            if line.strip().startswith(">"):
                content = line.strip()[1:].lstrip()
                output.append(f"> {content}")
            else:
                # End of admonition block
                output.append(line)
                in_admonition = False
        else:
            output.append(line)

    return "\n".join(output)

def process_readme_markdown_admonitions(app, docname, source):
    # Only transform Markdown (MyST) files. It's hardcoded to only transform README
    if docname.lower() == "readme":
        source[0] = convert_gfm_admonitions_to_simple(source[0])

#The following function is run when we do the sphinx "make html". It copies the example jupyter notebooks from outside docs and 

def setup(app):
    #Copy jupyter notebooks to folder docs/examples so nbsphinx can read them for our gallery (these will be ignored to avoid duplication)
    here = Path(__file__).resolve()
    project_root = here.parents[2]  # Goes from /docs/ to project root
    source_notebooks_dir = project_root / 'examples'
    dest_dir = here.parent / 'examples'  # This will be docs/examples

    dest_dir.mkdir(exist_ok=True)

    for notebook in source_notebooks_dir.glob('*.ipynb'):
        shutil.copy(notebook, dest_dir / notebook.name)

    #calls the auxiliary fucntion that transforms the readme
    # app.connect('source-read', process_readme_markdown_admonitions)  


