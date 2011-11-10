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

	//! sample start and sample end are times instead of frames, so that stuff still works when
	//! loading a setup with a different engine samplerate
	double sample_start;
	double sample_end;
	
	bool looping;
	double loop_start;
	double loop_end;
	
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
		double loop_start = 0,
		double loop_end = 0,
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
		double release_g = 0.001
	) :
		name(name),
		sample_(s),
		sample_start(sample_start),
		sample_end(sample_end),
		looping(looping),
		loop_start(loop_start),
		loop_end(loop_end),
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

			int current_frame = sample_start + floor(stretch * (last_frame_time + frame - v.note_on_frame));
					
			if (current_frame < 0 || current_frame >= sample_->t.data_0.size() + sample_end) {
				v.state = voice::OFF;
				return;
			} 

			double time_since_note_on = (double)(last_frame_time + frame - v.note_on_frame)/(double)sample_rate;

			double gain_envelope = 0.0;

			if (v.state == voice::ATTACK) {
				gain_envelope = adsr(attack_g, decay_g, sustain_g, release_g, time_since_note_on, time_since_note_on + 1000000.0);
			}

			if (v.state == voice::RELEASE) {
				double release_time = (double)(v.note_off_frame - v.note_on_frame)/(double)sample_rate;
				gain_envelope = adsr(attack_g, decay_g, sustain_g, release_g, time_since_note_on, release_time);
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
