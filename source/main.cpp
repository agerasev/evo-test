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

class Organism {
public:
	NetworkMutable net;
	INetwork inet;
	double score = 0;
	double fine = 0;
	
	Organism(const NetworkMutable &n) {
		net = n;
	}
	
	void build() {
		inet.build(net);
	}
	
	int mutate(Mutator *mut) {
		return mut->mutate(&net);
	}
};

class Selector {
public:
	std::vector<Organism*> orgs;
	Mutator *mut;
	double record = 0;
	
	Selector(int size, const Organism &org, Mutator *m) {
		orgs.resize(size, nullptr);
		for(int i = 0; i < size; ++i) {
			orgs[i] = new Organism(org.net);
			orgs[i]->build();
		}
		mut = m;
	}
	
	~Selector() {
		for(Organism *org : orgs) {
			delete org;
		}
	}
	
	void select() {
		double size_fine = 1e-4;
		double new_bonus = 1e-3;
		
		int size = orgs.size();
		std::sort(
			orgs.begin(), orgs.end(), 
			[](const Organism *o0, const Organism *o1) -> bool {
				return o0->score + o0->fine > o1->score + o1->fine; 
			}
		);
		record = orgs[0]->score;
		
		double dimf = 0.99;
		for(int i = 0; i < size; ++i) {
			orgs[i]->score *= dimf;
			orgs[i]->fine *= dimf;
			orgs[i]->fine -= size_fine*(orgs[i]->net.nodes.size() + orgs[i]->net.links.size());
		}
		
		int hsize = size/2;
		int shift = size - hsize;
		for(int i = 0; i < hsize; ++i) {
			Organism *&norg = orgs[i + shift];
			delete norg;
			norg = new Organism(orgs[i]->net);
			norg->score = orgs[i]->score;
			norg->fine += new_bonus*norg->mutate(mut);
			norg->net.nodes[1].bias = 0.0;
			norg->net.nodes[2].bias = 0.0;
			norg->net.nodes[3].bias = 0.0;
			norg->build();
		}
	}
};

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

int main(int argc, char *argv[]) {
	srand(87654321);
	
	RandEngine rand;
	
	NetworkMutable net;
	net.nodes[NodeID(1)] = Node(0);
	net.nodes[NodeID(2)] = Node(0);
	net.nodes[NodeID(3)] = Node(0);
	net.links[LinkID(1,3)] = Link(0);
	net.links[LinkID(2,3)] = Link(0);
	//net.links[LinkID(3,1)] = Link(rand.norm());
	
	Organism org(net);
	Mutator mut(3);
	Selector sel(16, org, &mut);
	
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
			
			for(int i = 0; i < int(sel.orgs.size()); ++i) {
				Organism *org = sel.orgs[i];
				INetwork &inet = org->inet;
				
				int samp = 16;
				double err = 0.0;
				for(int j = 0; j < 16; ++j) {
					int _in0 = rand.int_() % 2;
					int _in1 = rand.int_() % 2;
					
					int _out = (_in0 + _in1) % 2;
					//int _out = _in0 || _in1;
					
					double in0 = _in0 ? 0.9 : -0.9, in1 = _in1 ? 0.9 : -0.9, out = _out ? 0.9 : -0.9;
					
					int est = 8, tot = 16;
					double lerr = 0.0;
					for(int k = 0; k < tot; ++k) {
						inet.nodes[0].in += in0;
						inet.nodes[1].in += in1;
						inet.step();
						if(k > est) {
							lerr -= fabs(out - inet.nodes[2].out);
						}
					}
					err = lerr/(tot - est);
					org->inet.clear();
				}
				org->score = err/samp;
			}
			
			netview->connect(nullptr);
			sel.select();
			netview->connect(&sel.orgs[0]->net);
			
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
