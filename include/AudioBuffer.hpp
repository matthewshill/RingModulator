#include <stdint.h>
#include <stdlib.h>

using namespace std;

class AudioBuffer
{
public:
	AudioBuffer(float* InBuffer, uint32_t InBufferLen, uint32_t InNumChannels, uint32_t InSampleRate);
    ~AudioBuffer();

	// Get the number of channels.
    uint32_t GetNumChannels() const;

	// Get the buffer of the ith channel. 
	float* GetChannel(uint32_t Index);

    // Number of valid sample frames in the audio buffer
    uint32_t GetNumFrames() const;

    // Number of samples per second
    uint32_t GetSampleRate();

private:
    float* Buffer;
    uint32_t NumFrames;
    uint32_t NumChannels;
    uint32_t SampleRate;
};

AudioBuffer::AudioBuffer(float* InBuffer, uint32_t InNumFrames, uint32_t InNumChannels, uint32_t InSampleRate)
    : Buffer(InBuffer)
    , NumFrames(InNumFrames)
    , NumChannels(InNumChannels)
    , SampleRate(InSampleRate)
{ }

AudioBuffer::~AudioBuffer()
{
    if (Buffer)
    {
        free(Buffer);
    }
}

uint32_t AudioBuffer::GetNumChannels() const
{
    return NumChannels;
}

float* AudioBuffer::GetChannel(uint32_t Index)
{
    if (Index < NumChannels)
    {
        return Buffer + (GetNumFrames() * Index);
    }

    return nullptr;
}

uint32_t AudioBuffer::GetNumFrames() const
{
    return NumFrames;
}

uint32_t AudioBuffer::GetSampleRate()
{
    return SampleRate;
}
