#ifndef JASS_VOICE_HH
#define JASS_VOICE_HH

#include <jack/jack.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "disposable.h"


struct voice {
	enum envelope_state {OFF, ATTACK, RELEASE };
	int state;
	//double gain_envelope_on_note_off;

	unsigned int note;
	unsigned int channel;
	unsigned int note_on_velocity;

	//! The frame on which the event to start this voice happened
	jack_nframes_t note_on_frame;

	//! Dito for note off
	jack_nframes_t note_off_frame;
	
	voice(unsigned int note_on_velocity = 0, jack_nframes_t note_on_frame = 0, bool playing = false) :
		note_on_velocity(note_on_velocity),
		note_on_frame(note_on_frame),
		state(OFF)
	{
		setup_filters();
	}

	void setup_filters() {
		
	}
};

typedef disposable<std::vector<voice> > disposable_voice_vector;
typedef boost::shared_ptr<disposable_voice_vector> disposable_voice_vector_ptr;

#endif
