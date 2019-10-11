#include "MaBoSSCycleModels.h" 
#include "MaBoSSNetwork.h"
#include "../../../core/PhysiCell_cell.h" 

namespace PhysiCell{
	
Cycle_Model PhysiBoSSa_TNF, PhysiBoSSa_AGS;

void tnf_cycling_entry_function( Cell* pCell, Phenotype& phenotype, double dt )
{
	// the cell wants to double its volume 
	phenotype.volume.target_solid_nuclear *= 2.0; 
	phenotype.volume.target_solid_cytoplasmic *= 2.0; 	
	
	return; 
}

/* create standard models */ 

void create_tnf_cycle_model( void )
{
	MaBoSSNetwork* maboss = new MaBoSSNetwork();
	maboss->init("../BN/TNF/TNF_nodes.bnd","../BN/TNF/TNF_conf.cfg");

	PhysiBoSSa_TNF.code = PhysiCell_constants::custom_cycle_model; 
	PhysiBoSSa_TNF.name = "MaBoSS TNF"; 
	
	PhysiBoSSa_TNF.data.time_units = "min"; 
	
	PhysiBoSSa_TNF.add_phase( PhysiCell_constants::live , "Live" ); 
	
	PhysiBoSSa_TNF.phases[0].division_at_phase_exit = true; 
	
	PhysiBoSSa_TNF.add_phase_link( 0 , 0 , NULL ); 
	
	PhysiBoSSa_TNF.transition_rate(0,0) = 0.0432 / 60.0; // MCF10A have ~0.04 1/hr net birth rate
	
	PhysiBoSSa_TNF.phases[0].entry_function = tnf_cycling_entry_function;
		
	return; 
}

bool create_PhysiBoSSa_cycle_models( void )
{
	create_tnf_cycle_model();
	return true; 
}

};