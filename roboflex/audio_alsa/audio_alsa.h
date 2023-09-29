#ifndef ROBOFLEX_AUDIO_ALSA__H
#define ROBOFLEX_AUDIO_ALSA__H

#include <Eigen/Dense>
#include <alsa/asoundlib.h>
#include "roboflex/core/node.h"
#include "roboflex/core/serialization/flex_eigen.h"

namespace roboflex {
namespace audio_alsa {

using std::string, std::exception, std::ostream;

constexpr char ModuleName[] = "audio_alsa";

/**
 * The datatype containing audio data. Obviously we have the
 * customary two timestamps. Audio data is contained in the
 * AudioFrame, where columns correspond to channels and
 * rows to capture frames. Templated with 'short' or 'int',
 * for datatype representing one sample; 16 bit or 32 (or 24) bit.
 */
template <typename SampleType>
class AudioDataT: public core::Message{
public:
    typedef Eigen::Matrix<SampleType, Eigen::Dynamic, Eigen::Dynamic> AudioFrame;

    inline static const char MessageName[] = "AudioData";

    AudioDataT(Message& other): core::Message(other) {}
    AudioDataT(int channels, int rows, SampleType * buffer, double t0, double t1);

    double get_t0() const { return root_map()["t0"].AsDouble(); }
    double get_t1() const { return root_map()["t1"].AsDouble(); }

    const AudioFrame get_audio_data() const {
        return serialization::deserialize_eigen_matrix<SampleType, Eigen::Dynamic, Eigen::Dynamic>(root_map()["data"]);
    }

    void print_on(ostream& os) const override;
};

template <typename SampleType>
AudioDataT<SampleType>::AudioDataT(int channels, int rows, SampleType * buffer, double t0, double t1):
    core::Message(ModuleName, MessageName)
{
    AudioFrame data = Eigen::Map<Eigen::Matrix<SampleType, Eigen::Dynamic, Eigen::Dynamic>>(buffer, rows, channels);

    flexbuffers::Builder fbb = get_builder();
    WriteMapRoot(fbb, [&]() {
        fbb.Double("t0", t0);
        fbb.Double("t1", t1);
        serialization::serialize_eigen_matrix(fbb, data, "data");
    });
}

template <typename SampleType>
void AudioDataT<SampleType>::print_on(ostream& os) const
{
    os << "<AudioData(" << sizeof(SampleType)*8 << ")"
       << " data: (" << get_audio_data().rows() << ", " << get_audio_data().cols() << ")"
       << " t0: " << get_t0()
       << " t1: " << get_t1();
    core::Message::print_on(os);
    os << ">";
}

typedef AudioDataT<short> AudioData;
typedef AudioDataT<int> AudioData32;

/**
 * Can be thrown by AudioSensor
 */
struct AudioException: public exception {
    string reason;
    AudioException(const string & reason): exception(), reason(reason) {}
    const char* what() const noexcept { return reason.c_str(); }
};


/**
 * Reads from audio sensors and publishes AudioData.
 * Currently uses ALSA (therefore: linux only).
 * Currently has been tested only with the "Zoom f8n"
 * multichannel mixer.
 */
class AudioSensor: public core::RunnableNode {
public:

    enum class BitDepth {
        //S8 = 0,
        //U8 = 1,
        S16LE = 2,
        //U16LE = 3,
        S24LE = 4,
        //U24LE = 5,
        S32LE = 6,
        //U32LE = 7,
        //F32LE = 8,
        //F64LE = 9,
        S24_3LE = 10,
    };

    AudioSensor(
        const string& name = "AudioSensor",
        unsigned int channels = 8,
        unsigned int sampling_rate = 48000,
        unsigned int capture_frames = 512,
        unsigned int produce_frames = 1024,
        BitDepth bit_depth = BitDepth::S16LE,
        const string& device_name = "default",
        bool debug = false);
    virtual ~AudioSensor();

protected:
    void child_thread_fn() override;
    snd_pcm_t *capture_handle;
    unsigned int channels;
    unsigned int capture_frames;
    unsigned int produce_frames;
    BitDepth bit_depth;
    bool debug;

private:
    void child_thread_fn_S24_3LE();
    void child_thread_fn_others();
};

} // namespace audio_alsa
} // namespace roboflex

#endif // ROBOFLEX_AUDIO_ALSA__H
