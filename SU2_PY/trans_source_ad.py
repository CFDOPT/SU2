import SU2
from SU2.util.pyCppTap import *
import os.path
# Script to quickly convert the transition model source residual and perform AD

# Convert C++ to C
source_routine   = 'CSourcePieceWise_TransLM::SetResidual_TransLM'
source_directory = '/home/aniket/su2/trunk/SU2_CFD/src'
source_location  = source_directory + '/numerics_source.cpp'

convert_cpp2c(source_routine, source_location, overwrite=True)

# Differentiate C routine
routine_name = 'CSourcePieceWise_TransLM__SetResidual_TransLM'
invars  = ['TransVar_i']
outvars = ['val_residual']

croutinesdir = '/home/aniket/su2/trunk/SU2_CFD/src/c_routines'
file_list = [os.path.join(croutinesdir, 'CSourcePieceWise_TransLM__SetResidual_TransLM.c')]
file_list.append(os.path.join(croutinesdir, 'fabs.c'))
file_list.append(os.path.join(croutinesdir, 'pow.c'))

diff_routine(routine_name, invars, outvars, file_list)

# Convert differentiated C routine back to C++
convert_c2cpp(routine_name, overwrite=True)
