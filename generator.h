#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <vector>
#include <iostream>
#include <cassert>

#include <jack/jack.h>

#include "disposable.h"
#include "sample.h"

struct voice {
	bool playing;
	jack_nframes_t frame;
	unsigned int velocity;
	unsigned int note;

	voice(unsigned int vel = 0, unsigned int note = 0, bool playing = true, jack_nframes_t frame = 0) :
		playing(playing),
		frame(frame),
		velocity(vel),
		note(note)
	{

	}
};

struct voice_manager {
	std::vector<voice> voices;
	voice_manager() : voices(128) { }
	void trigger(unsigned int velocity, unsigned int note) {
		voices[0] = voice(velocity, note);
	}
};

struct generator {
	disposable_sample_ptr sample_ptr;

	voice_manager voices;

	//! the channel this generator listens on
	unsigned int channel;

	//! the tuning note
	unsigned int note_a;

	//! lowest note tp react to
	unsigned int low_note;

	//! highest note to react to
	unsigned int high_note;

	//! the lowest velocity to react to
	unsigned int low_velocity;

	//! the highest velocity to react to
	unsigned int high_velocity;

	//! resulting velocity is velocity_factor * (velocity - high_velocity)/(high_velocity - low_velocity)
	double velocity_factor;

	virtual ~generator()
	{ 
		std::cout << "~generator()" << std::endl; 
	}

	generator(disposable_sample_ptr s) :
		sample_ptr(s),
		channel(0),
		note_a(64),
		low_note(0),
		high_note(127),
		low_velocity(0),
		high_velocity(127),
		velocity_factor(1.0)
	{ 
		std::cout << "generator()" << std::endl; 
		voices.voices[0].frame = 0;
		voices.voices[0].velocity = 64;
		voices.voices[0].note = 64;
	}

	generator(const generator& g) {
		std::cout << "generator(const generator& g)" << std::endl;
		*this = g;
	}

	void process(float *out_0, float *out_1, jack_nframes_t nframes) {
		for (unsigned int i = 0; i < nframes; ++i) {
			assert((voices.voices[0].frame + i) % sample_ptr->t.data_0.size() < sample_ptr->t.data_0.size());
			float s = sample_ptr->t.data_0[(voices.voices[0].frame + i) % sample_ptr->t.data_0.size()];
			out_0[i] += 0.2 * s;
			out_1[i] += 0.2 * s;
		}
		voices.voices[0].frame = (voices.voices[0].frame + nframes) % sample_ptr->t.data_0.size();
	}
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
