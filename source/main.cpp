#include <thread>

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>

#include <genn/genetics.hpp>
#include <genn/mutation.hpp>

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
		layout->addWidget(game, 1);
		layout->addWidget(view, 1);
		
		setLayout(layout);
		
		game->startAnim();
		view->startAnim();
	}
};

int main(int argc, char *argv[]) {
	srand(87654321);
	
	Network net;
	
	net.nodes[NodeID(1)] = Node(1);
	net.nodes[NodeID(2)] = Node(-1);
	net.nodes[NodeID(3)] = Node(0);
	
	net.links[LinkID(1,2)] = Link(1);
	net.links[LinkID(2,3)] = Link(-1);
	net.links[LinkID(3,1)] = Link(0);
	
	Mutator mut;
	
	QApplication app(argc, argv);
	
	NetView *netview = new NetView(&net);
	
	GameView *game = new GameView(0.4);
	
	Window *window = new Window(game, netview);
	window->show();
	
	bool done = false;
	std::thread thread([&](){
		while(!done) {
			double dt = 1e-2;
			game->step(dt);
			
			netview->mtx.lock();
			mut.mutate(&net);
			netview->mtx.unlock();
			
			std::this_thread::sleep_for(
				std::chrono::microseconds(int(1e6*dt))
			);
		}
	});

	int rs = app.exec();

	done = true;
	thread.join();
	
	delete window;
	
	return rs;
}
