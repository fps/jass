#ifndef JASS_VOICE_HH
#define JASS_VOICE_HH

#include "generator.h"

struct voice {
	enum envelope_state {OFF, ATTACK, RELEASE };
	int gain_envelope_state;
	double gain_envelope;
	double gain_envelope_on_note_off;
	
	int filter_envelope_state;
	double filter_envelope;
	
	unsigned int note;

	//! The frame the event to start this voice happened
	jack_nframes_t note_on_frame;
	jack_nframes_t note_off_frame;
	
	unsigned int note_on_velocity;

	disposable_generator_ptr gen;

	voice(disposable_generator_ptr gen = disposable_generator_ptr(), unsigned int note_on_velocity = 0, jack_nframes_t note_on_frame = 0, bool playing = false) :
		note_on_velocity(note_on_velocity),
		note_on_frame(note_on_frame),
		gain_envelope_state(OFF),
		gain_envelope(0.0),
		filter_envelope_state(OFF),
		filter_envelope(0.0),
		gen(gen)
	{

	}
};

typedef disposable<std::vector<voice> > disposable_voice_vector;
typedef boost::shared_ptr<disposable_voice_vector> disposable_voice_vector_ptr;

#endif
