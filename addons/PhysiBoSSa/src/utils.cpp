#include "utils.h"

std::random_device rd;
std::mt19937 gen(rd());

long SeedRandom( void )
{ 
	unsigned seed = std::time(0);
	seed *= std::random_device()();
	gen.seed(seed);
	return seed;
}

int UniformInt()
{
	std::uniform_int_distribution<int> int_dis;
	return int_dis(gen);
}