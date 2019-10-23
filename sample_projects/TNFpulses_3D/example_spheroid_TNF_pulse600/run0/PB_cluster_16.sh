#!/usr/bin/env bash

##ADD YOUR SBATCH HEADERS BELOW!!!##
#SBATCH -n 16
#SBATCH -o %j.out
#SBATCH -e %j.err
#SBATCH -J "PB_16n"
#SBATCH -t 00:20:00
#SBATCH --qos debug

# module load python/3-intel-2018.2
# module load R/3.5.1
# module load perl/5.26
# module load java/latest

SECONDS=0
projectname=TNF_pulse600_16
echo "$projectname"
# cd $projectname/run0/
../../../bin/PhysiBoSS -p parameters.xml

# sed "s/projectname/"$projectname"/g" run_MaBoSS_Unix_Stew.sh >run_MaBoSS_Unix_Stew_post.sh
# chmod 755 run_MaBoSS_Unix_Stew_post.sh
# rm -r ./"$projectname"
# ./run_MaBoSS_Unix_Stew_post.sh
# rm -f ./projectname/*_statdist* ./projectname/*_probtraj.csv ./projectname/*_run.txt
# rm -f *_fp.csv *_probtraj_table.csv

duration=$SECONDS
echo "$projectname : $(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
