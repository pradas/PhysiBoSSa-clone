#ifndef _MaBoSS_Net_h_
#define _MaBoSS_Net_h_

/**
 * \brief Interface with MaBoSS software
 *
 * One network object contains network configs.
 * Handle initialization, running it...
 *
 * Created on 05/19/2017
 * G. Letort, Institute Curie
 */

#include "MaBEstEngine.h"
#include "utils.h"

class MaBoSSNetwork
{
	private:
		/** \brief MaBoSS instances: network */
		Network* network;
		/** \brief MaBoSS instances: configurations */
		RunConfig* runConfig;
		/** \brief MaBoSS instances: symbols list */
		SymbolTable* symbTable;

		/** \brief Time step to update the cycle */
		double update_time;

		/** \brief Names and indices of network nodes */
		std::map< std::string, int > node_names;

		/** \brief Keep default value of nodes from CFG file */
		std::vector<bool> def_nodes;

		/** \brief Read and load nodes initial states */
		void initNetworkState();
		
		/** \brief Load previous network states and inputs */
		void load( NetworkState* netState, std::vector<bool>* inputs );

	public:
		/** \brief Constructor */
		MaBoSSNetwork( std::string networkFile, std::string configFile );
		/** \brief Destructor */
		~MaBoSSNetwork();

		/** \brief return number of nodes */
		inline int number_of_nodes()
		{ return node_names.size(); };
		
		/** \brief Return update time value */
		inline double update_time_step()
		{ return update_time; };

		/** \brief Set values of nodes to default values */
		void set_default( std::vector<bool>* nodes );

		/** \brief Run the current network
		 *
		 * Otherwise put new network states in input netStates
		 * cellline: index of current cell_line for specific properties (rates) 
		 * */
		void run(std::vector<bool>* nodes_val);
		
		/** \brief Print current state of all the nodes of the network */
		void printNodes( NetworkState* netStates);

		/** \brief Return node of given name current value
		 *
		 * Return -1 if node doesn't exit \n
		 * Return 0 if node is 0 \n
		 * Return 1 if node is 1 */
		int get_node_index( std::string name );
};

#endif