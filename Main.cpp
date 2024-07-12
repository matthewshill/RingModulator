#include "include/WavUtils.hpp"

#define WAV_INPUT_FILE "Input.wav"
#define WAV_OUTPUT_FILE "Output.wav"
#define RING_MOD_FREQ 500.0f
#define MOD_SIGNAL ModulationSignal::Square
#define DRY_WET_MIX 0.8f

// Possible "shapes" for the modulation signal
enum ModulationSignal
{
    Sine,
    Saw,
    Square,
    Triangle
};

/**
 * Applies a ring modulation DSP effect
 * See: https://en.wikipedia.org/wiki/Ring_modulation
 */
class RingModulatorFX
{
public:
    /**
     * Constructor
     * 
     * @param Frequency The frequency of the ring modulation effect (in Hz)
     * @param ModSignal The "shape" of the modulation signal's waveform
     * @param DryWetMix The mix of dry vs. wet output where 0 is all dry and 1 is all wet
     */
    RingModulatorFX(float Frequency, ModulationSignal ModSignal, float DryWetMix)
    {
        RingModFrequency = Frequency;
        RingModSignal = ModSignal;    
        RingModMix = DryWetMix;
    }

    /**
     * Applies a ring modulation DSP effect to an audio buffer
     * 
     * @note The audio buffer should be modified inline/in-place
     * @param Buffer    Contains the audio data as a sequence of floats as well as
     *                  other relevant info... see the AudioBuffer for further details
     */
    void ProcessBuffer(AudioBuffer* Buffer)
    {
        switch (RingModSignal)
        {
        case ModulationSignal::Sine:
            ProcessSine(Buffer);
            break;
        case ModulationSignal::Saw:
            ProcessSaw(Buffer);
            break;
        case ModulationSignal::Square:
            ProcessSquare(Buffer);
            break;
        case ModulationSignal::Triangle:
            ProcessTriangle(Buffer);
            break;
        default:
            break;
        }
    }

    void ProcessSine(AudioBuffer* Buffer)
    {   
        // Buffer is non-interleaved
        for(uint32_t Channel = 0; Channel < Buffer->GetNumChannels(); Channel++) 
        {                    
            float* const ChannelData = Buffer->GetChannel(Channel);
            for(uint32_t Frame = 0; Frame < Buffer->GetNumFrames(); Frame++) 
            {
                const float Time = static_cast<float>(Frame) / Buffer->GetSampleRate();
                const float Sample = ChannelData[Frame];
                if(ChannelData) 
                {
                    float SampleMod  = 0.0f;
                    SampleMod  = Sample * sin(2.0f * M_PI * RingModFrequency * Time);
                    ChannelData[Frame] = (1.0f - RingModMix) * Sample + RingModMix * (SampleMod);
                }
            }
        }
    }

    void ProcessSaw(AudioBuffer* Buffer)
    {
        // Buffer is non-interleaved
        for(uint32_t Channel = 0; Channel < Buffer->GetNumChannels(); Channel++) 
        {                    
            float* const ChannelData = Buffer->GetChannel(Channel);
            for(uint32_t Frame = 0; Frame < Buffer->GetNumFrames(); Frame++) 
            {
                const float Time = static_cast<float>(Frame) / Buffer->GetSampleRate();
                const float Sample = ChannelData[Frame];
                if(ChannelData) 
                {
                    float SampleMod  = 0.0f;
                    SampleMod = Sample * (2.0f * (Time * RingModFrequency - floor(Time * RingModFrequency + 0.5f)));
                    ChannelData[Frame] = ((1.0f - RingModMix) * Sample) + (RingModMix * SampleMod);
                }
            }
        }
    }

    void ProcessSquare(AudioBuffer* Buffer)
    {   
        // Buffer is non-interleaved
        for(uint32_t Channel = 0; Channel < Buffer->GetNumChannels(); Channel++) 
        {                    
            float* const ChannelData = Buffer->GetChannel(Channel);
            for(uint32_t Frame = 0; Frame < Buffer->GetNumFrames(); Frame++) 
            {
                const float Time = static_cast<float>(Frame) / Buffer->GetSampleRate();
                const float Sample = ChannelData[Frame];
                const float Period = 1 / RingModFrequency;
                if(ChannelData) 
                {
                    float SampleMod  = 0.0f;
                    SampleMod = Sample * ((fmod(Time, Period) > Period / 2.0f) ? 1.0f : -1.0f);
                    ChannelData[Frame] = (1.0f - RingModMix) * Sample + RingModMix * (SampleMod);
                }
            }
        }
    }

    void ProcessTriangle(AudioBuffer* Buffer)
    {
        // Buffer is non-interleaved
        for(uint32_t Channel = 0; Channel < Buffer->GetNumChannels(); Channel++) 
        {                    
            float* const ChannelData = Buffer->GetChannel(Channel);
            for(uint32_t Frame = 0; Frame < Buffer->GetNumFrames(); Frame++) 
            {
                const float Time = static_cast<float>(Frame) / Buffer->GetSampleRate();
                const float Sample = ChannelData[Frame];
                if(ChannelData) 
                {
                    float SampleMod  = 0.0f;
                    SampleMod = Sample * (2.0f * fabs(2.0f * (Time * RingModFrequency - floor(Time * RingModFrequency +0.5f))) - 1.0f);
                    ChannelData[Frame] = ((1.0f - RingModMix) * Sample) + (RingModMix * SampleMod);
                }
            }
        }
    }

private:
    float RingModFrequency;
    ModulationSignal RingModSignal;
    float RingModMix;
};

int main()
{   
    #include "include/Exec.inc"
}