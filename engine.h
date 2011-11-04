#ifndef JASS_ENGINE_HH
#define JASS_ENGINE_HH

#include <vector>
#include <list>
#include <algorithm>
#include <iostream>

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/session.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "disposable.h"
#include "generator.h"
#include "ringbuffer.h"
#include "jass.hxx"
#include "xsd_error_handler.h"
#include "assign.h"
#include "voice.h"

#include <QObject>

typedef std::vector<disposable_generator_ptr> generator_vector;
typedef disposable<generator_vector> disposable_generator_vector;
typedef boost::shared_ptr<disposable_generator_vector> disposable_generator_vector_ptr;

typedef std::list<disposable_generator_ptr> generator_list;
typedef disposable<generator_list> disposable_generator_list;
typedef boost::shared_ptr<disposable_generator_list> disposable_generator_list_ptr;


typedef ringbuffer<boost::function<void(void)> > command_ringbuffer;

struct engine;

extern "C" {
	int process_callback(jack_nframes_t, void *p);
	void session_callback(jack_session_event_t *event, void *arg);
}


struct gvoice {
	disposable_generator_ptr g;
	voice v;
};
typedef disposable<std::vector<gvoice> > disposable_gvoice_vector;
typedef boost::shared_ptr<disposable_gvoice_vector> disposable_gvoice_vector_ptr;


class engine : public QObject {
	Q_OBJECT

	public:
		//! The ringbuffer for the commands that have to be passed to the process callback
		command_ringbuffer commands;

		//! When the engine is done processing a command that possibly alters references, it will signal completion by writing a 0 in this ringbuffer
		ringbuffer<char> acknowledgements;

		disposable_generator_list_ptr gens;

		//! a single generator to audit a sample
		disposable_generator_ptr auditor_gen;

		jack_client_t *jack_client;
		jack_port_t *out_0;
		jack_port_t *out_1;
		jack_port_t *midi_in;

		//! Set this member only using the set_samplerate method..
		double sample_rate;

		disposable_gvoice_vector_ptr voices;
		unsigned int current_voice;

