#ifndef RINGBUFFER_HH
#define RINGBUFFER_HH

#include <jack/ringbuffer.h>
#include <memory>

/**
	T needs to be a default constructable type.. And it has to have valid
	copy constructor/assignment operator

	Note that this class needs to construct n objects of type T (with n == size)  so 
	that the places in the ringbuffer become assignable
*/
template <class T> 
struct ringbuffer {
	unsigned int size;

	jack_ringbuffer_t *jack_ringbuffer;

	ringbuffer(unsigned int size) : size(size) {
		jack_ringbuffer = jack_ringbuffer_create(sizeof(T) * size);

		for (int i = 0; i < size; ++i) {
			T *t = new (jack_ringbuffer->buf + sizeof(T) * i) T();
		}
	}

	~ringbuffer() {
		for (int i = 0; i < size; ++i) {
			((T*)(jack_ringbuffer->buf + sizeof(T) * i))->~T();
		}

		jack_ringbuffer_free(jack_ringbuffer);
	}

	bool can_write() {
		if (jack_ringbuffer_write_space(jack_ringbuffer) >= sizeof(T)) {
			return true;
		}

		return false;
	}

	void write(const T &t) {
		jack_ringbuffer_data_t rb_data[2];
		jack_ringbuffer_get_write_vector(jack_ringbuffer, rb_data);
		*((T*)rb_data->buf) = t;
		jack_ringbuffer_write_advance(jack_ringbuffer, sizeof(T));
	}

	bool can_read() {
		if (jack_ringbuffer_read_space(jack_ringbuffer) >= sizeof(T)) {
			return true;
		}

		return false;
	}

	T read() {
		jack_ringbuffer_data_t rb_data[2];
		jack_ringbuffer_get_read_vector(jack_ringbuffer, rb_data);
		jack_ringbuffer_read_advance(jack_ringbuffer, sizeof(T));

		//! read T from buffer
		T t = *((T*)rb_data->buf);

		//! reset place in buffer to default T()
		*((T*)rb_data->buf) = T();
		return t;
	}
};

#endif
