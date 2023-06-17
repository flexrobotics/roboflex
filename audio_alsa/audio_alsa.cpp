#include <iostream>
#include "audio_alsa/audio_alsa.h"
#include "core/serialization/flex_eigen.h"
#include "core/util/utils.h"

namespace roboflex {
namespace audio_alsa {


snd_pcm_format_t alsa_bit_depth_from_our_bit_depth(AudioSensor::BitDepth bd) {
    switch (bd) {
        //case AudioSensor::BitDepth::S8: return SND_PCM_FORMAT_S8;
        //case AudioSensor::BitDepth::U8: return SND_PCM_FORMAT_U8;
        case AudioSensor::BitDepth::S16LE: return SND_PCM_FORMAT_S16_LE;
        //case AudioSensor::BitDepth::U16LE: return SND_PCM_FORMAT_U16_LE;
        case AudioSensor::BitDepth::S24LE: return SND_PCM_FORMAT_S24_LE;
        //case AudioSensor::BitDepth::U24LE: return SND_PCM_FORMAT_U24_LE;
        case AudioSensor::BitDepth::S32LE: return SND_PCM_FORMAT_S32_LE;
        //case AudioSensor::BitDepth::U32LE: return SND_PCM_FORMAT_U32_LE;
        //case AudioSensor::BitDepth::F32LE: return SND_PCM_FORMAT_FLOAT_LE;
        //case AudioSensor::BitDepth::F64LE: return SND_PCM_FORMAT_FLOAT64_LE;
        case AudioSensor::BitDepth::S24_3LE: return SND_PCM_FORMAT_S24_3LE;
        default: throw std::runtime_error("Unsupported bit depth!"); break;
    }
}

bool is_16_bit(AudioSensor::BitDepth bd) {
    return bd == AudioSensor::BitDepth::S16LE;
}

bool is_24_bit(AudioSensor::BitDepth bd) {
    return bd == AudioSensor::BitDepth::S24LE;
}

constexpr int DeviceBufferSize = 65536 * 8;

AudioSensor::AudioSensor(
    const string &name,
    unsigned int channels,
    unsigned int sampling_rate,
    unsigned int capture_frames,
    unsigned int produce_frames,
    AudioSensor::BitDepth bit_depth,
    const string &device_name,
    bool debug):
        core::RunnableNode(name),
        channels(channels),
        capture_frames(capture_frames),
        produce_frames(produce_frames),
        bit_depth(bit_depth),
        debug(debug)
{
    int err = 0;
    snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&capture_handle, device_name.c_str(), stream, 0)) < 0) {
        throw AudioException(string("cannot open audio device ") + device_name + ":" + snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        throw AudioException(string("cannot allocate hardware parameter structure:") + snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        throw AudioException(string("cannot initialize hardware parameter structure:") + snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        throw AudioException(string("cannot set access type:") + snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, alsa_bit_depth_from_our_bit_depth(bit_depth))) < 0) {
        throw AudioException(string("cannot set sample format:") + snd_strerror(err));
    }

    unsigned int crate = sampling_rate;
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &crate, 0)) < 0 || crate != sampling_rate) {
        throw AudioException(string("cannot set sample rate:") + snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, channels)) < 0) {
        throw AudioException(string("cannot set channel count:") + snd_strerror(err));
    }

    int buffersize = capture_frames * channels * 2;

    if (buffersize > DeviceBufferSize) {
        throw AudioException("buffer size must be <= " + std::to_string(DeviceBufferSize));
    }

    if ((err = snd_pcm_hw_params_set_buffer_size(capture_handle, hw_params, buffersize)) < 0) {
        throw AudioException(string("cannot set buffer size:") + snd_strerror(err));
    }

    // leave this here: this is how we get the buffer size (so far, we don't have to)
    // unsigned long int actual_buffer_size = 0;
    // kv = snd_pcm_hw_params_get_buffer_size(hw_params, &actual_buffer_size);

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        throw AudioException(string("cannot set parameters:") + snd_strerror(err));
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        throw AudioException(string("cannot prepare audio interface for use:") + snd_strerror(err));
    }
}

