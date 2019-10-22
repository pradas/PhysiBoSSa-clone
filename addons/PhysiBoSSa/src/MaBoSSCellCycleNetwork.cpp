#include "MaBoSSCellCycleNetwork.h"


CellCycleNetwork::CellCycleNetwork()
{
	this->time_to_update = 0;
	this->maboss = nullptr;
}

CellCycleNetwork::CellCycleNetwork( MaBoSSNetwork* mboss )
{
	this->set_maboss( mboss );
	this->time_to_update = ( 1 + 0.5*UniformRandom11() ) * maboss->update_time_step();
}

CellCycleNetwork::~CellCycleNetwork()
{
	// network will be freed from Simul class, here just pointer
	if ( maboss )
		maboss = nullptr;
}


/* Copy all parameters specific to cell cycle network object */
void CellCycleNetwork::copy_cycle( CellCycleNetwork* copy_me)
{
	maboss = copy_me->maboss;
	set_time_to_update();
	nodes = copy_me->nodes;
	networkState = copy_me->networkState;
}

/* Initialization: set network */
void CellCycleNetwork::set_maboss( MaBoSSNetwork* mboss )
{
	maboss = mboss;
	set_time_to_update();
	// initialize all nodes to 0
	nodes.resize( maboss->number_of_nodes() );
	maboss->initNetworkState();
	maboss->set_default( &nodes );
}

/* random update time, to asynchronize it between all cells */
void CellCycleNetwork::set_time_to_update()
{
	time_to_update = (1 + 0.5*UniformRandom11()) * maboss->update_time_step();
}

void CellCycleNetwork::randomize_nodes()
{
	int which;
	for ( int i = 0; i < (int) nodes.size()/10; i++ )
	{
		 which = (int) (UniformRandom() * nodes.size());
		nodes[which] = (UniformRandom() < 0.5);
	}
}

/* For a given node name, test if there are enough protein of given field 
 *
 * If the field is not present on the microenv, don't set anthing, then initial value from BN conf files is used */
void CellCycleNetwork::enough_to_node( std::string nody, std::string field )
{
	int ind = maboss->get_node_index( nody );
	if ( ind >= 0 )
	{
		int felt = mycell->feel_enough(field) ;
		if ( felt != -1 )
			nodes[ind] = felt;
	}
}

/* Set inputs of the network from current cell state */
void CellCycleNetwork::set_input_nodes()
{
	int ind;

	// Oxygen input node O2; Oxygen or Oxy
	ind = maboss->get_node_index( "Oxygen" );
	if ( ind < 0 )
		ind = maboss->get_node_index( "Oxy" );
	if ( ind < 0 )
		ind = maboss->get_node_index( "O2" );
	if ( ind >= 0 )
		nodes[ind] = ( !mycell->necrotic_oxygen() );
	

	ind = maboss->get_node_index( "Neighbours" );
	if ( ind >= 0 )
		nodes[ind] = ( mycell->has_neighbor(0) );
	
	ind = maboss->get_node_index( "Nei2" );
	if ( ind >= 0 )
		nodes[ind] = ( mycell->has_neighbor(1) );

	enough_to_node( "Glucose", "glucose" );
	enough_to_node( "EGF", "egf" );
	enough_to_node( "IGF", "igf" );
	enough_to_node( "TNF", "tnf" );
	enough_to_node( "GF", "gf" );
	enough_to_node( "TGFbeta", "tgfb" );

	// If has enough contact with ecm or not
	ind = maboss->get_node_index( "ECM_sensing" );
	if ( ind >= 0 )
		nodes[ind] = ( mycell->touch_ECM() );
	// If has enough contact with ecm or not
	ind = maboss->get_node_index( "ECM" );
	if ( ind >= 0 )
		nodes[ind] = ( mycell->touch_ECM() );
	// If has enough contact with ecm or not
	ind = maboss->get_node_index( "ECMicroenv" );
	if ( ind >= 0 )
		nodes[ind] = ( mycell->touch_ECM() );
	
	// If nucleus is deformed, probability of damage
	// Change to increase proba with deformation ? + put as parameter
	ind = maboss->get_node_index( "DNAdamage" );
	//std::cout << mycell->nucleus_deformation() << std::endl;
	if ( ind >= 0 )
		nodes[ind] = ( mycell->nucleus_deformation() > 0.5 ) ? (2*UniformRandom() < mycell->nucleus_deformation()) : 0;

}

/* Update MaboSS network states */
void CellCycleNetwork::run_maboss()
{
	set_input_nodes();
	bool converged;
	#pragma omp critical
	{
		converged =	maboss->run( &networkState, &nodes, mycell->get_cell_line() );
	}

	//if ( !converged )
	//	randomize_nodes();
	//from_nodes_to_cell(dt, t);
	
	set_time_to_update();
}

