#include <iostream>
#include <vector>
#include "AudioBuffer.hpp"

using namespace std;

#define OUTPUT_WAV_INFO 0
#define wav_info_printf(str) if (OUTPUT_WAV_INFO) printf(str);
#define wav_info_printf_p1(str, p1) if (OUTPUT_WAV_INFO) printf(str, p1);
#define wav_info_printf_p2(str, p1, p2) if (OUTPUT_WAV_INFO) printf(str, p1, p2);

namespace WavUtils
{
	struct WavData
	{
		unsigned char ChunkId[4];
		uint32_t ChunkSize;
		unsigned char Format[4];
		unsigned char SubChunkId1[4];
		uint32_t SubChunkSize1;
		unsigned char SubChunkId2[4];
		uint32_t SubChunkSize2;
		uint16_t AudioFormat;
		uint16_t NumChannels;
		uint32_t SampleRate;
		uint32_t ByteRate;
		uint16_t BlockAlign;
		uint16_t BitsPerSample;

		unsigned char* Data;
	};

	class Int24
	{
	public:
		Int24()
		{ }

		Int24(const int32_t val)
		{
			*this = val;
		}

		Int24(const Int24& val)
		{
			*this = val;
		}

		operator int32_t() const
		{
			if (Bytes[2] & 0x80)
			{
				return (0xff << 24) | (Bytes[2] << 16) | (Bytes[1] << 8) | (Bytes[0] << 0);
			}
			else
			{
				return (Bytes[2] << 16) | (Bytes[1] << 8) | (Bytes[0] << 0);
			}
		}

		operator float() const
		{
			return (float)this->operator int32_t();
		}

		Int24& operator =(const Int24& input)
		{
			Bytes[0] = input.Bytes[0];
			Bytes[1] = input.Bytes[1];
			Bytes[2] = input.Bytes[2];

			return *this;
		}

		Int24& operator =(const int32_t input)
		{
			Bytes[0] = ((unsigned char*)&input)[0];
			Bytes[1] = ((unsigned char*)&input)[1];
			Bytes[2] = ((unsigned char*)&input)[2];

			return *this;
		}

	private:
		unsigned char Bytes[3];
	};

	bool IsBigEndian()
	{
		union EndianTest
        {
			uint32_t i;
			char c[4];
        };
        
        EndianTest Test = { 0x01020304 };

		return Test.c[0] == 1;
	}

    uint32_t GetFmtChunkSize(const WavData& Wav)
    {
        return sizeof(Wav.AudioFormat) + sizeof(Wav.NumChannels) + sizeof(Wav.SampleRate) + 
               sizeof(Wav.ByteRate) + sizeof(Wav.BlockAlign) + sizeof(Wav.BitsPerSample);
    }

