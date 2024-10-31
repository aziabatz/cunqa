"""
Module to deploy cluster
"""

class SLURMJob():
    """ Class to create and launch a SLURM Jobsript """

    _script_template = []

    def __init__(self,parameter_dict=None,cores=None,ntasks=None,walltime=None, mem_per_cpu=None, job_name="myjob",disp=True):
        keys=["cores","ntasks","walltime","mem_per_cpu","job_name"]
        if parameter_dict is not None:
            for k in keys:
                if k in parameter_dict.keys():
                    setattr(self,k,parameter_dict[k])
        for k in keys:
            if locals()[k] is not None:
                setattr(self,k,locals()[k])
        

        # cores, walltime and memory are needed. Maybe we can set default values
        if hasattr(self, "cores") is False or hasattr(self, "walltime")is False or hasattr(self, "mem_per_cpu") is False:
            raise ValueError("You must at least specify cores, walltime and mem_per_cpu.")
        
        # ntasks have a default 1. 
        if hasattr(self,"ntasks") is False:
            setattr(self,"ntasks",1)

        if disp is True:
            cadena="Job object crated with: "
            for k in keys:
                try:
                    cadena+="\n"+k+": "+str(getattr(self, k))
                except:
                    pass# en caso de que no se haya definido alguna característica del SLURM, este la asigna por defecto, no la api
            print(cadena)

        header_lines=[]; header_lines.append("#!/bin/bash")

        if job_name is not None:
            header_lines.append(f"#SBATCH -J {self.job_name} \t\t # jobname")
            header_lines.append(f"#SBATCH -o {self.job_name}_%j.out \t # output file")
            header_lines.append(f"#SBATCH -e {self.job_name}_%j.err \t # error file")
        elif job_name is None:
            header_lines.append("#SBATCH -J {} \t\t # jobname".format("myjob"))
            header_lines.append("#SBATCH -o {}_%j.out \t # output file".format("myjob"))
            header_lines.append("#SBATCH -e {}_%j.err \t # error file".format("myjob"))

        if self.cores is not None:
            header_lines.append(f"#SBATCH -c {self.cores} \t\t\t # number of cores")
        if self.ntasks is not None:
            header_lines.append(f"#SBATCH -n {self.ntasks} \t\t\t # number of tasks")
        else:
            header_lines.append(f"#SBATCH -n 1 \t\t\t # number of tasks")
        if self.walltime is not None:
            header_lines.append(f"#SBATCH -t {self.walltime} \t\t # time for the job")
        if self.mem_per_cpu is not None:
            header_lines.append(f"#SBATCH --mem-per-cpu={self.mem_per_cpu} \t # memory per core")
        self.job_header = "\n".join(header_lines)
      
    def set_conda_env(self,name=None):
        """ To set the conda env.
        Parameters
        --------------
        name: str
            Name of the environment.
            """
        if name is not None:
            conda_specifications=f"conda activate {name}"
        self.conda_env="\n\n\n# loading conda env...\n" +"module load qmio/hpc miniconda3/22.11.1-1\n" + conda_specifications
    
    def generate_script(self,script_name=None):
        if script_name is None:
            self.script_name=self.job_name # por defecto, si no lo definimos al instaciar la clase, toma "myjob"
        else:
            self.script_name=script_name
        with open('./'+self.script_name+'.sh', 'w') as archivo:
            archivo.write(self.job_header)
            try:
                archivo.write(self.conda_env)
            except:
                pass
            archivo.write("\n\n\n# loading impi...\n"+"module load qmio/hpc gcc/12.3.0 impi/2021.13.0\n\n\n")
            archivo.write("""echo Soy la RESERVA: SLURM_NTASKS: $SLURM_NTASKS \necho Soy la RESERVA: SLURM_CPUS_PER_TASK: $SLURM_CPUS_PER_TASK """)
            archivo.write("\nsrun bash hola.sh")
        print(f"Job script was generated with name {self.script_name}.sh")

    def job_script(self):
        try:
            print(f"Job script {self.script_name}.sh :")
            print("-------------------------------------------")
            with open('./'+self.script_name+'.sh', 'r') as archivo:
                print(archivo.read())
            print("-------------------------------------------")
        except:
            raise FileNotFoundError("Make sure you already generated the job script using .generate_script() method before trying to print it.")
        
    def launch(self):
        from os import system
        command="sbatch "+self.script_name+".sh"
        system(command)







class Cluster(SLURMJob):
    """ Clase que levanta Cluster de QPUS """
    def __init__(self):
        return

    def deploy(self, config_dict=0):
        print("Levantamos el cluster usando el fichero de config")

    def is_deployed(self):
        print("El cluster esta levantado = True, en otro caso = False")
    
    def info(self):
        print("Informacion sobre los recursos levantados")

