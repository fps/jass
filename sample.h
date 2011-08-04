#ifndef SAMPLE_HH
#define SAMPLE_HH

#include <jack/jack.h>

#include "disposable.h"

struct sample {
		float *left;
};

typedef boost::shared_ptr<disposable<sample> > disposable_sample_ptr;

#endif