/* Read the value of output BN nodes and transfer action to the cell */
void CellCycleNetwork::from_nodes_to_cell(double dt, double t)
{

	if ( this->maboss->get_model_id() == "AGS" )
	{
		std::string prosurvival_basename = "Prosurvival_b";
		std::string antisurvival_basename = "Antisurvival_b";
		int prosurvival_value = 0;
		int antisurvival_value = 0;

		int index;
		for(int i=1; i<=3; i++)
		{
			index = this->maboss->get_node_index( prosurvival_basename + std::to_string(i) );
			if ( index >= 0)
			{
				prosurvival_value += this->nodes[index];
			}
			index = this->maboss->get_node_index( antisurvival_basename + std::to_string(i) );
			if ( index >= 0)
			{
				antisurvival_value += this->nodes[index];
			}
		}

		mycell->freezing(0);

		if ( prosurvival_value > 0 )
			this->do_proliferation( dt );

		else if ( antisurvival_value > 0 )
		{
			this->set_current_phase( PhysiCell_constants::necrotic_swelling );
			this->start_necrosis_swelling(dt);
			return;
		}

		else if ( prosurvival_value == 0 )
		{
			this->mycell->freezing(1);
		}

	}
	else if ( this->maboss->get_model_id() == "TNF" )
	{
		// Die if not enough oxygen, BN output Hypoxia or Necrosis or Autophagy (not enough glucose)
		int ind;
		int necr = 0;

		ind = this->maboss->get_node_index( "Hypoxia" );
		if ( ind >= 0 )
			necr += this->nodes[ind];

		ind = this->maboss->get_node_index( "Necrosis" );
		if ( ind >= 0 )
			necr += this->nodes[ind];

		ind = this->maboss->get_node_index( "Autophagy" );
		if ( ind >= 0 )
			necr += this->nodes[ind];

		if ( necr )
		{
			this->set_current_phase( PhysiCell_constants::necrotic_swelling );
			this->start_necrosis_swelling(dt);
			return;
		}

		ind = this->maboss->get_node_index( "Migration" );
		if ( ind >= 0 )
			this->mycell->evolve_motility_coef( nodes[ind], dt );

		ind = this->maboss->get_node_index( "Polarization" );
		if ( ind >= 0 )
			this->mycell->evolve_polarity_coef( nodes[ind], dt );

		ind = this->maboss->get_node_index( "Cell_cell" );
		if ( ind >= 0 )
			this->mycell->evolve_cellcell_coef( nodes[ind], dt );

		ind = this->maboss->get_node_index( "Matrix_adhesion" );
		if ( ind >= 0 )
			this->mycell->evolve_integrin_coef( nodes[ind], dt );

		ind = this->maboss->get_node_index( "Matrix_modif" );
		if ( ind >= 0 )
			this->mycell->set_mmp( nodes[ind] );

		ind = this->maboss->get_node_index( "EMT" );
		if ( ind >= 0 )
			this->mycell->set_mmp( nodes[ind] );

	

		// Proliferation: cell going into cycling and growing
		int pro = 0;
		ind = this->maboss->get_node_index( "Proliferation" );
		if ( ind >= 0 )
			pro += nodes[ind];

		ind = this->maboss->get_node_index( "Cell_growth" );
		if ( ind >= 0 )
			pro += nodes[ind];

		ind = this->maboss->get_node_index( "Survival" );
		if ( ind >= 0 )
			pro += nodes[ind];

		if ( pro > 0 )
			this->do_proliferation( dt );

		this->mycell->freezing( 0 );
		ind = this->maboss->get_node_index( "Quiescence" );
		if ( ind >= 0 && this->nodes[ind] )
			this->mycell->freezing(1);

		ind = this->maboss->get_node_index( "CCA" );
		if ( ind >= 0 && this->nodes[ind] )
			this->mycell->freezing(1);

		ind = this->maboss->get_node_index( "Cell_freeze" );
		if ( ind >= 0 )
			this->mycell->freezer( 3 * this->nodes[ind] ); // 3 for bitwise operation
	
		ind = this->maboss->get_node_index( "Apoptosis" );
		if ( ind >= 0 && this->nodes[ind] )
		{
			this->set_current_phase( PhysiCell_constants::apoptotic );
			this->start_apoptosis();
			return;
		}

		ind = this->maboss->get_node_index( "NonACD" );
		if ( ind >= 0 && this->nodes[ind] )
		{
			this->set_current_phase( PhysiCell_constants::necrotic_swelling );
			this->start_necrosis_swelling(dt);
			return;
		}
	
		// For model with TNF production
		ind = this->maboss->get_node_index( "NFkB" );
		if ( ind >= 0 )
		{
			// produce some TNF
			if ( this->nodes[ind] )
			{
				this->mycell->secrete("tnf", 1, dt);
				if ( this->got_activated == 0 )
					this->got_activated = t;
			}
			else
				this->mycell->secrete("tnf", 0, dt);
		}
	}
	else
	{
		std::cout << "======================================================================================="    << std::endl;
		std::cout << " WARNING!!!! bn_model_id not defined in XML configuration file. Impossible to map nodes" << std::endl;
		std::cout << "======================================================================================="  << std::endl;
	}
}