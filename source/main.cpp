#include <thread>

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>

#include <genn/netview.hpp>

#include <game.hpp>


class Window : public QWidget {
public:
	NetworkGene gene;
	
	GameView *game;
	NetView *netview;
	QHBoxLayout *layout;
	
	Window(GameView *g) : QWidget(), game(g) {
		resize(1200, 600);
		setWindowTitle("Evo2048");
		
		netview = new NetView();
		
		layout = new QHBoxLayout;
		layout->addWidget(game, 1);
		layout->addWidget(netview, 1);
		
		setLayout(layout);
		
		gene.nodes[NodeID(1)] = NodeGene(1);
		gene.nodes[NodeID(2)] = NodeGene(-1);
		gene.nodes[NodeID(3)] = NodeGene(0);
		
		gene.links[LinkID(1,3)] = LinkGene(1);
		gene.links[LinkID(2,3)] = LinkGene(-1);
		
		game->startAnim();
		
		netview->sync(gene);
		netview->startAnim();
	}
};

int main(int argc, char *argv[]) {
	srand(87654321);
	
	QApplication app(argc, argv);
	
	GameView *game = new GameView(0.4);
	
	Window *window = new Window(game);
	window->show();
	
	std::thread thread([&game](){game->run();});

	int rs = app.exec();

	game->done = true;
	thread.join();
	
	delete window;
	
	return rs;
}
