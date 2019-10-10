#ifndef __PhysiBoSSa_custom_models_h__
#define __PhysiBoSSa_custom_models_h__

#include "../../../core/PhysiCell_constants.h" 
#include "../../../core/PhysiCell_phenotype.h" 

namespace PhysiCell
{

// custom cycle models: 
extern Cycle_Model PhysiBoSSa_TNF, PhysiBoSSa_AGS;

// create custom models 
bool create_PhysiBoSSa_cycle_models( void );
	
};

#endif 
