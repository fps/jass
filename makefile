.PHONY: all

all: jass

jass: *.h *.cc qfunctor.moc.cc
	g++ -g -o jass *.cc `pkg-config --cflags --libs jack QtCore QtGui sndfile samplerate`

qfunctor.moc.cc: qfunctor.h
	moc -o qfunctor.moc.cc qfunctor.h