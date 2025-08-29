{{ fullname | escape | underline}}

.. automodule:: {{ fullname }}


{% block modules %}
{% if modules %}
.. raw:: html

   <h2> {{ _('Modules') }} </h2>

.. autosummary::
   :toctree:
   :template: custom-module-template.rst
   :recursive:
{% for item in modules %}
   {{ item }}
{%- endfor %}
{% endif %}
{% endblock %}
