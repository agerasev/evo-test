#pragma once

#include <thread>
#include <mutex>

#include <la/vec.hpp>

#include <QWidget>
#include <QPainter>
#include <QTimer>

class Game {
public:
	bool done = true;
	std::mutex mtx;
	
	double pos = 0;
	double vel = 0;
	double acc = 0;
	
	Game(double p = 0) : pos(p) {
		
	}
	void accel(double a) {
		acc += a;
	}
	void move(double dt) {
		pos += vel*dt;
		vel += acc*dt;
		acc = 0;
	}
	
	virtual void run() {
		done = false;
		int delay = 10;
		while(!done) {
			mtx.lock();
			accel(1.0*sin(pos) - 0.1*vel);
			move(1e-3*delay);
			mtx.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
	}
};

class GameView : public QWidget, public Game {
	Q_OBJECT
public:
	bool anim_done = true;
	
	GameView(double p = 0) : QWidget(), Game(p) {
		// setStyleSheet("background-color:#CDC0B4;");
	}
	
	void paintEvent(QPaintEvent *event) override {
		QWidget::paintEvent(event);
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		
		mtx.lock();
		double s = 0.5*std::min(rect().width(), rect().height());
		QPointF c = rect().center();
		double rl = 0.8*s, rs = 0.15*s;
		QPointF p(c.x() + rl*sin(Game::pos), c.y() - rl*cos(Game::pos));
		mtx.unlock();
		
		QPen pen;
		pen.setColor(QColor(0,0,0));
		pen.setWidthF(1e-2*s);
		painter.setPen(pen);
		painter.setBrush(QBrush(QColor(0xff,0xff,0xff)));
		
		painter.drawLine(c, p);
		painter.drawEllipse(p, rs, rs);
	}
	
	void timer_func() {
		int ms = 40;
		update();
		if(!done) {
			QTimer::singleShot(ms, [this](){timer_func();});
		}
	}
	
	void startAnim() {
		done = false;
		timer_func();
	}
	
	void stopAnim() {
		done = true;
	}
};
