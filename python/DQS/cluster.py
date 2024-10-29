"""
Module to deploy cluster
"""

class SLURMJob():
    """ Class to create and launch a SLURM Jobsript """

    _script_template = []

    def __init__(self,parameter_dict=None,cores=None,walltime=None, mem_per_cpu=None, job_name="myjob",disp=True):
        keys=["cores","walltime","mem_per_cpu","job_name"]
        if parameter_dict is not None:
            for k in keys:
                if k in parameter_dict.keys():
                    setattr(self,k,parameter_dict[k])
        for k in keys:
            if locals()[k] is not None:
                setattr(self,k,locals()[k])
        
        if hasattr(self, "cores") is False or hasattr(self, "walltime")is False or hasattr(self, "mem_per_cpu") is False:
            raise ValueError("You must at least specify cores, walltime and mem_per_cpu.")
         
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
            header_lines.append(f"#SBATCH -J {job_name} \t\t # jobname")
            header_lines.append(f"#SBATCH -o {job_name}_%j.out \t # output file")
            header_lines.append(f"#SBATCH -e {job_name}_%j.err \t # error file")
        elif job_name is None:
            header_lines.append("#SBATCH -J {} \t\t # jobname".format("myjob"))
            header_lines.append("#SBATCH -o {}_%j.out \t # output file".format("myjob"))
            header_lines.append("#SBATCH -e {}_%j.err \t # error file".format("myjob"))

        header_lines.append("#SBATCH -n 1")

        if cores is not None:
            header_lines.append(f"#SBATCH -c {cores} \t\t\t\t # number of cores")
        if walltime is not None:
            header_lines.append(f"#SBATCH -t {walltime} \t\t # time for the job")
        if mem_per_cpu is not None:
            header_lines.append(f"#SBATCH --mem-per-cpu={mem_per_cpu} \t # memory per core")
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
        self.conda_env="\n\n\n" + conda_specifications
    
    def generate_script(self,script_name=None):
        if script_name is None:
            self.script_name=self.job_name # por defecto, si no lo definimos al instaciar la clase, toma "myjob"
        else:
            self.script_name=script_name
        with open('./'+self.script_name+'.sh', 'w') as archivo:
            archivo.write(self.job_header)
            archivo.write(self.conda_env)
            archivo.write("\n\n\n"+"module load qmio/hpc gcc/12.3.0 impi/2021.13.0")
        print(f"Job script was generated with name {self.script_name}.sh")

    def job_script():
        try:
            print(f"Job script {self.script_name}.sh :")
            print("-------------------------------------------")
            with open('./'+self.script_name+'.sh', 'r') as archivo:
                print(load(archivo))
        except:
            raise FileNotFoundError("Make sure you already generated the job script using .generate_script() method before trying to print it.")
        


class Cluster(SLURMJob):
	
	def __init__(self):
		pass

	def deploy(self, config_dict=0):
		
		print("Levantamos el cluster usando el fichero de config")

	def is_deployed(self):
	
		print("El cluster esta levantado = True, en otro caso = False")

	def info(self):
		
		print("Informacion sobre los recursos levantados")

