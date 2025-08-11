{{ fullname | escape | underline }}

.. currentmodule:: {{ module }}

.. autoclass:: {{ objname }}
   :show-inheritance:
   :members:
   :member-order: bysource
   :private-members:         # no, para ocultar privados
   :exclude-members: __weakref__, __dict__, __init_subclass__
   :autosummary:             # tablas automáticas encima del detalle
   :autosummary-sections: Attributes ;; Methods
   :autosummary-no-titles:
   :autosummary-nosignatures:

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
{%- endif %}
{% endfor %}
{% endif %}
