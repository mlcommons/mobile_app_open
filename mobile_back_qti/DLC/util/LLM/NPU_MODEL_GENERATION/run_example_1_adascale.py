import papermill as pm
import os, sys
import nbformat
import re

model_name = sys.argv[1] if len(sys.argv) > 1 else "llama3.1"

if model_name != "llama3.1":
    print(f"Model not supported: {model_name}. Only 'llama3.1' is supported.")
    sys.exit(1)

print("#=====================================Setting up qairt sdk path============================================#")
qairt_path = '../qairt'
if not os.path.exists(qairt_path):
    print(f"Qualcomm's QAIRT SDK is not present at {qairt_path}, please ensure that it is present.")
    sys.exit(1)
qairt_path = os.path.abspath(qairt_path)
qairt_version_path = os.listdir(qairt_path)[0]
print(f"Qairt Version Used: {qairt_version_path}")
QNN_SDK_ROOT = os.path.join(qairt_path, qairt_version_path) + "/"
model_id = os.path.abspath(os.path.join(qairt_path, "..", "Llama-3.1-8B-Instruct")) + "/"

print(f"model_id: {model_id}")

parameters = {"model_id": model_id}


def modify_notebook(notebook_path, output_path, variable_changes):
    with open(notebook_path, "r", encoding="utf-8") as f:
        nb = nbformat.read(f, as_version=4)

    for cell in nb.cells:
        if cell.cell_type == "code" and isinstance(cell.source, str):
            for var_name, new_value in variable_changes.items():
                if var_name == "model_id":
                    pattern = r'^(?P<prefix>\s*model_id\s*=\s*)(?P<q>["\'])(?P<val>.*?)(?P=q)\s*$'
                replacement = f"{var_name} = {repr(new_value)}"
                cell.source = re.sub(pattern, replacement, cell.source, flags=re.MULTILINE)

    with open(output_path, "w", encoding="utf-8") as f:
        nbformat.write(nb, f)


modify_notebook("adascale.ipynb", "updated-adascale.ipynb", parameters)
pm.execute_notebook(
    "updated-adascale.ipynb",
    "output-adascale.ipynb",
    kernel_name="python"
)