	public:
		engine(const char *uuid = 0) 
		: 
			commands(1024),
			acknowledgements(1024),
			gens(disposable_generator_list::create(generator_list())),
			voices(disposable_gvoice_vector::create(std::vector<gvoice>(128))),
			current_voice(0)
		{
			heap *h = heap::get();	

			jack_client = jack_client_open("jass", JackSessionID, NULL, uuid);
			out_0 = jack_port_register(jack_client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			out_1 = jack_port_register(jack_client, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			midi_in = jack_port_register(jack_client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

			sample_rate = jack_get_sample_rate(jack_client);

			jack_set_session_callback(jack_client, ::session_callback, this);

			jack_set_process_callback(jack_client, process_callback, (void*)this);
			jack_activate(jack_client);
		}

		~engine() {
			jack_deactivate(jack_client);
			jack_client_close(jack_client);
		}

		void set_number_of_voices(unsigned int num) {
			disposable_voice_vector_ptr voices = disposable_voice_vector::create(std::vector<voice>(num));
		}

		void set_sample_rate(double rate) {
			if (rate != sample_rate) {
				sample_rate = rate;
				//! TODO: Reload all samples..
			}
		}

		void session_callback(jack_session_event_t *event) {
			emit session_event(event);
		}

		void play_auditor() {
			assert(auditor_gen.get());
#if 0
			auditor_gen->t.channel = 16;
			auditor_gen->t.voices->t[0].gain_envelope_state = voice::ATTACK;
			auditor_gen->t.voices->t[0].filter_envelope_state = voice::ATTACK;
			auditor_gen->t.voices->t[0].note_on_frame = jack_last_frame_time(jack_client);
			auditor_gen->t.voices->t[0].note_on_velocity = 64;
			auditor_gen->t.voices->t[0].note = 64;
#endif
		}

		void process_note_on(jack_nframes_t nframes, unsigned int note, unsigned int velocity, unsigned int channel) {
			// find responsible generator
			for (generator_list::iterator it = gens->t.begin(); it != gens->t.end(); ++it) {
				if (
					(*it)->t.channel == channel &&
					(*it)->t.min_note < note &&
					(*it)->t.max_note > note &&
					(*it)->t.min_velocity < velocity &&
					(*it)->t.max_velocity > velocity
				) {
					//! setup voice with parameters
					voices->t[current_voice].g = (*it);
					voices->t[current_voice].v.channel = channel;
					voices->t[current_voice].v.note = note;
					voices->t[current_voice].v.note_on_velocity = velocity;
					voices->t[current_voice].v.note_on_frame = nframes;
					voices->t[current_voice].v.state = voice::ATTACK;

					//! TODO: decide on better voice allocation algo
					current_voice = (++current_voice) % voices->t.size();
					//std::cout << "current_voice" << current_voice << std::endl;
				}
			}
		}

		//! switch envelope states off voices responsible for this note to RELEASE
		void process_note_off(jack_nframes_t nframes, unsigned int note, unsigned int channel) {
			for (unsigned int voice = 0; voice != voices->t.size(); ++voice) {
				if (
					voices->t[voice].v.state == voice::ATTACK && 
					voices->t[voice].v.channel == channel && 
					voices->t[voice].v.note == note
				) {
					voices->t[voice].v.state = voice::RELEASE;
					//voices->t[voice].v.gain_envelope_on_note_off = voices->t[voice].v.gain_envelope;
					voices->t[voice].v.note_off_frame = nframes;
				}
			}	
		}

		void process(jack_nframes_t nframes) {
			//! Execute commands passed in through ringbuffer
			while(commands.can_read()) { /* std::cout << "read()()" << std::endl; */ 
				commands.read()(); 
				if (!acknowledgements.can_write()) std::cout << "ack buffer full" << std::endl;
				else acknowledgements.write(0);
			}

			float *out_0_buf = (float*)jack_port_get_buffer(out_0, nframes);
			float *out_1_buf = (float*)jack_port_get_buffer(out_1, nframes);
			void *midi_in_buf = jack_port_get_buffer(midi_in, nframes);	

			//process_midi(midi_in_buf, nframes, jack_client);

			//! zero the buffers first
			std::fill(out_0_buf, out_0_buf + nframes, 0);
			std::fill(out_1_buf, out_1_buf + nframes, 0);

			//! Synthesize
			if (auditor_gen.get()) {
				//auditor_gen->t.process(out_0_buf, out_1_buf, midi_in_buf, nframes, jack_client);
			}

			jack_nframes_t last_frame_time = jack_last_frame_time(jack_client);

			jack_nframes_t midi_in_event_index = 0;
			jack_nframes_t midi_in_event_count = jack_midi_get_event_count(midi_in_buf);
	
			jack_midi_event_t midi_event;
			if (midi_in_event_count > 0)
				jack_midi_event_get(&midi_event, midi_in_buf, midi_in_event_index);

			for (unsigned int frame = 0; frame < nframes; ++frame) {
				while (midi_in_event_index < midi_in_event_count && midi_event.time == frame) {
					if (((*(midi_event.buffer) & 0xf0)) == 0x80
						|| (((*(midi_event.buffer) & 0xf0) == 0x90 && *(midi_event.buffer+2) == 0))
					) {
						process_note_off(
							last_frame_time+frame, 
							*(midi_event.buffer+1), 
							(*(midi_event.buffer) & 0x0f)
						);
					}
		
					if (((*(midi_event.buffer) & 0xf0)) == 0x90 && *(midi_event.buffer+2) != 0) {
						process_note_on(
							last_frame_time + frame, 
							*(midi_event.buffer+1), 
							*(midi_event.buffer+2),
							(*(midi_event.buffer) & 0x0f)
						);
					}
					++midi_in_event_index;
					jack_midi_event_get(&midi_event, midi_in_buf, midi_in_event_index);
				}
				for (unsigned int index = 0; index < voices->t.size(); ++index) {
					if (voices->t[index].v.state != voice::OFF) {
						voices->t[index].g->t.process(out_0_buf, out_1_buf, last_frame_time, frame, jack_get_sample_rate(jack_client), voices->t[index].v);
					}
				}
			}
		}
	
	signals:
		void session_event(jack_session_event_t *);
};


#endif
