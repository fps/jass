#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <vector>
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

#include <jack/jack.h>
#include <jack/midiport.h>

#include "disposable.h"
#include "sample.h"

struct voice {
	enum { ATTACK, DECAY, SUSTAIN, RELEASE } state;
	bool playing;

	unsigned int note;

	//! The frame the event to start this voice happened
	jack_nframes_t note_on_frame;
	unsigned int note_on_velocity;

	voice(unsigned int note_on_velocity = 0, jack_nframes_t note_on_frame = 0, bool playing = false) :
		note_on_velocity(note_on_velocity),
		note_on_frame(note_on_frame),
		playing(playing)
	{

	}
};

typedef disposable<std::vector<voice> > disposable_voice_vector;
typedef boost::shared_ptr<disposable_voice_vector> disposable_voice_vector_ptr;

struct generator {
	std::string name;

	disposable_sample_ptr sample_;
	double sample_start;
	double sample_end;
	
	bool looping;
	
	disposable_voice_vector_ptr voices;
	
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

	void handle_midi_events(
		void *midi_in_buf, 
		jack_nframes_t frame, 
		jack_nframes_t last_frame_time,
		unsigned int midi_in_event_count, 
		unsigned int &midi_in_event_index,
		jack_midi_event_t &midi_event
	) {
		if (midi_in_event_index < midi_in_event_count && midi_event.time == frame) {
			if (((*(midi_event.buffer) & 0xf0)) == 0x80
				|| (((*(midi_event.buffer) & 0xf0)) == 0x90 && *(midi_event.buffer+2) == 0)
			) {
				//! Note off
				if((*(midi_event.buffer) & 0x0f) == channel)
				{
					//std::cout << "note off" << std::endl;
					for (unsigned int voice = 0; voice != voices->t.size(); ++voice) {
						if (voices->t[voice].note == *(midi_event.buffer+1)) {
							voices->t[voice].playing = false;
						}
					}	
				}
			}

			if (((*(midi_event.buffer) & 0xf0)) == 0x90 && *(midi_event.buffer+2) != 0) {
				//! Note on
				if(
					(*(midi_event.buffer) & 0x0f) == channel 
					&& *(midi_event.buffer+1) >= min_note
					&& *(midi_event.buffer+1) <= max_note
					&& *(midi_event.buffer+2) >= min_velocity
					&& *(midi_event.buffer+2) <= max_velocity
				) {
					//std::cout << "note on" << std::endl;
					//! We be responsible for this note command
					voices->t[current_voice].note = *(midi_event.buffer+1);
					voices->t[current_voice].note_on_velocity = *(midi_event.buffer+2);
					voices->t[current_voice].note_on_frame = last_frame_time + frame;
					voices->t[current_voice].playing = true;
					current_voice = (++current_voice) % voices->t.size();
				}
			}
			jack_midi_event_get(&midi_event, midi_in_buf, midi_in_event_index);
			++midi_in_event_index;
		}
	}

	generator(
		const std::string &name,
		disposable_sample_ptr s,
		double sample_start = 0,
		double sample_end = 0,
		bool looping = false,
		double gain = 1.0,
		unsigned int polyphony = 1, 
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
		voices(disposable_voice_vector::create(std::vector<voice>(polyphony))),
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

	void process(float *out_0, float *out_1, void * midi_in_buf, jack_nframes_t nframes, jack_client_t *jack_client) {
		jack_nframes_t midi_in_event_index = 0;
		jack_nframes_t midi_in_event_count = jack_midi_get_event_count(midi_in_buf);

		jack_midi_event_t midi_event;
		if (midi_in_event_count > 0)
			jack_midi_event_get(&midi_event, midi_in_buf, midi_in_event_index);

		jack_nframes_t last_frame_time = jack_last_frame_time(jack_client);
		for (unsigned int frame = 0; frame < nframes; ++frame) {
			handle_midi_events(
				midi_in_buf, 
				frame, 
				last_frame_time, 
				midi_in_event_count, 
				midi_in_event_index, 
				midi_event
			);

			//! Actually generate output
			for (unsigned int voice_index = 0; voice_index < voices->t.size(); ++voice_index) {
				if (voices->t[voice_index].playing) 
				{
					double stretch = 1.0;
					if (((int)voices->t[voice_index].note - (int)note) != 0) 
						stretch = pow(pow(2.0, 1.0/12.0), (int)voices->t[voice_index].note - (int)note);

					int current_frame = floor(stretch * (last_frame_time + frame - voices->t[voice_index].note_on_frame));
					if (current_frame >= 0 && current_frame < sample_->t.data_0.size())
					{
						//double gain = ((double)voices->t[voice_index].note_on_velocity/128.0) * velocity_factor;
						double vel_gain = 
							velocity_factor * (((double)voices->t[voice_index].note_on_velocity-min_velocity)
							/(double)(max_velocity-min_velocity));

						out_0[frame] += vel_gain * gain * sample_->t.data_0[current_frame];
						out_1[frame] += vel_gain * gain * sample_->t.data_0[current_frame];
					}
				}
			}
		}
	}

	void set_sample(disposable_sample_ptr s) {
		sample_ = s;
		//! Update voice info..
	}

	disposable_sample_ptr get_sample() {
		return sample_;
	}

	protected:
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
