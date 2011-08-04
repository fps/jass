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

	virtual ~generator()
	{ 
		std::cout << "~generator()" << std::endl; 
	}

	generator()  :
		channel(0),
		note_a(64),
		low_note(0),
		high_note(127),
		low_velocity(0),
		high_velocity(127),
		velocity_factor(1.0)
	{ std::cout << "generator()" << std::endl; }

	generator(const generator& g) {
		std::cout << "generator(const generator& g)" << std::endl;
		*this = g;
	}
};

typedef disposable<generator> disposable_generator;
typedef boost::shared_ptr<disposable<generator> > disposable_generator_ptr;

#endif