AudioSensor::~AudioSensor()
{
    snd_pcm_close (capture_handle);
    capture_handle = NULL;
	snd_config_update_free_global();
}

void AudioSensor::child_thread_fn_S24_3LE()
{
    // In "S24_3LE" mode, we read 24-bit mode in 'packed format'. In the
    // other modes, 24-bit is read in S24_LE, which means 4-byte format
    // (in other words, the high-order byte is 0). So in this mode,
    // we need to unpack.
    uint8_t total_buf_packed[DeviceBufferSize];
    int total_buf_unpacked[DeviceBufferSize];

    unsigned int frames_captured = 0;

    double t0 = core::get_current_time();

    while (!this->stop_signal) {
        int offset = channels * frames_captured * 3;

        int read_frames = snd_pcm_readi(capture_handle, total_buf_packed+offset, capture_frames);

        frames_captured += read_frames;

        if (debug) {
            std::cout << "snd_pcm_readi read: " << read_frames << "frames, total: " << frames_captured << " offset:" << offset << std::endl;
        }

        if (read_frames < 0) {
            throw AudioException(string("snd_pcm_readi returned non-positive (error!): ") + std::to_string(read_frames));
        }

        if (frames_captured == produce_frames) {

            // We've accumulated enough data: signal observers
            double t1 = core::get_current_time();

            int nz = channels * frames_captured;
            for (int z=0; z<=nz; z++) {
                total_buf_unpacked[z] = (int)(*(total_buf_packed+(z*3)));
            }

            this->signal(std::make_shared<AudioData32>(channels, produce_frames, total_buf_unpacked, t0, t1));

            // reset
            frames_captured = 0;
            memset(total_buf_unpacked, 0, DeviceBufferSize*sizeof(int));
            t0 = core::get_current_time();
        }
    }
}

void AudioSensor::child_thread_fn_others()
{
    short total_buf_short[DeviceBufferSize]; // just make a big buffer
    int total_buf_int[DeviceBufferSize];

    unsigned int frames_captured = 0;

    double t0 = core::get_current_time();

    while (!this->stop_signal) {
        int offset = channels * frames_captured;

        int read_frames = 0; //snd_pcm_readi(capture_handle, total_buf + offset, capture_frames);
        if (is_16_bit(bit_depth)) {
            read_frames = snd_pcm_readi(capture_handle, total_buf_short + offset, capture_frames);
        } else {
            read_frames = snd_pcm_readi(capture_handle, total_buf_int + offset, capture_frames);
        }

        frames_captured += read_frames;

        if (debug) {
            std::cout << "snd_pcm_readi read: " << read_frames << "frames, total: " << frames_captured << " offset:" << offset << std::endl;
        }

        if (read_frames < 0) {
            throw AudioException(string("snd_pcm_readi returned non-positive (error!): ") + std::to_string(read_frames));
        }

        if (frames_captured == produce_frames) {

            // We've accumulated enough data: signal observers
            double t1 = core::get_current_time();

            if (is_16_bit(bit_depth)) {
                this->signal(std::make_shared<AudioData>(channels, produce_frames, total_buf_short, t0, t1));
            } else {
                this->signal(std::make_shared<AudioData32>(channels, produce_frames, total_buf_int, t0, t1));
            }

            // reset
            frames_captured = 0;
            if (is_24_bit(bit_depth)) {
                memset(total_buf_int, 0, DeviceBufferSize*sizeof(int));
            }
            t0 = core::get_current_time();
        }
    }
}

void AudioSensor::child_thread_fn()
{
     if (bit_depth == AudioSensor::BitDepth::S24_3LE) {
         child_thread_fn_S24_3LE();
     } else {
         child_thread_fn_others();
     }
}


} // namespace audio_alsa
} // namespace roboflex
