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
   ~{{ name }}.{{ item }}
{% endfor %}
{% endif %}

{% if methods %}
{{ _('Methods') | escape | underline('-') }}

.. autosummary::
   :nosignatures:
{% for item in methods %}
   ~{{ name }}.{{ item }}
{% endfor %}
{% endif %}

{# === DETALLE (caja grande con docstrings) === #}
{% if attributes %}
{{ _('Attribute reference') | escape | underline('~') }}

{% for item in attributes %}
.. autoattribute:: {{ name }}.{{ item }}

{% endfor %}
{% endif %}

{% if methods %}
{{ _('Method reference') | escape | underline('~') }}

{% for item in methods %}
.. automethod:: {{ name }}.{{ item }}

{% endfor %}
{% endif %}
