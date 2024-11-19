git submodule add git@github.com:Qiskit/qiskit-aer.git ../src/third-party/aer-cpp 
cd ../src/third-party/aer-cpp
git config core.sparsecheckout true 
echo src/ >> ../.git/modules/aer-cpp/info/sparse-checkout 
git read-tree -mu HEAD 
git add -A
git commit -m 'add qiskit aer cpp as submodule/sparse-checkout' 

#!/bin/bash

# Script para configurar sparse checkout en submódulos

# Definir los submódulos y sus rutas de sparse checkout
declare -A submodule_sparse
submodule_sparse["aer-cpp"]="../src/third-party/aer-cpp"

# Iterar sobre cada submódulo y configurar sparse checkout
for submodule in "${!submodule_sparse[@]}"; do
    sparse_paths=${submodule_sparse[$submodule]}

    # Definir la ruta del archivo sparse-checkout del submódulo
    sparse_file="../.git/modules/$submodule/info/sparse-checkout"

    # Habilitar sparse checkout en el submódulo
    git -C "../$submodule" config core.sparseCheckout true

    # Añadir las rutas especificadas al archivo sparse-checkout
    for path in $sparse_paths; do
        echo "$path" >> "$sparse_file"
    done

    # Aplicar la configuración de sparse checkout
    git -C "../$submodule" read-tree -mu HEAD
done

echo "Sparse checkout configurado para los submódulos."