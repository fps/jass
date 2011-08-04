.PHONY: all

all: jass

jass: *.h *.cc qfunctor.moc.cc
	moc -o qfunctor.moc.cc qfunctor.h
	moc -o main_window.moc.cc main_window.h
	g++ -g -o jass *.cc `pkg-config --cflags --libs jack QtCore QtGui sndfile samplerate`


