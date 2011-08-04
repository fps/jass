#ifndef GENERATOR_HH
#define GENERATOR_HH

#include <vector>
#include <iostream>

#include "disposable.h"

struct generator {
	disposable_sample_ptr sample_ptr;

	//! the channel this generator listens on
	unsigned int channel;

	//! the tuning note
	unsigned int note_a;

	//! lowest note tp react to
	unsigned int low_note;

	//! highest note to react to
	unsigned int high_note;

	//! the lowest velocity to react to
	unsigned int low_velocity;

	//! the highest velocity to react to
	unsigned int high_velocity;

	//! resulting velocity is velocity_factor * (velocity - high_velocity)/(high_velocity - low_velocity)
	double velocity_factor;

	virtual ~generator() { std::cout << "~generator()" << std::endl; }

	generator() { std::cout << "generator()" << std::endl; }

	generator(const generator& g) {
		std::cout << "generator(const generator& g)" << std::endl;
		*this = g;
	}
};

typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
