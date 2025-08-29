{{ fullname | escape | underline}}

CUNQA module
=============

.. automodule:: {{ fullname }}

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
