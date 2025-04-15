Module Set Up
=============



Cloning the repository 
------------------------
GitHub provides different options for cloning a repository, which can be checked in the dropdown menu of the green "<> Code" buttom. 
For ensuring a correct cloning of the repository, the SSH is the one preferred. This can be achieved by running the following on your terminal:

.. code-block:: bash

    eval "$(ssh-agent -s)"
    ssh-add ~/.ssh/SSH_KEY
We are set to retrieve the source code:

.. code-block:: bash

    git clone --recursive git@github.com:CESGA-Quantum-Spain/cunqa.git

The ``--recursive`` option ensures that all submodules are correctly loaded when cloning. As a additional step, we encourage to run the setup_submodules.sh file, which removes some of the files unused in the submodules and makes the repository lighter:

.. code-block:: bash

    cd cunqa/scripts
    bash setup_submodules.sh


Installation
--------------------------
blablabla isntall




