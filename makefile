.PHONY: all

all: jass

jass: *.h *.cc timed_functor.moc.cc
	g++ -g -o jass *.cc `pkg-config --cflags --libs jack QtCore QtGui sndfile samplerate`

timed_functor.moc.cc: timed_functor.h
	moc -o timed_functor.moc.cc timed_functor.h