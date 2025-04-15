import re
import sys
from pathlib import Path

# Mapping GitHub-style to MyST-style
ADMONITION_MAP = {
    "NOTE": "note",
    "WARNING": "warning",
    "TIP": "tip",
    "IMPORTANT": "important",
    "CAUTION": "caution",
    "INFO": "info",
}

def convert_gfm_admonitions(text):
    lines = text.splitlines()
    output = []
    in_admonition = False
    indent = ""
    for i, line in enumerate(lines):
        admonition_match = re.match(r'^>\s*\[!(\w+)\]\s*$', line.strip(), re.IGNORECASE)
        if admonition_match:
            # Start of an admonition block
            kind = admonition_match.group(1).upper()
            myst_kind = ADMONITION_MAP.get(kind, kind.lower())
            in_admonition = True
            indent = "    "  # MyST requires 4-space indent inside admonition
            output.append(f"!!! {myst_kind}")
        elif in_admonition:
            if line.strip().startswith(">"):
                # Strip `>` and keep indentation
                content = line.strip()[1:].lstrip()
                output.append(f"{indent}{content}")
            else:
                # End of the block
                output.append(line)
                in_admonition = False
        else:
            output.append(line)

    return "\n".join(output)


def convert_file(filepath):
    path = Path(filepath)
    if not path.exists():
        print(f"File not found: {filepath}")
        return

    original = path.read_text(encoding="utf-8")
    converted = convert_gfm_admonitions(original)
    path.write_text(converted, encoding="utf-8")
    print(f"Converted: {filepath}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python convert_admonitions.py <file.md>")
    else:
        convert_file(sys.argv[1])
