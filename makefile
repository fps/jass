.PHONY: all jass

all: jass

jass: 
	moc -o qfunctor.moc.cc qfunctor.h
	moc -o main_window.moc.cc main_window.h
	g++ -g -o jass *.cc *.cxx `pkg-config --cflags --libs jack QtCore QtGui sndfile samplerate` -lxerces-c


