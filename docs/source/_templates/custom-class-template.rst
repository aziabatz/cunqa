{{ fullname | escape | underline }}

.. currentmodule:: {{ module }}

.. autoclass:: {{ objname }}
   :show-inheritance:

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

{% if methods %}
{{ _('Methods') | escape | underline('-') }}

{% for item in methods%}
{%- if not item.startswith('_') %}
.. automethod:: {{ name }}.{{ item }}
{% elif item == '__init__' %}
.. automethod:: {{ name }}.{{ item }}
{%- endif %}{% endfor %}
{% endif %}

