#include "maboss_network.h"
#include "utils.h"

/* Default constructor */
MaBoSSNetwork::MaBoSSNetwork()
{
	// This is needed for the external functions
	MaBEstEngine::init();
	this->network = new Network();
	this->runConfig = RunConfig::getInstance();
	this->runConfig->setSeedPseudoRandom( UniformInt() );
	this->symbTable = SymbolTable::getInstance();
	this->update_time = 10;
	this->def_nodes.clear();
	this->conffile = "";
	this->model_id = "";
}

/* Default destructor */
MaBoSSNetwork::~MaBoSSNetwork()
{
	delete network;
}

/* Set the name/id of the boolean model */
void MaBoSSNetwork::set_model_id(std::string id)
{
	this->model_id = id;
}

/* Get the name/id of the boolean model */
std::string MaBoSSNetwork::get_model_id()
{
	return this->model_id;
}

// TODO
/* Read properties from XML parameter file for network */
/*void MaBossNetwork::read_properties( ReadXML* reader, std::string what )
{
	reader->getDoubleValue( what, "network_update_step", &update_time );
	
	// Read mutations
	int left = 1; // if has still some mutation to read
	int next = 0; // number of mutation read so far
	std::string sname = "";
	double rate = 1;
	int cl = 0;
	while ( left )
	{
		left = reader->getStringValue( what, "symbol_name", &sname, "mutation_"+std::to_string(next) );
		left *= reader->getDoubleValue( what, "rate", &rate, "mutation_"+std::to_string(next) );
		left *= reader->getIntValue( what, "cell_line", &cl, "mutation_"+std::to_string(next) );
		
		// has succeed to read one other mutation, add it in the map
		if ( left )
		{
			strip(&sname);
			mbRates[std::pair<std::string, int>(sname, cl)] = rate;
			next ++;
		}
	}
}*/

/* Initialize the network by reading input files */
void MaBoSSNetwork::init( std::string networkFile, std::string configFile )
{
	network->parse(networkFile.c_str());
	conffile = configFile;
	runConfig->parse(network, conffile.c_str());
	
	// This was missing, for some reason
	network->updateRandomGenerator(RunConfig::getInstance());
	
	initNetworkState();
}

/* Initialize the state of the network */
void MaBoSSNetwork::initNetworkState()
{
	NetworkState initial_state;
	network->initStates(initial_state);
	std::vector<Node *> nodes = network->getNodes();
	int i = 0;
	def_nodes.resize( nodes.size() );
	for (auto node : nodes)
	{
		node_names[ node->getLabel() ] = i;
		def_nodes[i] = initial_state.getNodeState(node);
		i++;	
	}
}

/* Set values of nodes to default values */
void MaBoSSNetwork::set_default( std::vector<bool>* nodes )
{
	for ( int i = 0; i < (int) def_nodes.size(); i++ )
	{
		(*nodes)[i] = def_nodes[i];
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

/* Load symbol rates value if they are in the map for the current cell line, useful for mutations */
void MaBoSSNetwork::loadSymbol( int cellline )
{
	for (auto symit = mbRates.begin(); symit != mbRates.end(); symit++ )
	{
		// current cell line, keep it
		if ( symit->first.second == cellline )
		{
			const Symbol* symb = symbTable->getSymbol( symit->first.first );
			if ( symb != NULL )
			{
				symbTable->overrideSymbolValue( symb, symit->second );
				//std::cout << symit->second << " " << symit->first.first << " " << symit->first.second << std::endl;
			}
		}

	}
}

/* Run the current network */
bool MaBoSSNetwork::run( NetworkState* netStates, std::vector<bool>* nodes_val, int cellline )
{
	runConfig->setSeedPseudoRandom( UniformInt() ); // pick random number

	// Load network state and values of current cell in the network instance
	//loadSymbol( cellline );
	load( netStates, nodes_val );
	MaBEstEngine mabossEngine( network, runConfig );
	// No output from MaBoSS run
	std::ostream* os = NULL; 
	// And run it
	mabossEngine.run(os);
		
	// save fixed point as initial state for the network for the next time step
	const STATE_MAP<NetworkState_Impl, double>& states = mabossEngine.getAsymptoticStateDist();
	if (states.begin() != states.end()) 
	{
		(*netStates) = states.begin()->first;
	}
	bool converged = true;	
	/**if ( ! mabossEngine.converges() )
	{
		std::cerr << "WARNING: Network did not converge;" << std::endl;
//		converged = false;
	}*/

	int i = 0;
	std::vector<Node*> node3 = network->getNodes();
	for ( auto node: node3 )
	{
		(*nodes_val)[i] = netStates->getNodeState( node ) ;
		//std::cout << node->getLabel() << " " << (*nodes_val)[i] << std::endl;
		i ++;
	}
	//std::cout << std::endl;

	return converged;
}

/* Print current state of all the nodes of the network */
void MaBoSSNetwork::printNodes(NetworkState* netStates)
{
	std::vector<Node*> node3 = network->getNodes();
	for ( auto node: node3 )
	{
		std::cout << node->getLabel() << "=" << netStates->getNodeState(node) << "; ";
	}
	std::cout << std::endl;
}

/* Return the index of node based on node's name */
int MaBoSSNetwork::get_node_index( std::string name )
{
	auto res = node_names.find(name);
	if ( res != node_names.end() )
		return res->second;
	return -1;	
}




