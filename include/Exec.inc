    if (WavUtils::IsBigEndian())
    {
        printf("Error: Wrong endianness detected");
        return -1;
    }

    vector<AudioBuffer*> BufferList;
    WavUtils::WavData* Wav = new WavUtils::WavData();

    printf("Loading input wav file %s...\n", WAV_INPUT_FILE);

    if (!WavUtils::LoadWavFile(WAV_INPUT_FILE, *Wav))
    {
        WavUtils::Cleanup(Wav, BufferList);
        printf("Error: Failed to load input file");
        return -1;
    }

    printf("Generating audio buffers...\n");

    if (!WavUtils::GenerateAudioBuffers(BufferList, 1024, *Wav))
    {
        WavUtils::Cleanup(Wav, BufferList);
        printf("Error: Failed to generate audio buffers\n");
        return -1;
    }

    RingModulatorFX Fx(RING_MOD_FREQ, MOD_SIGNAL, DRY_WET_MIX);

    printf("Applying the ring modulator FX...\n");

    for (vector<AudioBuffer*>::iterator It = BufferList.begin(); It != BufferList.end(); ++It)
    {
        AudioBuffer* Buffer = *It;

        if (Buffer)
        {
            Fx.ProcessBuffer(Buffer);
        }
    }

    printf("Saving output wav file %s...\n", WAV_OUTPUT_FILE);

    if (!WavUtils::SaveWavFile(WAV_OUTPUT_FILE, *Wav, BufferList))
    {
        WavUtils::Cleanup(Wav, BufferList);
        printf("Error: Failed to save output file");
        return -1;
    }

    WavUtils::Cleanup(Wav, BufferList);

    printf("Program completed successfully\n");

    return 0;