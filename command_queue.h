#ifndef JASS_COMMAND_QUEUE_HH
#define JASS_COMMAND_QUEUE_HH

#include <boost/function.hpp>
#include <QApplication>
#include <iostream>

#include "ringbuffer.h"

typedef ringbuffer<boost::function<void(void)> > command_ringbuffer;

struct command_queue {
	//! The ringbuffer for the commands that have to be passed to the process callback
	command_ringbuffer commands;

	//! When the engine is done processing a command that possibly alters references, it will signal completion by writing a 0 in this ringbuffer
	ringbuffer<char> acknowledgements;

	//! Commands to be executed in the main app's thread..
	command_ringbuffer deferred_commands;
	int outstanding_acks;

	//! Write command without blocking the GUI
	void write_command(boost::function<void(void)> f) {
		if (commands.can_write()) {
			++outstanding_acks;
			commands.write(f);
		}
	}

	void write_blocking_command(boost::function<void(void)> f) {
		//QApplication::setEnabled(false);
		//! Will be reenabled by acknowledgement 
		if (commands.can_write()) {
			++outstanding_acks;
			//setEnabled(false);
			commands.write(f);
		}
	}


	void check_acknowledgements() {
		while(acknowledgements.can_read()) { 
			acknowledgements.read(); 
			--outstanding_acks; 
		}

		assert(outstanding_acks >= 0);

		if (outstanding_acks == 0) {
			while(deferred_commands.can_read()) {
				// std::cout << "deferred" << std::endl;
				deferred_commands.read()();
			}

//			setEnabled(true);
		}
		QApplication::processEvents();
	}


	void write(boost::function<void()> cmd) {
		commands.write(cmd);
	}

	command_queue(unsigned int cmds_size = 1024, unsigned int acks_size = 1024, unsigned deffered_cmds_size = 1024) :
		commands(cmds_size),
		acknowledgements(acks_size),
		deferred_commands(deffered_cmds_size),
		outstanding_acks(0)
	{

	}
};

#endif
