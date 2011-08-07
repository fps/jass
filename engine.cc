#include "engine.h"

extern "C" {
	int process_callback(jack_nframes_t frames, void *p) {
		((engine*)p)->process(frames);
		return 0;
	}
	void session_callback(jack_session_event_t *event, void *p) {
		((engine*)p)->session_callback(event);
	}
}
