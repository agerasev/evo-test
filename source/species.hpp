#pragma once

#include <vector>

#include <genn/genetics.hpp>
#include <genn/instance.hpp>

class Indiv {
public:
	NetworkInst net;
	double fitness;
	
	void load_from(const Indiv &other) {
		net.load_from(other.net);
		fitness = other.fitness;
	}
};

class Species {
public:
	
};

