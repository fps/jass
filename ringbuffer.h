#ifndef RINGBUFFER_HH
#define RINGBUFFER_HH

#include <jack/ringbuffer.h>
#include <memory>

/**
	T needs to be a default constructable type.. And it has to have valid
	copy constructor/assignment operator.
	
	Note that if you want to use this ringbuffer in a realtime context then the
	assignment operator and copy constructor must be realtime safe.

	Note that this class needs to construct n objects of type T (with n == size)  so 
	that the places in the ringbuffer become assignable

	Note that the objects remain in the ringbuffer until they are overwritten again
	when the ringbuffer returns to the current position the next time around. I.e. a
	read() does not assign a T() to the read object that was read.
*/
template <class T> 
struct ringbuffer {
	unsigned int size;

	jack_ringbuffer_t *jack_ringbuffer;
	
	std::vector<T> elements;

	size_t elements_write_pos;

	ringbuffer(unsigned int size) : 
		size(size),
		elements(size), 
		elements_write_pos(0)
	{
		jack_ringbuffer = jack_ringbuffer_create(sizeof(size_t) * size);
	}

	~ringbuffer() {
		jack_ringbuffer_free(jack_ringbuffer);
	}

	bool can_write() {
		if (jack_ringbuffer_write_space(jack_ringbuffer) >= sizeof(size_t)) {
			return true;
		}

		return false;
	}

	void write(const T &t) {
		elements[elements_write_pos] = t;
		jack_ringbuffer_write(jack_ringbuffer, (char *)&elements_write_pos, sizeof(size_t));
		++elements_write_pos;
		elements_write_pos = elements_write_pos % size;
	}

	bool can_read() {
		if (jack_ringbuffer_read_space(jack_ringbuffer) >= sizeof(size_t)) {
			return true;
		}

		return false;
	}

	void read_advance()
	{
		jack_ringbuffer_read_advance(jack_ringbuffer, sizeof(size_t));
	}
	
	T read() {
		size_t n;
		jack_ringbuffer_read(jack_ringbuffer, (char*)&n, sizeof(size_t));
		return elements[n];
	}

	/**
		The reference returned here must only be used until the next
		read() call.
	*/
	T& snoop() {
		size_t n;	
		jack_ringbuffer_peek(jack_ringbuffer, (char*)&n, sizeof(size_t));
		return elements[n];
	}
};

#endif
