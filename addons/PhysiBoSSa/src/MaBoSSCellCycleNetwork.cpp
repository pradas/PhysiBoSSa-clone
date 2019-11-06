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

/* Update MaboSS network states */
void CellCycleNetwork::run_maboss()
{
	//#pragma omp critical
	{
		maboss->run(&networkState, &nodes);
	}
	
	set_time_to_update();
}
