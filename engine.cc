#include "engine.h"

extern "C" {
	int process_callback(jack_nframes_t frames, void *p) {
		((engine*)p)->process(frames);
		return 0;
	}
}
