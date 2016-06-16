/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
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
 
    File: tX_aduiodevice.cc
 
    Description: Implements Audiodevice handling... 
*/    

#define ALSA_PCM_NEW_HW_PARAMS_API

#include "tX_audiodevice.h"
#include "tX_vtt.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <config.h>
#include <string.h>

#include "tX_endian.h"

#ifndef __USE_XOPEN
#	define __USE_XOPEN // we need this for swab()
#	include <unistd.h>
#	undef __USE_XOPEN
#else
#	include <unistd.h>
#endif

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "tX_engine.h"

tX_audiodevice :: tX_audiodevice() : samples_per_buffer(0),
current_buffer(0), buffer_pos(0), is_open(false)
{
	sample_buffer[0]=NULL;
	sample_buffer[1]=NULL;
	engine=tX_engine::get_instance();
}

void tX_audiodevice :: start() {
	sample_buffer[0]=new int16_t[samples_per_buffer*2];
	sample_buffer[1]=new int16_t[samples_per_buffer*2];
	int current=0;
	vtt_buffer_size=vtt_class::get_mix_buffer_size()<<1;
	
	buffer_pos=0;
	
	while (!engine->is_stopped()) {
		current=current ? 0 : 1;
		
		int16_t *current_buffer=sample_buffer[current];
		int16_t *next_buffer=sample_buffer[current ? 0 : 1];
		
		fill_buffer(current_buffer, next_buffer);
		play(current_buffer);
	}
	
	delete [] sample_buffer[0];
	delete [] sample_buffer[1];
}

void tX_audiodevice :: fill_buffer(int16_t *target_buffer, int16_t *next_target_buffer) {
	int prefill=0;
	
	while (buffer_pos <= samples_per_buffer) {
		int16_t *data=engine->render_cycle();
		
		int rest=(buffer_pos+vtt_buffer_size)-samples_per_buffer;
		
		if (rest<=0) {
			memcpy(&target_buffer[buffer_pos], data, vtt_buffer_size << 1);
		} else {
			memcpy(&target_buffer[buffer_pos], data, (vtt_buffer_size-rest) << 1);
			memcpy(next_target_buffer, &data[vtt_buffer_size-rest], rest << 1);
			prefill=rest;
		}
		
		buffer_pos+=vtt_buffer_size;
	}
	
	buffer_pos=prefill;
}

/* Driver Specific Code follows.. */

#ifdef USE_OSS

int tX_audiodevice_oss :: open()
{
	int i=0;
	int p;
	int buff_cfg;

	if (fd) return (1);
	fd=::open(globals.oss_device, O_WRONLY, 0);
	
	if (fd==-1) {
		tX_error("tX_audiodevice_oss::open() can't open device: %s", strerror(errno));
		return -1;
	}
	
	is_open=true;
	
	/* setting buffer size */	
	buff_cfg=(globals.oss_buff_no<<16) | globals.oss_buff_size;
	p=buff_cfg;

	tX_debug("tX_audiodevice_oss::open() - buff_no: %i, buff_size: %i, buff_cfg: %08x", globals.oss_buff_no, globals.oss_buff_size, buff_cfg);
		
	i = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &p);
	ioctl(fd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
	p =  16;
	i +=  ioctl(fd, SOUND_PCM_WRITE_BITS, &p);

	/* STEREO :) */
	
	p =  2;
	i += ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

	p =  globals.oss_samplerate;
	i += ioctl(fd, SOUND_PCM_WRITE_RATE, &p);
	
	sample_rate=globals.oss_samplerate;
	
	/* Figure actual blocksize.. */
	
	i += ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);

	samples_per_buffer=blocksize/sizeof(int16_t);

	tX_debug("tX_adudiodevice_oss::open() - blocksize: %i, samples_per_buffer: %i", blocksize, samples_per_buffer);
	
	ioctl(fd, SNDCTL_DSP_SYNC, 0);

	return i;
}

