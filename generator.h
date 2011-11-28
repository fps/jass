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

	//! sample start and sample end are fractions of the total length of the sample 
	double sample_start;
	double sample_end;
	
	bool looping;
	double loop_start;
	double loop_end;


	bool muted;	
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
	
	unsigned int current_voice;

	//! precalculated stretch table for different note differences
	//! stretch[128] == 1.0
	double stretch_factors[256];

	virtual ~generator()
	{ 
		//std::cout << "~generator()" << std::endl; 
	}

	generator(
		const std::string &name,
		disposable_sample_ptr s,
		double sample_start = 0,
		double sample_end = 1.0,
		bool looping = false,
		double loop_start = 0,
		double loop_end = 1.0,
		bool muted = false,
		double gain = 0.0,
		unsigned int channel = 0,
		unsigned int note = 60,
		unsigned int min_note = 0,
		unsigned int max_note = 127,
		unsigned int min_velocity = 0,
		unsigned int max_velocity = 127,
		double velocity_factor = 1.0,
		double attack_g = 0.01,
		double decay_g = 0.0,
		double sustain_g = 0.0,
		double release_g = 0.01
	) :
		name(name),
		sample_(s),
		sample_start(sample_start),
		sample_end(sample_end),
		looping(looping),
		loop_start(loop_start),
		loop_end(loop_end),
		muted(muted),
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
		current_voice(0)
	{ 
		//! initialize stretch factors lookup table
		for (int i = 0; i < 256; ++i)
			stretch_factors[i] = (i - 128 != 0) ? pow(pow(2.0, 1.0/12.0), i - 128) : 1.0;
	}

	//! Process a single frame
	//! Updates voice info
	inline void process(
		float *out_0, float *out_1, 
		const jack_nframes_t last_frame_time, 
		const jack_nframes_t frame, 
		const jack_nframes_t sample_rate, 
		voice &v
	) {
		if (muted || v.state == voice::OFF) return;


		const double stretch = stretch_factors[(v.note - note) + 128];

		const unsigned int sample_length = sample_->t.data_0.size();
		const unsigned int frames_since_note_on = (last_frame_time + frame - v.note_on_frame);

		double current_frame = sample_length * sample_start + (stretch * frames_since_note_on);

		if (looping) {
			if (current_frame >= loop_end * sample_length) 
				current_frame = 
					(unsigned int)(loop_start * sample_length) + 
						fmod(
							(current_frame - (unsigned int)(loop_start * sample_length)), 
							(unsigned int)(sample_length * (loop_end - loop_start))
						);
		}
					
		if (current_frame < 0 || current_frame >= sample_length * sample_end) {
			v.state = voice::OFF;
			return;
		} 

		const double time_since_note_on = (double)(frames_since_note_on)/(double)sample_rate;
		double gain_envelope = 0.0;

		if (v.state == voice::ATTACK) {
			gain_envelope = adsr_attack(attack_g, decay_g, sustain_g, release_g, time_since_note_on);
		}

		if (v.state == voice::RELEASE) {
			const double release_time = (double)(v.note_off_frame - v.note_on_frame)/(double)sample_rate;

			gain_envelope = adsr(attack_g, decay_g, sustain_g, release_g, time_since_note_on, release_time);
			if (release_time - v.note_on_frame/(double)sample_rate >= release_g) 
			v.state = voice::OFF;
		}

		const double vel_gain = 
			velocity_factor * (((double)v.note_on_velocity-min_velocity)
				/(double)(max_velocity-min_velocity));

		const double g =  pow(10.0, gain_envelope/20.0) * vel_gain * pow(10.0, gain/20.0);

		float *data_0 = &(sample_->t.data_0[0]);
		float *data_1 = &(sample_->t.data_1[0]);

		const double mix = fmod(current_frame, 1.0);
		const double one_minus_mix = 1.0 - mix;

		const unsigned int floor_current_frame = std::min((unsigned int)floor(current_frame), sample_length - 1);
		const unsigned int ceil_current_frame = std::min((unsigned int)ceil(current_frame), sample_length - 1);

		out_0[frame] += g * (one_minus_mix * data_0[floor_current_frame] + mix * data_0[ceil_current_frame]);
		out_1[frame] += g * (one_minus_mix * data_1[floor_current_frame] + mix * data_1[ceil_current_frame]);
	}

	protected:
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
