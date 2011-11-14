#ifndef JASS_ENGINE_HH
#define JASS_ENGINE_HH

#include <vector>
#include <list>
#include <algorithm>
#include <iostream>

#include <jack/jack.h>
#include <jack/midiport.h>
#ifndef NO_JACK_SESSION
	#include <jack/session.h>
#endif

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
#include "command_queue.h"

#include <QObject>

typedef std::vector<disposable_generator_ptr> generator_vector;
typedef disposable<generator_vector> disposable_generator_vector;
typedef boost::shared_ptr<disposable_generator_vector> disposable_generator_vector_ptr;

typedef std::list<disposable_generator_ptr> generator_list;
typedef disposable<generator_list> disposable_generator_list;
typedef boost::shared_ptr<disposable_generator_list> disposable_generator_list_ptr;


struct engine;

extern "C" {
	int process_callback(jack_nframes_t, void *p);
#ifndef NO_JACK_SESSION
	void session_callback(jack_session_event_t *event, void *arg);
#endif
}


struct gvoice {
	disposable_generator_ptr g;
	voice v;
};
typedef disposable<std::vector<gvoice> > disposable_gvoice_vector;
typedef boost::shared_ptr<disposable_gvoice_vector> disposable_gvoice_vector_ptr;


class engine : public QObject, public command_queue {
	Q_OBJECT

	public:
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
		
		static engine *get(const char *uuid = 0) {
			if (instance) return instance;
			return (instance = new engine(uuid));
		}

	protected:
		static engine *instance;
		engine(const char *uuid = 0) 
		: 
			command_queue(1024, 1024),
			gens(disposable_generator_list::create(generator_list())),
			voices(disposable_gvoice_vector::create(std::vector<gvoice>(32))),
			current_voice(0)
		{
			heap *h = heap::get();	

#ifndef NO_JACK_SESSION
			jack_client = jack_client_open("jass", JackSessionID, NULL, uuid);
#endif
#ifdef NO_JACK_SESSION
			jack_client = jack_client_open("jass", JackNullOption, NULL, uuid);
#endif

			out_0 = jack_port_register(jack_client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			out_1 = jack_port_register(jack_client, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			midi_in = jack_port_register(jack_client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

			sample_rate = jack_get_sample_rate(jack_client);

#ifndef NO_JACK_SESSION
			jack_set_session_callback(jack_client, ::session_callback, this);
#endif

			jack_set_process_callback(jack_client, process_callback, (void*)this);
			jack_activate(jack_client);
		}

	public:
		~engine() {
			jack_deactivate(jack_client);
			jack_client_close(jack_client);
			instance = 0;
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

#ifndef NO_JACK_SESSION
		void session_callback(jack_session_event_t *event) {
			emit session_event(event);
		}
#endif

		void play_auditor() {
			assert(auditor_gen.get());
			
			voices->t[current_voice].g = auditor_gen;
			voices->t[current_voice].v.channel = 17;
			voices->t[current_voice].v.note = 64;
			voices->t[current_voice].v.note_on_velocity = 128;
			voices->t[current_voice].v.note_on_frame = jack_last_frame_time(jack_client);
			voices->t[current_voice].v.state = voice::ATTACK;
			current_voice = (++current_voice) % voices->t.size();
		}

		void process_note_on(jack_nframes_t nframes, unsigned int note, unsigned int velocity, unsigned int channel) {
			// find responsible generator
			for (generator_list::iterator it = gens->t.begin(); it != gens->t.end(); ++it) {
				if (
					(*it)->t.channel == channel &&
					(*it)->t.min_note <= note &&
					(*it)->t.max_note >= note &&
					(*it)->t.min_velocity <= velocity &&
					(*it)->t.max_velocity >= velocity
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

		//! switch envelope states of voices responsible for this note to RELEASE
		void process_note_off(jack_nframes_t nframes, unsigned int note, unsigned int channel) {
			for (unsigned int voice = 0; voice != voices->t.size(); ++voice) {
				if (
					voices->t[voice].v.state == voice::ATTACK && 
					voices->t[voice].v.channel == channel && 
					voices->t[voice].v.note == note
				) {
					voices->t[voice].v.state = voice::RELEASE;
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
				//! process midi events first to update voice states
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

				//! then process voices
				//! TODO: introduce linked list in the preallocated voices to make this faster. i.e. only iterate over active voices
				for (unsigned int index = 0; index < voices->t.size(); ++index) {
					if (voices->t[index].v.state != voice::OFF) {
						voices->t[index].g->t.process(out_0_buf, out_1_buf, last_frame_time, frame, jack_get_sample_rate(jack_client), voices->t[index].v);
					}
				}
			}
		}

#ifndef NO_JACK_SESSION	
	signals:
		void session_event(jack_session_event_t *);
#endif
};


#endif