    void LoadNextWavChunk(FILE* FileHandle, WavData& Wav)
    {
        unsigned char ChunkId[5];
        size_t len = fread(ChunkId, sizeof(unsigned char), 4, FileHandle);
        ChunkId[len] = '\0';
        wav_info_printf_p1("chunkId: %s\n", ChunkId);

        if (!strcmp((const char*)ChunkId, "fmt "))
		{
			strcpy((char*)Wav.SubChunkId1, (const char*)ChunkId);

			fread(&Wav.SubChunkSize1, sizeof(uint32_t), 1, FileHandle);
			wav_info_printf_p1("subchunk1Size: %d\n", Wav.SubChunkSize1);

			fread(&Wav.AudioFormat, sizeof(uint16_t), 1, FileHandle);
			wav_info_printf_p1("audioFormat: %d\n", Wav.AudioFormat);

			fread(&Wav.NumChannels, sizeof(uint16_t), 1, FileHandle);
			wav_info_printf_p1("numChannels: %d\n", Wav.NumChannels);

			fread(&Wav.SampleRate, sizeof(uint32_t), 1, FileHandle);
			wav_info_printf_p1("sampleRate: %d\n", Wav.SampleRate);

			fread(&Wav.ByteRate, sizeof(uint32_t), 1, FileHandle);
			wav_info_printf_p1("byteRate: %d\n", Wav.ByteRate);

			fread(&Wav.BlockAlign, sizeof(uint16_t), 1, FileHandle);
			wav_info_printf_p1("blockAlign: %d\n", Wav.BlockAlign);

			fread(&Wav.BitsPerSample, sizeof(uint16_t), 1, FileHandle);
			wav_info_printf_p1("bitsPerSample: %d\n", Wav.BitsPerSample);

			uint32_t FmtChunkSize = GetFmtChunkSize(Wav);

			if (Wav.SubChunkSize1 > FmtChunkSize)
			{
				fpos_t FilePos = 0;

				fgetpos(FileHandle, &FilePos);

				FilePos += Wav.SubChunkSize1 - FmtChunkSize;

				fsetpos(FileHandle, &FilePos);

				Wav.SubChunkSize1 = FmtChunkSize;
			}
        }
        else if (!strcmp((const char*)ChunkId, "data"))
        {
            strcpy((char*)Wav.SubChunkId2, (const char*)ChunkId);

            fread(&Wav.SubChunkSize2, sizeof(uint32_t), 1, FileHandle);
            wav_info_printf_p1("subchunk2Size: %d\n", Wav.SubChunkSize2);

            if (Wav.Data)
            {
                free(Wav.Data);
            }

            Wav.Data = (unsigned char*)malloc(sizeof(unsigned char) * Wav.SubChunkSize2);
        
            fread(Wav.Data, sizeof(unsigned char), Wav.SubChunkSize2, FileHandle);
        }
        else
        {
            uint32_t ChunkSize = 0;

            wav_info_printf_p1("skipping chunk: %s\n", ChunkId);

            fread(&ChunkSize, sizeof(uint32_t), 1, FileHandle);
            wav_info_printf_p1("subchunkSize: %d\n", ChunkSize);

            fpos_t FilePos = 0;

            fgetpos(FileHandle, &FilePos);

            FilePos += ChunkSize;

            fsetpos(FileHandle, &FilePos);
        }
    }

    bool LoadWavFile(const char FilePath[], WavData& Wav)
    {
        FILE *FileHandle = fopen(FilePath, "rb");

        if (!FileHandle)
        {
            printf("Error: Failed to open file: %s\n", FilePath);
            return false;
        }
        else
        {
            wav_info_printf_p1("reading file: %s\n", FilePath);
        }

        Wav.Data = nullptr;

        size_t len = fread(Wav.ChunkId, sizeof(unsigned char), 4, FileHandle);
        Wav.ChunkId[len] = '\0';
        
        wav_info_printf_p2("length: %d chunkId: %s\n", (int32_t)len , Wav.ChunkId);
        
        if (strcmp((const char*)Wav.ChunkId, "RIFF"))
        {
            fclose(FileHandle);
            printf("Error: Invalid wav type: %s\n", Wav.ChunkId);
            return false;
        }

        fread(&Wav.ChunkSize, sizeof(uint32_t), 1, FileHandle);
        wav_info_printf_p1("chunkSize: %d\n", Wav.ChunkSize);

        len = fread(Wav.Format, sizeof(unsigned char), 4, FileHandle);
        Wav.Format[len] = '\0';

        wav_info_printf_p1("format: %s\n", Wav.Format);
        
        if (strcmp((const char*)Wav.Format, "WAVE"))
        {
            fclose(FileHandle);
            printf("Error: Invalid wav format: %s\n", Wav.Format);
            return false;
        }

        Wav.SubChunkSize1 = 0;
        Wav.SubChunkSize2 = 0;

        do
        {
            wav_info_printf("loading next chunk...\n");
            LoadNextWavChunk(FileHandle, Wav);
        } while (!feof(FileHandle) && (Wav.SubChunkSize1 == 0 || Wav.SubChunkSize2 == 0));

        fclose(FileHandle);

        return true;
    }

