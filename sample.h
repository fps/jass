#ifndef SAMPLE_HH
#define SAMPLE_HH

#include <jack/jack.h>

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>

#include <sndfile.h>

#include "disposable.h"


struct sample {
	std::vector<float> data_0;
	std::vector<float> data_1;

	std::string file_name;

	sample(const std::string &file_name, jack_nframes_t samplerate) :
		file_name(file_name)
	{
		SF_INFO sf_info;
		sf_info.format = 0;
		SNDFILE* snd_file = sf_open(file_name.c_str(), SFM_READ, &sf_info);
		if (snd_file == 0) {
			throw std::runtime_error("Couldn't read sound file: " + file_name);
		}

		data_0.resize(sf_info.frames);
		data_1.resize(sf_info.frames);

		if (sf_info.channels != 1 && sf_info.channels != 2) throw std::runtime_error("wrong channel count");

		std::vector<float> frames(sf_info.channels * sf_info.frames);

		std::cout << "read: " << sf_readf_float(snd_file, &frames[0], sf_info.frames) << " samples from " << file_name << std::endl;

		if (sf_info.channels == 1) {
			std::copy(frames.begin(), frames.end(), data_0.begin());
			std::copy(frames.begin(), frames.end(), data_0.begin());
		}

		if (sf_info.channels == 2) {
			for (unsigned int i = 0; i < sf_info.frames; ++i) {
				data_0[i] = frames[2 * i];
				data_1[i] = frames[2 * i + 1];
			}
		}
	}
};

typedef disposable<sample> disposable_sample;
typedef boost::shared_ptr<disposable<sample> > disposable_sample_ptr;

#endif
