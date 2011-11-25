#include "engine.h"

engine *engine::instance = 0;

extern "C" {
	int process_callback(jack_nframes_t frames, void *p) {
		((engine*)p)->process(frames);
		return 0;
	}

	void shutdown_callback(void *arg) {
		((engine*)arg)->shutdown();
	}


#ifndef NO_JACK_SESSION
	void session_callback(jack_session_event_t *event, void *p) {
		((engine*)p)->session_callback(event);
	}
#endif
}
