"""
Module to deploy cluster
"""

class SLURMJob():
    """ Class to create and launch a SLURM Jobsript """

    _script_template = []

    def __init__(self,parameter_dict=None,cores=None,walltime=None, mem_per_cpu=None, job_name=None):
        self.cores=cores; self.walltime=walltime; self.mem_per_cpe=mem_per_cpu; self.job_name=job_name

        if parameter_dict is not None:
            self.cores=parameter_dict["cores"]; self.walltime=parameter_dict["walltime"]; self.mem_per_cpu=parameter_dict["mem_per_cpu"]
            self.job_name=parameter_dict["job_name"]

        print(f"Me he creado con {cores} cores y {walltime} tiempo")
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
            if self.job_name is None:
                script_name="myjob"
            elif self.job_name is not None:
                script_name=self.job_name
        with open('./'+script_name+'.sh', 'w') as archivo:
            archivo.write(self.job_header)
            archivo.write(self.conda_env)
            archivo.write("\n\n\n"+"module load qmio/hpc gcc/12.3.0 impi/2021.13.0")
        print("Script generado")


class Cluster(SLURMJob):
	
	def __init__(self):
		pass

	def deploy(self, config_dict=0):
		
		print("Levantamos el cluster usando el fichero de config")

	def is_deployed(self):
	
		print("El cluster esta levantado = True, en otro caso = False")

	def info(self):
		
		print("Informacion sobre los recursos levantados")

