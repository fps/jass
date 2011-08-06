#ifndef JASS_ENGINE_HH
#define JASS_ENGINE_HH

#include <vector>
#include <algorithm>
#include <iostream>

#include <jack/jack.h>

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

typedef std::vector<disposable_generator_ptr> generator_vector;
typedef disposable<generator_vector> disposable_generator_vector;
typedef boost::shared_ptr<disposable_generator_vector> disposable_generator_vector_ptr;

typedef ringbuffer<boost::function<void(void)> > command_ringbuffer;

struct engine;

extern "C" {
	int process_callback(jack_nframes_t, void *p);
}

struct engine {
	//! The ringbuffer for the commands that have to be passed to the process callback
	command_ringbuffer commands;
	ringbuffer<char> acknowledgements;

	//! disposable vector holding generators
	disposable_generator_vector_ptr gens;

	//! a single generator to audit a sample
	disposable_generator_ptr auditor_gen;

	jack_client_t *jack_client;
	jack_port_t *out_0;
	jack_port_t *out_1;
	jack_port_t *midi_in;

	engine() 
	: 
		commands(1024),
		acknowledgements(64),
		gens(disposable_generator_vector::create(generator_vector(128))) 
	{
		heap *h = heap::get();

		jack_client = jack_client_open("jass", JackNullOption, 0);
		out_0 = jack_port_register(jack_client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		out_1 = jack_port_register(jack_client, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		midi_in = jack_port_register(jack_client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
		jack_set_process_callback(jack_client, process_callback, (void*)this);
		jack_activate(jack_client);
	}

	~engine() {
		jack_deactivate(jack_client);
		jack_client_close(jack_client);
	}

	void process(jack_nframes_t nframes) {
		float *out_0_buf = (float*)jack_port_get_buffer(out_0, nframes);
		float *out_1_buf = (float*)jack_port_get_buffer(out_1, nframes);
		void *midi_in_buf = jack_port_get_buffer(midi_in, nframes);

		//! zero the buffers first
		std::fill(out_0_buf, out_0_buf + nframes, 0);
		std::fill(out_1_buf, out_1_buf + nframes, 0);

		//! Execute commands passed in through ringbuffer
		while(commands.can_read()) { commands.read()(); }

		//! Send ack to (possibly) enable the GUI again
		if (!acknowledgements.can_write()) std::cout << "ack buffer full" << std::endl;
		else acknowledgements.write(0);

		for (unsigned int i = 0; i < gens->t.size(); ++i) {
			if (gens->t[i].get()) {
				//std::cout << "." << std::endl;
				gens->t[i]->t.process(out_0_buf, out_1_buf, nframes);
			}
		}
		//! Synthesize
	}
};


#endif
