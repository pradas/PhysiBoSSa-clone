#include "MaBoSSCellCycleNetwork.h"

CellCycleNetwork::CellCycleNetwork( MaBoSSNetwork* maboss )
{
	this->set_maboss( maboss );
	this->time_to_update = ( 1 + 0.5*UniformRandom11() ) * maboss->update_time_step();
}

CellCycleNetwork::~CellCycleNetwork()
{
	delete this->maboss;
	this->maboss = NULL;
}

/* Initialization: set network */
void CellCycleNetwork::set_maboss( MaBoSSNetwork* maboss )
{
	this->maboss = maboss;
	this->set_time_to_update();

	// initialize all nodes to 0
	this->nodes.resize( maboss->number_of_nodes() );
	this->maboss->set_default( &this->nodes );
}

/* random update time, to asynchronize it between all cells */
void CellCycleNetwork::set_time_to_update()
{
	this->time_to_update = (1 + 0.5*UniformRandom11()) * this->maboss->update_time_step();
}

/* Update MaboSS network states */
void CellCycleNetwork::run_maboss()
{
	this->maboss->run(&this->nodes);
	this->set_time_to_update();
}
