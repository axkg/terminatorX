/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2020  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_audiodevice.h
 
    Description: Header to tX_mastergui.cc
*/    

#ifndef _h_tx_audiodevice
#define _h_tx_audiodevice 1

#include "tX_types.h"
#include "tX_global.h"
#include "pthread.h"
#include <config.h>

#include <sys/time.h>

#define NON_RT_BUFF 12

#ifdef USE_ALSA
#include <alsa/asoundlib.h>
#endif

#ifdef USE_JACK
#include <jack/jack.h>
#endif

#ifdef USE_PULSE
#include <pulse/pulseaudio.h>
#endif

class tX_engine;

class tX_audiodevice
{
	protected:
	int samples_per_buffer;
	int16_t *sample_buffer[2];
	int current_buffer;
	int buffer_pos;
	int vtt_buffer_size;
	tX_engine *engine;
	bool is_open;
	
	int sample_rate;
	tX_audiodevice();
	
	public:
	int get_buffersize() { return samples_per_buffer; } /* call only valid *after* open() */
	int get_sample_rate() { return sample_rate; }
	
	virtual int open()=0;
	virtual int close()=0;
	
	void fill_buffer(int16_t *target_buffer, int16_t *next_target_buffer);

	bool get_is_open() { return is_open; }
	virtual void start();	
	virtual void play(int16_t*)=0; /* play blocked */
	virtual ~tX_audiodevice() {}
};


#ifdef USE_OSS

class tX_audiodevice_oss : public tX_audiodevice
{
	int fd;
	int blocksize;	

	public:
	virtual int open();
	virtual int close();
	virtual void play(int16_t*);
	
	tX_audiodevice_oss();
};

#endif


#ifdef USE_ALSA

class tX_audiodevice_alsa : public tX_audiodevice
{
	snd_pcm_t *pcm_handle;
	snd_pcm_uframes_t period_size;
	
	public:
	virtual int open();
	virtual int close();
	virtual void play(int16_t*);
	
	tX_audiodevice_alsa();
};

#endif

#ifdef USE_PULSE

class tX_audiodevice_pulse : public tX_audiodevice
{
	pa_mainloop *mainloop;
	pa_mainloop_api *mainloop_api;
	pa_context *context;
	pa_stream *stream;
	int16_t *overrun_buffer;
	unsigned int samples_in_overrun_buffer;

	public:
	virtual int open();
	virtual int close();
	virtual void start();
	virtual void play(int16_t*);

	tX_audiodevice_pulse();
	~tX_audiodevice_pulse();
	
	private:
	// context callbacks
	static void wrap_context_state_callback(pa_context *context, void *userdata);
	void context_state_callback(pa_context *context);

	static void wrap_context_drain_complete_callback(pa_context *context, void *userdata);
	void context_drain_complete_callback(pa_context *context);

	// stream callbacks
	static void wrap_stream_started_callback(pa_stream *stream, void *userdata);
	void stream_started_callback(pa_stream *stream);

	static void wrap_stream_underflow_callback(pa_stream *stream, void *userdata);
	void stream_underflow_callback(pa_stream *stream);
	
	static void wrap_stream_overflow_callback(pa_stream *stream, void *userdata);
	void stream_overflow_callback(pa_stream *stream);
	
	static void wrap_stream_drain_complete_callback(pa_stream *stream, int success, void *userdata);
	void stream_drain_complete_callback(pa_stream *stream, int success);

	static void wrap_stream_trigger_success_callback(pa_stream *stream, int success, void *userdata);
	void stream_trigger_success_callback(pa_stream *stream, int success);

	static void wrap_stream_write_callback(pa_stream *stream, size_t length, void *userdata);
	void stream_write_callback(pa_stream *stream, size_t length);
};

#endif

#ifdef USE_JACK

class tX_jack_client;

class tX_audiodevice_jack : public tX_audiodevice
{
	private:
	tX_jack_client *client;
	jack_default_audio_sample_t *overrun_buffer;
	unsigned int samples_in_overrun_buffer;
	
	public:
	virtual int open();
	virtual int close();
	virtual void play(int16_t*);
	virtual void start();
	void fill_frames(jack_default_audio_sample_t *left, jack_default_audio_sample_t *right, jack_nframes_t nframes);
	
	tX_audiodevice_jack();	
};

class tX_jack_client
{
	public:
	static tX_jack_client *get_instance() { return &instance; };
	bool init();
	~tX_jack_client();
	
	private:
	tX_jack_client();	
	static tX_jack_client instance;
	static void error(const char *desc);
	static int srate(jack_nframes_t nframes, void *arg);
	static void shutdown(void *arg);
	static int process(jack_nframes_t nframes, void *arg);
	
	jack_client_t *client;	
	tX_audiodevice_jack *device;
	jack_port_t *left_port;
	jack_port_t *right_port;
	bool jack_shutdown;
	bool client_initialized;
	int play(jack_nframes_t nframes);
	
	public:
	int get_sample_rate();
	bool get_jack_shutdown() { return jack_shutdown; }
	void set_device(tX_audiodevice_jack *dev) { device=dev; }
};

#endif

#endif