	bool SaveWavFile(const char FilePath[], const WavData& Wav, const vector<AudioBuffer*>& BufferList)
	{
		if (WavUtils::IsBigEndian())
		{
			printf("Error: Wrong endianness detected");
			return false;
		}
        
        FILE* FileHandle = fopen(FilePath, "wb");

		if (!FileHandle)
		{
			printf("Error: Failed to open file: %s\n", FilePath);
			return false;
		}

		fwrite(Wav.ChunkId, sizeof(unsigned char), 4, FileHandle);
		fwrite(&Wav.ChunkSize, sizeof(uint32_t), 1, FileHandle);
		fwrite(Wav.Format, sizeof(unsigned char), 4, FileHandle);
		fwrite(Wav.SubChunkId1, sizeof(unsigned char), 4, FileHandle);
		fwrite(&Wav.SubChunkSize1, sizeof(uint32_t), 1, FileHandle);
		fwrite(&Wav.AudioFormat, sizeof(uint16_t), 1, FileHandle);
		fwrite(&Wav.NumChannels, sizeof(uint16_t), 1, FileHandle);
		fwrite(&Wav.SampleRate, sizeof(uint32_t), 1, FileHandle);
		fwrite(&Wav.ByteRate, sizeof(uint32_t), 1, FileHandle);
		fwrite(&Wav.BlockAlign, sizeof(uint16_t), 1, FileHandle);
		fwrite(&Wav.BitsPerSample, sizeof(uint16_t), 1, FileHandle);

        // Note: assumes compiler is little endian

        if (Wav.Data)
        {
			const uint32_t kBytesPerSrcSample = Wav.BitsPerSample / 8;
			const float kFMax = (float)pow(2, Wav.BitsPerSample - 1);

            fwrite(Wav.SubChunkId2, sizeof(unsigned char), 4, FileHandle);
            fwrite(&Wav.SubChunkSize2, sizeof(uint32_t), 1, FileHandle);

			for (vector<AudioBuffer*>::const_iterator It = BufferList.begin(); It != BufferList.end(); ++It)
			{
				AudioBuffer* Buffer = *It;

				if (Buffer)
				{
                    for (uint32_t FrameIdx = 0; FrameIdx < Buffer->GetNumFrames(); ++FrameIdx)
					{
						for (uint32_t ChannelIdx = 0; ChannelIdx < Buffer->GetNumChannels(); ++ChannelIdx)
						{
                            float* FBuffer = Buffer->GetChannel(ChannelIdx);
                            float FSample = FBuffer[FrameIdx] * kFMax;
                            
							if (Wav.BitsPerSample == 32)
							{
                                int32_t ISampleValue = (int32_t)FSample;

                                fwrite(&ISampleValue, kBytesPerSrcSample, 1, FileHandle);
							}
							else if (Wav.BitsPerSample == 24)
							{
                                Int24 ISampleValue = (Int24)((int32_t)FSample);

                                fwrite(&ISampleValue, kBytesPerSrcSample, 1, FileHandle);
							}
							else if (Wav.BitsPerSample == 16)
							{
                                int16_t ISampleValue = (int16_t)FSample;

                                fwrite(&ISampleValue, kBytesPerSrcSample, 1, FileHandle);
							}
							else if (Wav.BitsPerSample == 8)
							{
                                int8_t ISampleValue = (int32_t)FSample;

                                fwrite(&ISampleValue, kBytesPerSrcSample, 1, FileHandle);
							}
						}
                    }
				}
			}
        }

		fclose(FileHandle);

		return true;
	}

