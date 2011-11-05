#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <vector>
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <algorithm>

#include <jack/jack.h>
#include <jack/midiport.h>

#include "disposable.h"
#include "sample.h"
#include "voice.h"
#include "adsr.h"

struct generator {
	std::string name;

	disposable_sample_ptr sample_;
	double sample_start;
	double sample_end;
	
	bool looping;
	
	double gain;

	unsigned int channel;
	unsigned int note;

	unsigned int min_note;
	unsigned int max_note;
	
	unsigned int min_velocity;
	unsigned int max_velocity;

	double velocity_factor;
	
	double attack_g;
	double decay_g;
	double sustain_g;
	double release_g;
	
	enum filter_type { NONE, LOW_PASS, HIGH_PASS, BAND_PASS };
	int filter;
	
	double freq_f;
	double q_f;
	double key_follow_f;
	
	double attack_f;
	double decay_f;
	double sustain_f;
	double release_f;

	unsigned int current_voice;

	virtual ~generator()
	{ 
		//std::cout << "~generator()" << std::endl; 
	}

	generator(
		const std::string &name,
		disposable_sample_ptr s,
		double sample_start = 0,
		double sample_end = 0,
		bool looping = false,
		double gain = 1.0,
		unsigned int channel = 0,
		unsigned int note = 64,
		unsigned int min_note = 0,
		unsigned int max_note = 127,
		unsigned int min_velocity = 0,
		unsigned int max_velocity = 127,
		double velocity_factor = 1.0,
		double attack_g = 0.001,
		double decay_g = 0.0,
		double sustain_g = 1.0,
		double release_g = 0.001,
		int filter = NONE,
		double freq_f = 1.0,
		double q_f = 0.5,
		double key_follow_f = 0.0,
		double attack_f = 0.001,
		double decay_f = 0.0,
		double sustain_f = 1.0,
		double release_f = 0.001
	) :
		name(name),
		sample_(s),
		sample_start(sample_start),
		sample_end(sample_end),
		looping(looping),
		gain(gain),
		channel(channel),
		note(note),
		min_note(min_note),
		max_note(max_note),
		min_velocity(min_velocity),
		max_velocity(max_velocity),
		velocity_factor(velocity_factor),
		attack_g(attack_g),
		decay_g(decay_g),
		sustain_g(sustain_g),
		release_g(release_g),
		filter(filter),
		freq_f(freq_f),
		q_f(q_f),
		key_follow_f(key_follow_f),
		attack_f(attack_f),
		decay_f(decay_f),
		sustain_f(sustain_f),
		release_f(release_f),
		current_voice(0)
	{ 
		// std::cout << "generator()" << std::endl; 
	}

	//! Process a single frame
	//! Updates voice info
	inline void process(
		float *out_0, float *out_1, 
		jack_nframes_t last_frame_time, 
		jack_nframes_t frame, 
		jack_nframes_t 
		sample_rate, 
		voice &v
	) {
		//! Overall check if we play at all (short curcuit to save CPU time)
		if (v.state != voice::OFF) 
		{
			//! TODO: replace retarded sample and hold interpolation
			double stretch = 1.0;
			if (((int)v.note - (int)note) != 0) 
				stretch = pow(pow(2.0, 1.0/12.0), (int)v.note - (int)note);

			int current_frame = floor(stretch * (last_frame_time + frame - v.note_on_frame));
			double time_in_sample = (double)(last_frame_time + frame - v.note_on_frame)/(double)sample_rate;
					
			if (current_frame < 0 || current_frame >= sample_->t.data_0.size()) {
				v.state = voice::OFF;
				return;
			} 

			double gain_envelope = 0.0;

			if (v.state == voice::ATTACK)
				gain_envelope = adsr(attack_g, decay_g, sustain_g, release_g, time_in_sample, time_in_sample + 1000000.0);

			if (v.state == voice::RELEASE) {
				double release_time = (double)(v.note_off_frame - v.note_on_frame)/(double)sample_rate;
				gain_envelope = adsr(attack_g, decay_g, sustain_g, release_g, time_in_sample, release_time);
			}

			if (v.state == voice::RELEASE && gain_envelope == 0.0) {
				v.state = voice::OFF;
				return;
			}

			double vel_gain = 
				velocity_factor * (((double)v.note_on_velocity-min_velocity)
					/(double)(max_velocity-min_velocity));

			out_0[frame] += gain_envelope * vel_gain * gain * sample_->t.data_0[current_frame];
			out_1[frame] += gain_envelope * vel_gain * gain * sample_->t.data_0[current_frame];
		}
	}

	protected:
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
