all: 
	g++ -g -o jass *.cc `pkg-config --cflags --libs jack QtCore QtGui`