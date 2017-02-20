#include <iostream>
#include <algorithm>
#include <functional>

#include <QApplication>
#include <QThread>
#include <QWidget>
#include <QHBoxLayout>

#include <genn/genetics.hpp>
#include <genn/mutation.hpp>
#include <genn/instance.hpp>

#include <genn/netview.hpp>
#include <genn/plot.hpp>

#include <sampler.hpp>

class Window : public QWidget {
	Q_OBJECT
public:
	NetView *view;
	Plot *fitness_plot;
	
	QHBoxLayout *layout;
	
	Window(NetView *v, Plot *fp) 
	: QWidget(), view(v), fitness_plot(fp) {
		resize(1200, 600);
		setWindowTitle("Evo2048");
		
		layout = new QHBoxLayout;
		layout->addWidget(view, 1);
		layout->addWidget(fitness_plot, 1);
		
		setLayout(layout);
		
		view->anim_start();
		fitness_plot->anim_start();
	}
	
	virtual ~Window() {
		view->anim_stop();
		fitness_plot->anim_stop();
	}
};

class Organism {
public:
	NetworkInst inst;
	double score = 0.0;
	
	Organism(const NetworkGene &orig) {
		inst.build(orig);
	}
};

class Selector {
public:
	std::vector<Organism> orgs;
	double record = 0.0;
	
	Selector(const NetworkGene &sample, int count) {
		orgs.resize(count, Organism(sample));
	}
	
	void select(int ndrop) {
		std::sort(
			orgs.begin(), orgs.end(), 
			[](const Organism &o0, const Organism &o1) -> bool {
				return o0.score < o1.score;
			}
		);
		record = orgs[0].score;
		
		if (ndrop > int(orgs.size())/2) {
			ndrop = orgs.size()/2;
		}
		auto fi = orgs.begin();
		auto ri = orgs.rbegin();
		for (int i = 0; i < ndrop; ++i) {
			ri->inst.load_from(fi->inst);
			++fi;
			++ri;
		}
	}
	
	void mutate(Mutator &mut, int ndrop) {
		int nmut = orgs.size() - ndrop;
		for (Organism &org : orgs) {
			mut.step_rand_weights(&org.inst, 5e-3);
			nmut -= 1;
			if (nmut <= 0) {
				break;
			}
		}
	}
};

class Thread : public QThread {
	Q_OBJECT
public:
	std::function<void()> func;
	Thread(std::function<void()> fn) : func(fn) {}
	virtual void run() override {
		func();
	}
};

int main(int argc, char *argv[]) {
	srand(87654321);
	
	RandEngine rand;
	
	NetworkGene net;
	net.nodes.add(NodeID(1), NodeGene(0));
	net.nodes.add(NodeID(2), NodeGene(0));
	net.nodes.add(NodeID(3), NodeGene(0));
	net.links.add(LinkID(1,3), LinkGene(0));
	net.links.add(LinkID(2,3), LinkGene(0));
	
	Mutator mut(3);
	
	Selector sel(net, 32);
	const int drop = 1;
	
	QApplication app(argc, argv);
	
	NetView *netview = new NetView();
	Plot *fitness_plot = new Plot(Plot::LOG_SCALE_Y);
	int point_count = 0;
	
	Sampler samp;
	
	Window *window = new Window(netview, fitness_plot);
	window->show();
	
	bool done = false;
	Thread thread([&](){
		int cnt = 0;
		while(!done) {
			for(int i = 0; i < int(sel.orgs.size()); ++i) {
				Organism &org = sel.orgs[i];
				org.score = samp.sample(&org.inst);
			}
			
			sel.select(drop);
			sel.orgs[0].inst.upload(&net);
			sel.mutate(mut, drop);
			
			if(cnt % 0x40 == 0) {
				netview->sync(net);
				fitness_plot->add(point_count, sel.record);
				point_count += 1;
				std::cout << sel.record << std::endl;
			}
			
			cnt += 1;
		}
	});
	thread.start();
	
	int rs = app.exec();

	done = true;
	thread.wait();
	
	// delete window;
	
	return rs;
}

#include "main.moc"
