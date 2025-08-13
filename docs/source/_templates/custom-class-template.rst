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

.. automethod:: {{ name }}.__init__

{% set ns = namespace(pub=[]) %}
{% for item in methods %}
{% if item != '__init__' and item[0] != '_' %}
{% set ns.pub = ns.pub + [item] %}
{% endif %}
{% endfor %}

{% set sorted_methods = ns.pub | sort %}
{% for item in sorted_methods %}
.. automethod:: {{ name }}.{{ item }}
{% endfor %}
{% endif %}