int tX_audiodevice_oss :: close()
{
	if (!fd) {	
		return(1);		
	}
	is_open=false;
	::close(fd);
	fd=0;
	blocksize=0;
		
	return 0;
}

tX_audiodevice_oss :: tX_audiodevice_oss() : tX_audiodevice(),
fd(0), blocksize(0) {}

void tX_audiodevice_oss :: play(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples_per_buffer);
#endif
	int res=write(fd, buffer, blocksize);	
	if (res==-1) {
		tX_error("failed to write to audiodevice: %s", strerror(errno));
		exit(-1);
	}
}

#endif //USE_OSS

#ifdef USE_ALSA

#define tX_abs(x) (x<0 ? -x : x)

int tX_audiodevice_alsa :: open()
{
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_hw_params_t *hw_params;
	char pcm_name[64];
	char *pos;
	
	strncpy(pcm_name, globals.alsa_device_id, sizeof(pcm_name));
	if ((pos = strchr(pcm_name, '#')) != NULL) *pos = 0;
	
	if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
		tX_error("ALSA: Failed to access PCM device \"%s\"", pcm_name);
		return -1;
	}
	
	is_open=true;

	snd_pcm_hw_params_alloca(&hw_params);	
	
	if (snd_pcm_hw_params_any(pcm_handle, hw_params) < 0) {
		tX_error("ALSA: Failed to configure PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
  	
	/* Setting INTERLEAVED stereo... */
#ifdef USE_ALSA_MEMCPY
	if (snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
#else
	if (snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED) < 0) {
#endif	
		tX_error("ALSA: Failed to set interleaved access for PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	/* Make it 16 Bit native endian */
	if (snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16) < 0) {
		tX_error("ALSA: Error setting 16 Bit sample width for PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	/* Setting sampling rate */
	unsigned int hw_rate=(unsigned int)globals.alsa_samplerate;
	int dir;
	
	if (snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &hw_rate, &dir) < 0) {
		tX_error("ALSA: Failed setting sample rate: %i", globals.alsa_samplerate);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	if (dir != 0) {
		if (tX_abs(globals.alsa_samplerate - hw_rate) > 2) {
			tX_warning("ALSA: The PCM device \"%s\" doesn\'t support %i Hz playback - using %i instead", pcm_name, globals.alsa_samplerate, hw_rate);
		}
	}	

	sample_rate=hw_rate;
	
	/* Using stereo output */
	if (snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2) < 0) {
		tX_error("ALSA: PCM device \"%s\" does not support stereo operation", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	unsigned int buffer_time=globals.alsa_buffer_time;
	unsigned int period_time=globals.alsa_period_time;
	
	if (snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hw_params, &buffer_time, &dir) < 0) {
		tX_error("ALSA: failed to set the buffer time opf %i usecs", globals.alsa_buffer_time);
		return -1;
	}

	long unsigned int buffer_size;

	if (snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size) < 0) {
		tX_error("ALSA: failed to retrieve buffer size");
		return -1;
	}
	
	tX_debug("ALSA: buffer size is %lu", buffer_size);
	
	if (snd_pcm_hw_params_set_period_time_near(pcm_handle, hw_params, &period_time, &dir) < 0) {
		tX_error("ALSA: failed to set period time %i", globals.alsa_period_time);
		return -1;
	}
	
	if (snd_pcm_hw_params_get_period_size(hw_params, &period_size, &dir)<0) {
		tX_error("ALSA: failed to retrieve period size");
		return -1;
	}
	
	samples_per_buffer=period_size;
	
	/* Apply all that setup work.. */
	if (snd_pcm_hw_params(pcm_handle, hw_params) < 0) {
		tX_error("ALSA: Failed to apply settings to PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	if (globals.alsa_free_hwstats) {
		snd_pcm_hw_params_free (hw_params);
	}
	
	return 0; //snd_pcm_prepare(pcm_handle);
}

int tX_audiodevice_alsa :: close()
{
	if (is_open) {
		snd_pcm_close(pcm_handle);
	}
	is_open=false;
	
	return 0;
}

tX_audiodevice_alsa :: tX_audiodevice_alsa() : tX_audiodevice(),
pcm_handle(NULL) {}

void tX_audiodevice_alsa :: play(int16_t *buffer)
{
	int underrun_ctr=0;
	snd_pcm_sframes_t pcmreturn;
	
#ifdef USE_ALSA_MEMCPY
	pcmreturn = snd_pcm_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#else	
	pcmreturn = snd_pcm_mmap_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#endif
	
	while (pcmreturn==-EPIPE) {
		snd_pcm_prepare(pcm_handle);
		
#ifdef USE_ALSA_MEMCPY
		pcmreturn = snd_pcm_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#else	
		pcmreturn = snd_pcm_mmap_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#endif
		underrun_ctr++;
		if (underrun_ctr>100) {
			tX_error("tX_audiodevice_alsa::play() more than 10 EPIPE cycles. Giving up.");
			break;
		}
		//tX_warning("ALSA: ** buffer underrun **");
	}
	
	if (pcmreturn<0) {
		printf("snd_pcm_writei says: %s.\n", strerror(-1*pcmreturn));
	}
}

#endif //USE_ALSA

#ifdef USE_PULSE
#include <pulse/error.h>

void tX_audiodevice_pulse::wrap_stream_started_callback(pa_stream *stream, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_started_callback(stream);
}

void tX_audiodevice_pulse::wrap_stream_underflow_callback(pa_stream *stream, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_underflow_callback(stream);
}

void tX_audiodevice_pulse::wrap_stream_overflow_callback(pa_stream *stream, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_overflow_callback(stream);
}

void tX_audiodevice_pulse::wrap_stream_drain_complete_callback(pa_stream *stream, int success, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_drain_complete_callback(stream, success);
}

void tX_audiodevice_pulse::wrap_stream_trigger_success_callback(pa_stream *stream, int success, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_trigger_success_callback(stream, success);
}

void tX_audiodevice_pulse::wrap_stream_write_callback(pa_stream *stream, size_t length, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->stream_write_callback(stream, length);
}

void tX_audiodevice_pulse::wrap_context_state_callback(pa_context *context, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->context_state_callback(context);
}

void tX_audiodevice_pulse::context_state_callback(pa_context *context) {
	pa_context_state_t state;

	state = pa_context_get_state(context);

	tX_debug("pulseaudio context state: %i", state);
	switch (state) {
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			if (!engine->is_stopped()) {
				tX_error("pulseaudio disconnected");
			}
			break;

		case PA_CONTEXT_READY:
			pa_sample_spec spec = {
				.format = PA_SAMPLE_S16LE,
				.rate = 44100,
				.channels = 2
		 	};
	
			pa_buffer_attr attr = {
				.maxlength = (uint32_t) -1,
				.tlength = (uint32_t) (globals.pulse_buffer_length * 4), // 2 bytes per sample, 2 channels
				.prebuf = (uint32_t) -1,
				.minreq = (uint32_t) -1,
				.fragsize = (uint32_t) -1
			};

			pa_stream_flags_t flags = PA_STREAM_ADJUST_LATENCY;

			if ((stream = pa_stream_new(context, "terminatorX", &spec, NULL))) {
				tX_debug("pulseaudio stream created");
				//pa_stream_set_started_callback(stream, tX_audiodevice_pulse::wrap_stream_started_callback, this);
				//pa_stream_set_underflow_callback(stream, tX_audiodevice_pulse::wrap_stream_underflow_callback, this);
				pa_stream_set_overflow_callback(stream, tX_audiodevice_pulse::wrap_stream_overflow_callback, this);
				pa_stream_set_write_callback(stream, tX_audiodevice_pulse::wrap_stream_write_callback, this);

				if (pa_stream_connect_playback(stream, NULL, &attr, flags, NULL, NULL) >= 0) {
					// start the playback.
					pa_stream_trigger(stream, tX_audiodevice_pulse::wrap_stream_trigger_success_callback, this);
				} else {
					tX_error("Failed to connect pulseaudio stream playback: %s", pa_strerror(pa_context_errno(context)));
				}
			}	else {
				tX_error("Failed to create pulseaudio stream: %s", pa_strerror(pa_context_errno(context)));
			}
			break;
	}
}

void tX_audiodevice_pulse::wrap_context_drain_complete_callback(pa_context *context, void *userdata) {
	tX_audiodevice_pulse *device = (tX_audiodevice_pulse *) userdata;
	device->context_drain_complete_callback(context);
}

void tX_audiodevice_pulse::context_drain_complete_callback(pa_context *context) {
	pa_context_disconnect(context);
	is_open = false;
	pa_mainloop_quit(mainloop, 0);
}

void tX_audiodevice_pulse::stream_started_callback(pa_stream *stream) {
	tX_debug("pulseaudio stream started");
}

void tX_audiodevice_pulse::stream_underflow_callback(pa_stream *stream) {
	tX_debug("pulseaudio stream underflow");
}

void tX_audiodevice_pulse::stream_overflow_callback(pa_stream *stream) {
	tX_debug("pulseaudio stream overflow");
}

void tX_audiodevice_pulse::stream_trigger_success_callback(pa_stream *stream, int success) {
	tX_debug("pulseaudio trigger success %i", success);
}

void tX_audiodevice_pulse::stream_drain_complete_callback(pa_stream *stream, int success) {
	if (!success) {
		tX_debug("pulseaudio drain failed %s", pa_strerror(pa_context_errno(context)));
	} else {
		pa_operation *operation;
		pa_stream_disconnect(stream);
		pa_stream_unref(stream);
		stream = NULL;

		if (!(operation = pa_context_drain(context, tX_audiodevice_pulse::wrap_context_drain_complete_callback, this))) {
			pa_context_disconnect(context);
			is_open = false;
			pa_mainloop_quit(mainloop, -1);
		} else {
			pa_operation_unref(operation);
		}
	}
}

void tX_audiodevice_pulse::stream_write_callback(pa_stream *stream, size_t length) {
	size_t sample_length = length/2;

	if (engine->is_stopped()) {
		tX_debug("pulseaudio write callback trying to disconnect pulseaudio");
		pa_operation *operation;
		pa_stream_set_write_callback(stream, NULL, NULL);
		if (!(operation = pa_stream_drain(stream, tX_audiodevice_pulse::wrap_stream_drain_complete_callback, this))) {
			tX_error("pulseaudio failed to initiate drain %s", pa_strerror(pa_context_errno(context)));
		}
		pa_operation_unref(operation);
	} else {
		//tX_debug("pulseaudio write %i samples", sample_length);
		unsigned int outbuffer_pos=0;
		unsigned int sample;
	
		// re-alloc outbuffer only when not yet allocated or allocated buffer smaller
		// than the chunk requested by pulseaudio
/*		if (!outbuffer || (outbuffer_length < sample_length)) {
			if (outbuffer) {
				pa_xfree(outbuffer);
			}
			outbuffer = (int16_t* ) pa_xmalloc(length);
			outbuffer_length = sample_length;
		}
*/

		int16_t *outbuffer = NULL;
		size_t outbuffer_bytes = length;
  		pa_stream_begin_write(stream, (void **) &outbuffer, &outbuffer_bytes);

	  	//tX_debug("begin write %i %i", outbuffer_bytes, length)

		if (samples_in_overrun_buffer) {
			for (sample=0; ((sample<samples_in_overrun_buffer) && (outbuffer_pos<sample_length));) {
				outbuffer[outbuffer_pos++]=overrun_buffer[sample++];
			}
		}
	
		while (outbuffer_pos<sample_length) {
			//tX_debug("render %i %i %i", outbuffer_pos, sample_length, vtt_class::samples_in_mix_buffer);
			int16_t *data=engine->render_cycle();
		
			for (sample=0; ((sample<(unsigned int) vtt_class::samples_in_mix_buffer) && (outbuffer_pos<sample_length));) {
				outbuffer[outbuffer_pos++]=data[sample++];
			}
		
			if (sample<(unsigned int) vtt_class::samples_in_mix_buffer) {
				samples_in_overrun_buffer=vtt_class::samples_in_mix_buffer-sample;
				/* There's more data in the mixbuffer... */
				memcpy(overrun_buffer, &data[sample], sizeof(int16_t) * samples_in_overrun_buffer);
			} else {
				samples_in_overrun_buffer=0;
			}
		}
	
		//tX_debug("write %i bytes", length);
		if (pa_stream_write(stream, (uint8_t *) outbuffer, length, NULL, 0, PA_SEEK_RELATIVE) < 0) {
			tX_error("pulseaudio error writing to stream: %s", pa_strerror(pa_context_errno(context)));
		}
		outbuffer = NULL;
	}
}

void tX_audiodevice_pulse::start() {
	overrun_buffer=new int16_t[vtt_class::samples_in_mix_buffer];
	samples_in_overrun_buffer = 0;
	int result;
	
	tX_debug("handover flow control to pulseaudio");

	while (!engine->is_stopped()) {
		pa_mainloop_run(mainloop, &result);
		tX_debug("pulseaudio mainloop has terminated: %i", result);
	}	
	
	delete [] overrun_buffer;
}

int tX_audiodevice_pulse::open() {
	mainloop = pa_mainloop_new();
	mainloop_api = pa_mainloop_get_api(mainloop);
	context = pa_context_new(mainloop_api, "terminatorX");
	pa_context_flags_t flags = PA_CONTEXT_NOFLAGS;
	pa_context_connect(context, NULL, flags, NULL);
	pa_context_set_state_callback(context, tX_audiodevice_pulse::wrap_context_state_callback, this);

	sample_rate=44100;
	tX_debug("pulseaudio opened.");

	is_open = true;
	return 0;
}

int tX_audiodevice_pulse :: close() {
	return 0;
}

tX_audiodevice_pulse :: tX_audiodevice_pulse() : tX_audiodevice(),
mainloop(NULL), mainloop_api(NULL), context(NULL), stream(NULL)  {
}

void tX_audiodevice_pulse :: play(int16_t *buffer) {
	tX_error("tX_audiodevice_pulse::play()");
}

tX_audiodevice_pulse :: ~tX_audiodevice_pulse() {
}

#endif //USE_PULSE

#ifdef USE_JACK

tX_jack_client tX_jack_client::instance;


bool tX_jack_client::init()
{
	if (!client_initialized) {
		if ((client=jack_client_open("terminatorX", (jack_options_t) NULL, NULL))==0) {
			tX_error("tX_jack_client() -> failed to connect to jackd.");
		} else {
			client_initialized = true;
			const char **ports;
			
			/* Setting up jack callbacks... */		
			jack_set_process_callback(client, tX_jack_client::process, NULL);
			jack_set_sample_rate_callback(client, tX_jack_client::srate, NULL);		
			jack_on_shutdown (client, tX_jack_client::shutdown, NULL);
			
			/* Creating the port... */
			left_port = jack_port_register (client, "output_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			right_port = jack_port_register (client, "output_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		
			/* Action... */
			jack_activate(client);
			
			/* Connect some ports... */
			if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
				tX_error("tX_jack_client() no ports to connect to found. Connect manually.");
			} else if (ports[0] && ports[1]) {
				if (jack_connect (client, jack_port_name(left_port), ports[0])) {
					tX_error("tX_jack_client() failed to connect left port.");
				}
				if (jack_connect (client, jack_port_name(right_port), ports[1])) {
					tX_error("tX_jack_client() failed to connect right port.");
				}
				free (ports);
			}
		}
	}
	
	return client_initialized;
}

tX_jack_client::tX_jack_client():device(NULL),jack_shutdown(false),client_initialized(false)
{
	jack_set_error_function(tX_jack_client::error);
}

tX_jack_client::~tX_jack_client()
{
	if (client) jack_client_close(client);
}

void tX_jack_client::error(const char *desc)
{
	tX_error("jack error: %s.", desc);
}

int tX_jack_client::srate(jack_nframes_t nframes, void *arg)
{
	tX_error("tX_jack_client::srate() jack changed samplerate - ignored.");
	return 0;
}

void tX_jack_client::shutdown(void *arg)
{
	tX_error("tX_jack_client::shutdown() jack daemon has shut down. Bad!");
	instance.jack_shutdown=true;
}

int tX_jack_client::process(jack_nframes_t nframes, void *arg)
{
	return instance.play(nframes);
	
	/* Hmm, what to do in such a case? */
	return 0;
}

int tX_jack_client::play(jack_nframes_t nframes)
{
	jack_default_audio_sample_t *left = (jack_default_audio_sample_t *) jack_port_get_buffer (left_port, nframes);
	jack_default_audio_sample_t *right = (jack_default_audio_sample_t *) jack_port_get_buffer (right_port, nframes);

	if (device) {
		device->fill_frames(left, right, nframes);
	} else {
		memset(left, 0, sizeof (jack_default_audio_sample_t) * nframes);
		memset(right, 0, sizeof (jack_default_audio_sample_t) * nframes);
	}
	
	return 0;
}

int tX_jack_client::get_sample_rate() 
{
	return jack_get_sample_rate(client);
}

/* tX_audiodevice_jack */

tX_audiodevice_jack::tX_audiodevice_jack():tX_audiodevice()
{
	client=NULL;
}

int tX_audiodevice_jack::open()
{
	tX_jack_client *jack_client=tX_jack_client::get_instance();
	
	if (jack_client) {
		if (jack_client->init()) {
			sample_rate=jack_client->get_sample_rate();
			client=jack_client;
			is_open=true;
			
			return 0;
		}
	}
	
	return 1;
}

int tX_audiodevice_jack::close()
{
	if (client) {
		client->set_device(NULL);
	}
	
	is_open=false;	
	
	return 0;
}

void tX_audiodevice_jack::play(int16_t *buffer)
{
	tX_error("tX_audiodevice_jack::play()");
}

void tX_audiodevice_jack::start()
{
	overrun_buffer=new f_prec[vtt_class::samples_in_mix_buffer];
	
	client->set_device(this);
	while ((!engine->is_stopped()) && !(client->get_jack_shutdown())) {
		usleep(100);
	}	
	client->set_device(NULL);
	
	delete [] overrun_buffer;
}

void tX_audiodevice_jack::fill_frames(jack_default_audio_sample_t *left, jack_default_audio_sample_t *right, jack_nframes_t nframes)
{
	unsigned int outbuffer_pos=0;
	unsigned int sample;
	
	if (samples_in_overrun_buffer) {
		for (sample=0; ((sample<samples_in_overrun_buffer) && (outbuffer_pos<nframes));) {
			left[outbuffer_pos]=overrun_buffer[sample++]/32767.0;
			right[outbuffer_pos++]=overrun_buffer[sample++]/32767.0;
		}
	}
	
	while (outbuffer_pos<nframes) {
		engine->render_cycle();
		
		for (sample=0; ((sample<(unsigned int) vtt_class::samples_in_mix_buffer) && (outbuffer_pos<nframes));) {
			left[outbuffer_pos]=vtt_class::mix_buffer[sample++]/32767.0;
			right[outbuffer_pos++]=vtt_class::mix_buffer[sample++]/32767.0;
		}
		
		if (sample<(unsigned int) vtt_class::samples_in_mix_buffer) {
			samples_in_overrun_buffer=vtt_class::samples_in_mix_buffer-sample;
			/* There's more data in the mixbuffer... */
			memcpy(overrun_buffer, &vtt_class::mix_buffer[sample], sizeof(f_prec) * samples_in_overrun_buffer);
		} else {
			samples_in_overrun_buffer=0;
		}
	}
}

#endif // USE_JACK
