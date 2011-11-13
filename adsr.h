#ifndef JASS_ADSR_HH
#define JASS_ADSR_HH

#include <algorithm>

#define JASS_ADSR_LIMIT (-70.0)

//! sustain has to be a value between limit and 0
inline double adsr_attack(double attack, double decay, double sustain, double release, double time) {
	if (time <= attack) {
		return JASS_ADSR_LIMIT + -JASS_ADSR_LIMIT * (time/attack);
	} else if (time > attack && time <= attack + decay) {
		double time_in_decay = time - attack;
		return time_in_decay/decay * sustain;
	} else if (time > (attack + decay)) {
		return sustain;
	}
}

//! If release_time > time give envelope after note on, if release_time <= time give envelope after note off at release_time
inline double adsr(double attack, double decay, double sustain, double release, double time, double release_time) {
	if (time < release_time) return adsr_attack(attack, decay, sustain, release, time);

	//! Ok, we are in release time
	double release_gain = adsr_attack(attack, decay, sustain, release, release_time);
	return std::max(
		release_gain + ((time - release_time)/release) * (JASS_ADSR_LIMIT - release_gain), 
		JASS_ADSR_LIMIT
	);
}

#endif


