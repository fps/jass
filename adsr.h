#ifndef JASS_ADSR_HH
#define JASS_ADSR_HH

#include <algorithm>

inline double adsr_attack(double attack, double decay, double sustain, double release, double time) {
	if (time <= attack) {
		return time/attack;
	} else if (time > attack && time <= attack + decay) {
		double time_in_decay = time - attack;
		return (1.0 - time_in_decay/decay) * (1.0 - sustain) + sustain;
	} else if (time > (attack + decay)) {
		return sustain;
	}
}

//! If release_time > time give envelope after note on, if release_time <= time give envelope after note off at release_time
inline double adsr(double attack, double decay, double sustain, double release, double time, double release_time) {
	if (release_time > time) return adsr_attack(attack, decay, sustain, release, time);

	//! Ok, we are in release time
	double last_gain = adsr_attack(attack, decay,sustain, release, release_time);
	return std::max(
				last_gain * (1.0 - ((time - release_time)/release)), 
				0.0
	);
}

#endif


