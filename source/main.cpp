#include <iostream>
#include <thread>
#include <algorithm>

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>

#include <genn/genetics.hpp>
#include <genn/mutation.hpp>
#include <genn/instance.hpp>

#include <genn/netview.hpp>

#include <game.hpp>

class Window : public QWidget {
public:
	GameView *game;
	NetView *view;
	
	QHBoxLayout *layout;
	
	Window(GameView *g, NetView *v) 
	: QWidget(), game(g), view(v) {
		resize(1200, 600);
		setWindowTitle("Evo2048");
		
		layout = new QHBoxLayout;
		layout->addWidget(game, 0);
		layout->addWidget(view, 1);
		
		setLayout(layout);
		
		game->startAnim();
		view->startAnim();
	}
};

class Organism {
public:
	NetworkGene gene;
	NetworkInst inst;
	double score = 0.0;
	
	Organism(const NetworkGene &orig) {
		gene = orig;
	}
	
	void build() {
		inst.build(gene);
	}
};

class Selector {
public:
	std::vector<Organism*> orgs;
	double record = 0.0;
	
	Selector(const NetworkGene &sample, int count) {
		orgs.resize(count, nullptr);
		for (Organism * &org : orgs) {
			org = new Organism(sample);
		}
	}
	
	~Selector() {
		for (auto org : orgs) {
			delete org;
		}
	}
	
	void select(int ndrop) {
		std::sort(
			orgs.begin(), orgs.end(), 
			[](const Organism *o0, const Organism *o1) -> bool {
				return o0->score > o1->score; 
			}
		);
		record = orgs[0]->score;
		
		if (ndrop > orgs.size()/2) {
			ndrop = orgs.size()/2;
		}
		auto fi = orgs.begin();
		auto ri = orgs.rbegin();
		for (int i = 0; i < ndrop; ++i) {
			delete *ri;
			*ri = new Organism((*fi)->gene);
		}
	}
	
	void mutate(Mutator &mut, int ndrop) {
		int nmut = orgs.size() - ndrop;
		for (auto *org : orgs) {
			mut.step_rand_weights(&org->gene, 1e-2);
			org->build();
			nmut -= 1;
			if (nmut <= 0) {
				break;
			}
		}
	}
};

int main(int argc, char *argv[]) {
	srand(87654321);
	
	RandEngine rand;
	
	NetworkGene net;
	net.nodes[NodeID(1)] = NodeGene(0);
	net.nodes[NodeID(2)] = NodeGene(0);
	net.nodes[NodeID(3)] = NodeGene(0);
	net.links[LinkID(1,3)] = LinkGene(0);
	net.links[LinkID(2,3)] = LinkGene(0);
	// net.links[LinkID(3,1)] = Link(rand.norm());
	
	Mutator mut(3);
	
	Selector sel(net, 8);
	
	QApplication app(argc, argv);
	
	NetView *netview = new NetView();
	// netview->connect(&net);
	
	GameView *game = new GameView(0.4);
	
	Window *window = new Window(game, netview);
	window->show();
	
	bool done = false;
	std::thread thread([&](){
		int cnt = 0;
		while(!done) {
			// double dt = 1e-2;
			// game->step(dt);
			
			int drop = 1;
			for(int i = 0; i < int(sel.orgs.size()); ++i) {
				Organism *org = sel.orgs[i];
				org->build();
				NetworkInst &inst = org->inst;
				
				int samp = 4;
				double err = 0.0;
				for(int j = 0; j < samp; ++j) {
					int _in0 = j % 2;
					int _in1 = j / 2;
					
					int _out = (_in0 + _in1) % 2; // xor
					// int _out = (_in0 + _in1) > 0; // or
					// int _out = (_in0 + _in1) >= 2; // and
					
					double in0 = _in0 ? 0.9 : -0.9, in1 = _in1 ? 0.9 : -0.9, out = _out ? 0.9 : -0.9;
					
					int est = 8, tot = 16;
					double lerr = 0.0;
					for(int k = 0; k < tot; ++k) {
						inst.nodes[0].in += in0;
						inst.nodes[1].in += in1;
						inst.step();
						if(k > est) {
							lerr -= fabs(out - inst.nodes[2].out);
						}
					}
					err = lerr/(tot - est);
					org->inst.clear();
				}
				org->score = err/samp;
			}
			
			netview->connect(nullptr);
			sel.select(drop);
			sel.mutate(mut, drop);
			netview->connect(&sel.orgs[0]->gene);
			
			if(cnt % 0x100 == 0) {
				std::cout << sel.record << std::endl;
			}
			
			/*
			std::this_thread::sleep_for(
				std::chrono::microseconds(int(1e6*dt))
			);
			*/
			cnt += 1;
		}
	});

	int rs = app.exec();

	done = true;
	thread.join();
	
	delete window;
	
	return rs;
}
