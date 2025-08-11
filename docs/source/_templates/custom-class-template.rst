{{ fullname | escape | underline }}

.. currentmodule:: {{ module }}

.. autoclass:: {{ objname }}
   :show-inheritance:
   :inherited-members:
   :special-members: __call__, __add__, __mul__

{# === TABLAS (resumen) === #}
{% if attributes %}
{{ _('Attributes') | escape | underline('-') }}

.. autosummary::
   :nosignatures:
{% for item in attributes %}
{%- if not item.startswith('_') %}
   ~{{ name }}.{{ item }}
{%- endif %}{% endfor %}
{% endif %}

{% if methods %}
{{ _('Method reference') | escape | underline('~') }}

{% for item in methods %}
{%- if not item.startswith('_') %}
.. automethod:: {{ name }}.{{ item }}
{%- endif %}
{% endfor %}
{% endif %}
