#pragma once

#include <cstdio>

#include <genn/random.hpp>
#include <genn/instance.hpp>

class Sampler {
private:
	mutable RandEngine rand;
	
	static bool func(bool a, bool b) {
		return a && b;
	}
	
	static float btof(bool v) {
		if (v) {
			return 0.9;
		} else {
			return -0.9;
		}
	}

public:
	double sample(NetworkInst *inst) const {
		if (inst->nodes.size() < 3) {
			fprintf(stderr, "too few nodes\n");
		}
		
		double err = 0.0;
		const int nsamp = 16;
		const int len = 16;
		for (int i = 0; i < nsamp; ++i) {
			bool in0 = rand.int_() % 2, in1 = rand.int_() % 2;
			int out = func(in0, in1);
			
			double lerr = 0.0;
			for(int k = 0; k < len; ++k) {
				inst->nodes[0].in += btof(in0);
				inst->nodes[1].in += btof(in1);
				inst->step();
				lerr += fabs(btof(out) - inst->nodes[2].out);
			}
			
			err += lerr/len;
			inst->clear();
		}
		
		return err/nsamp;
	}
};
