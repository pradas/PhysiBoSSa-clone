#include "MaBoSSNetwork.h"

/* Default constructor */
MaBoSSNetwork::MaBoSSNetwork( std::string networkFile, std::string configFile )
{
	// This is needed for the external functions
	MaBEstEngine::init();
	this->network = new Network();
	this->runConfig = RunConfig::getInstance();
	this->runConfig->setSeedPseudoRandom( UniformInt() );
	this->symbTable = SymbolTable::getInstance();
	this->update_time = 10;
	this->def_nodes.clear();

	this->network->parse(networkFile.c_str());
	this->runConfig->parse(this->network, configFile.c_str());
	
	// This was missing, for some reason
	this->network->updateRandomGenerator(RunConfig::getInstance());

	this->initNetworkState();
}

/* Default destructor */
MaBoSSNetwork::~MaBoSSNetwork()
{
	delete this->network;
	delete this->runConfig;
	delete this->symbTable;

	this->network = NULL;
	this->runConfig = NULL;
	this->symbTable = NULL;
}

/* Initialize the state of the network */
void MaBoSSNetwork::initNetworkState()
{
	NetworkState initial_state;
	this->network->initStates(initial_state);
	std::vector<Node *> nodes = this->network->getNodes();
	int i = 0;
	this->def_nodes.resize( nodes.size() );
	for (auto node : nodes)
	{
		this->node_names[ node->getLabel() ] = i;
		this->def_nodes[i] = initial_state.getNodeState(node);
		i++;	
	}
}

/* Set values of nodes to default values */
void MaBoSSNetwork::set_default( std::vector<bool>* nodes )
{
	for ( int i = 0; i < (int)this->def_nodes.size(); i++ )
	{
		(*nodes)[i] = this->def_nodes[i];
	}
}

/* Load previous network states and inputs */
void MaBoSSNetwork::load( NetworkState* netState, std::vector<bool>* inputs )
{
	std::vector<Node*> nodes;
	nodes = network->getNodes();
	int ind = 0;
	for (auto node : nodes)
	{
		netState->setNodeState(node, (NodeState) (*inputs)[ind]);
		ind ++;
	}

	// IStateGroup::reset();
	IStateGroup::setInitialState(network, netState);
}

/* Run the current network */
void MaBoSSNetwork::run(std::vector<bool>* nodes_val)
{
	this->runConfig->setSeedPseudoRandom( UniformInt() ); // pick random number

	// Load network state and values of current cell in the network instance
	//loadSymbol( cellline );
	NetworkState netStates;
	load( &netStates, nodes_val );
	MaBEstEngine mabossEngine( this->network, this->runConfig );
	// No output from MaBoSS run
	std::ostream* os = NULL; 
	// And run it
	mabossEngine.run(os);
		
	// save fixed point as initial state for the network for the next time step
	const STATE_MAP<NetworkState_Impl, double>& states = mabossEngine.getAsymptoticStateDist();
	if (states.begin() != states.end()) 
	{
		netStates = states.begin()->first;
	}

	int i = 0;
	std::vector<Node*> nodes = this->network->getNodes();
	for ( auto node: nodes )
	{
		(*nodes_val)[i] = netStates.getNodeState( node ) ;
		//std::cout << node->getLabel() << " " << (*nodes_val)[i] << std::endl;
		i ++;
	}
}

/* Print current state of all the nodes of the network */
void MaBoSSNetwork::printNodes(NetworkState* netStates)
{
	std::vector<Node*> nodes = network->getNodes();
	for ( auto node: nodes )
	{
		std::cout << node->getLabel() << "=" << netStates->getNodeState(node) << "; ";
	}
	std::cout << std::endl;
}

/* Return the index of node based on node's name */
int MaBoSSNetwork::get_node_index( std::string name )
{
	auto res = this->node_names.find(name);
	if ( res != this->node_names.end() )
		return res->second;
	return -1;	
}




