{{ fullname | escape | underline }}

.. currentmodule:: {{ module }}

.. autoclass:: {{ objname }}
   :show-inheritance:
   :member-order: bysource
   :special-members: __init__

{# === TABLAS (resumen) === #}
{% if attributes %}
{{ _('Attributes') | escape | underline('-') }}

.. autosummary::
   :nosignatures:
{% for item in attributes%}
{%- if not item.startswith('_') %}
   ~{{ name }}.{{ item }}
{%- endif %}{% endfor %}
{% endif %}

.. .. {% if methods %}
.. .. {{ _('Methods') | escape | underline('-') }}

.. .. {% for item in methods%}
.. .. {%- if not item.startswith('_') %}
.. .. automethod:: {{ name }}.{{ item }}
.. .. {%- endif %}
.. .. {% endfor %}
.. .. {% endif %}
