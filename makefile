.PHONY: all

all: jass

jass: *.h *.cc
	g++ -g -o jass *.cc `pkg-config --cflags --libs jack QtCore QtGui`