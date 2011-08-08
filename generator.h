#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <vector>
#include <iostream>
#include <cassert>
#include <string>

#include <jack/jack.h>
#include <jack/midiport.h>

#include "disposable.h"
#include "sample.h"

struct note {
	bool playing;

	unsigned int key;

	//! The frame the event to start this voice happened
	jack_nframes_t note_on_frame;
	unsigned int note_on_velocity;

	note(unsigned int note_on_velocity = 0, jack_nframes_t note_on_frame = 0, bool playing = false) :
		note_on_velocity(note_on_velocity),
		note_on_frame(note_on_frame),
		playing(playing)
	{

	}
};


struct generator {
	std::string name;

	std::vector<note> notes;
	unsigned int current_note;

	//! the channel this generator listens on
	unsigned int channel;

	//! the tuning note
	unsigned int transpose;

	//! lowest note tp react to
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
		std::cout << "~generator()" << std::endl; 
	}

	generator(
		disposable_sample_ptr s,
		unsigned int polyphony = 4, 
		unsigned int channel = 0,
		unsigned int transpose = 0,
		unsigned int min_note = 0,
		unsigned int max_mote = 127,
		unsigned int min_velocity = 0,
		unsigned int max_velocity = 127,
		double velocity_factor = 1.0
	) :
		notes(polyphony),
		current_note(0),
		sample_(s),
		channel(0),
		transpose(transpose),
		min_note(min_note),
		max_note(max_note),
		min_velocity(min_velocity),
		max_velocity(max_velocity),
		velocity_factor(velocity_factor)
	{ 
		std::cout << "generator()" << std::endl; 
	}

	generator(const generator& g) : sample_(g.sample_) {
		std::cout << "generator(const generator& g)" << std::endl;
		*this = g;
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
				if (((*(midi_event.buffer) & 0xf0)) == 0x90) {
					notes[current_note].key = *(midi_event.buffer+1);
					notes[current_note].note_on_frame = last_frame_time + frame;
					notes[current_note].playing = true;
					current_note = (++current_note) % notes.size();
					//notes[*(midi_event.buffer+1)].note_on_velocity = *(midi_event.buffer+2);;
				}

				jack_midi_event_get(&midi_event, midi_in_buf, midi_in_event_index);
				++midi_in_event_index;
			}

			for (unsigned int note_index = 0; note_index < notes.size(); ++note_index) {
				if (notes[note_index].playing) 
				{
					if (last_frame_time + frame < notes[note_index].note_on_frame + sample_->t.data_0.size())
					{
						out_0[frame] += sample_->t.data_0[last_frame_time + frame - notes[note_index].note_on_frame];				
						out_1[frame] += sample_->t.data_0[last_frame_time + frame - notes[note_index].note_on_frame];				
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