    bool GenerateAudioBuffers(vector<AudioBuffer*>& BufferList, uint32_t FramesPerBuffer, const WavData& Wav)
    {
		if (WavUtils::IsBigEndian())
		{
			printf("Error: Wrong endianness detected");
			return false;
		}

		if (Wav.BitsPerSample != 8 && Wav.BitsPerSample != 16 && Wav.BitsPerSample != 24 && Wav.BitsPerSample != 32)
        {
            printf("Error: Invalid number of bits per sample: %d\n", Wav.BitsPerSample);
            return false;
        }

        const uint32_t kBytesPerSrcSample = Wav.BitsPerSample / 8;
        const uint32_t kBytesPerDstSample = sizeof(float);
		const float kFMax = (float)pow(2, Wav.BitsPerSample - 1);
		unsigned char* BBuffer = (unsigned char*)malloc(kBytesPerSrcSample);
        uint32_t TotalFramesProcessed = 0;
        uint32_t TotalFramesRemaining = Wav.SubChunkSize2 / Wav.BlockAlign;

		// Note: assumes compiler is little endian

        while (TotalFramesRemaining > 0)
        {
            const uint32_t kNumFrames = min(FramesPerBuffer, TotalFramesRemaining);
            const uint32_t kBytesPerDstChannel = kBytesPerDstSample * kNumFrames;
            uint32_t FramesProcessed = 0;
            uint32_t FramesRemaining = kNumFrames;
            float* FBuffer = (float*)malloc(kBytesPerDstSample * kNumFrames * Wav.NumChannels);
            
            while (FramesRemaining > 0)
            {
                for (uint32_t ChannelIdx = 0; ChannelIdx < Wav.NumChannels; ++ChannelIdx)
                {
                    uint32_t SrcOffset = (((TotalFramesProcessed + FramesProcessed) * Wav.NumChannels) + ChannelIdx) * kBytesPerSrcSample;
                    uint32_t DstOffset = (kNumFrames * ChannelIdx) + FramesProcessed;
                    float FSample = 0.0f;

                    memcpy(BBuffer, Wav.Data + SrcOffset, kBytesPerSrcSample);

                    if (Wav.BitsPerSample == 32)
                    {
                        int32_t ISampleValue = 0;

						memcpy(&ISampleValue, BBuffer, kBytesPerSrcSample);

                        FSample = (float)ISampleValue / kFMax;
                    }
                    else if (Wav.BitsPerSample == 24)
                    {
                        Int24 ISampleValue = 0;

                        memcpy(&ISampleValue, BBuffer, kBytesPerSrcSample);
                        
                        FSample = (float)ISampleValue / kFMax;
                    }
                    else if (Wav.BitsPerSample == 16)
                    {
						int16_t ISampleValue = 0;

						memcpy(&ISampleValue, BBuffer, kBytesPerSrcSample);

                        FSample = (float)ISampleValue / kFMax;
                    }
                    else if (Wav.BitsPerSample == 8)
                    {
						int8_t ISampleValue = 0;

						memcpy(&ISampleValue, BBuffer, kBytesPerSrcSample);

                        FSample = (float)ISampleValue / kFMax;
                    }
                    
                    memcpy(FBuffer + DstOffset, &FSample, kBytesPerDstSample);
                }

                ++FramesProcessed;
                --FramesRemaining;
            }        

            AudioBuffer* ABuffer = new AudioBuffer(FBuffer, kNumFrames, Wav.NumChannels, Wav.SampleRate);

            BufferList.push_back(ABuffer);

            TotalFramesProcessed += kNumFrames;
            TotalFramesRemaining -= kNumFrames;
        }

        free(BBuffer);

        return true;
    }

    void Cleanup(WavData* Wav, vector<AudioBuffer*>& BufferList)
    {
        if (Wav)
        {
            if (Wav->Data)
            {
                free(Wav->Data);
            }

            delete Wav;
        }

        for (vector<AudioBuffer*>::iterator It = BufferList.begin(); It != BufferList.end(); ++It)
        {
            AudioBuffer* Buffer = *It;

            if (Buffer)
            {
                delete Buffer;
            }
        }
    }
}
