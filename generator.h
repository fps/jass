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

	//! TODO: think about writing a copy constructor/assignment operator
	disposable_voice_vector_ptr voices;
	unsigned int current_voice;

	//! the channel this generator listens on
	unsigned int channel;

	//! the tuning note
	unsigned int note;

	//! lowest note to react to
	unsigned int min_note;

	//! highest note to react to
	unsigned int max_note;

	//! the lowest velocity to react to
	unsigned int min_velocity;

	//! the highest velocity to react to
	unsigned int max_velocity;

	//! resulting velocity is velocity_factor * (velocity - high_velocity)/(high_velocity - low_velocity)
	double velocity_factor;

	virtual ~generator()
	{ 
		//std::cout << "~generator()" << std::endl; 
	}

	generator(
		const std::string &name,
		disposable_sample_ptr s,
		unsigned int polyphony = 1, 
		unsigned int channel = 0,
		unsigned int note = 64,
		unsigned int min_note = 0,
		unsigned int max_note = 127,
		unsigned int min_velocity = 0,
		unsigned int max_velocity = 127,
		double velocity_factor = 1.0
	) :
		name(name),
		voices(disposable_voice_vector::create(std::vector<voice>(polyphony))),
		current_voice(0),
		sample_(s),
		channel(channel),
		note(note),
		min_note(min_note),
		max_note(max_note),
		min_velocity(min_velocity),
		max_velocity(max_velocity),
		velocity_factor(velocity_factor)
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
			if (midi_in_event_index < midi_in_event_count && midi_event.time == frame) {
				if (((*(midi_event.buffer) & 0xf0)) == 0x80
					|| (((*(midi_event.buffer) & 0xf0)) == 0x90 && *(midi_event.buffer+2) == 0)) {
					//! Note off
					if((*(midi_event.buffer) & 0x0f) == channel)
					{
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

			for (unsigned int voice_index = 0; voice_index < voices->t.size(); ++voice_index) {
				if (voices->t[voice_index].playing) 
				{
					double stretch = 1.0;
					if (((int)voices->t[voice_index].note - (int)note) != 0) 
						stretch = pow(pow(2.0, 1.0/12.0), (int)voices->t[voice_index].note - (int)note);

					int current_frame = floor(stretch * (last_frame_time + frame - voices->t[voice_index].note_on_frame));
					if (current_frame >= 0 && current_frame < sample_->t.data_0.size())
					{
						double gain = ((double)voices->t[voice_index].note_on_velocity/128.0) * velocity_factor;

						out_0[frame] += gain * sample_->t.data_0[current_frame];
						out_1[frame] += gain * sample_->t.data_0[current_frame];
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
		disposable_sample_ptr sample_;
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
