#include <AudioInterfaceManager.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <strings.h>
#include<cstring>

void AudioInterfaceManager::InitInterface()
{
    PaError err = Pa_Initialize();
    HandlePortAudioError(err);
    _numDevices = Pa_GetDeviceCount();
    if (_numDevices < 0)
    {
        printf("ERROR: Pa_CountDevices returned 0x%x\n", _numDevices);
    }
}

void AudioInterfaceManager::Terminate()
{
    PaError err = Pa_Terminate();
    HandlePortAudioError(err);
}

void AudioInterfaceManager::ListDevices()
{
    const PaDeviceInfo *di = nullptr;
    for (int i = 0; i < _numDevices; ++i)
    {
        di = Pa_GetDeviceInfo(i);
        if (di)
        {
            std::cout << i << "- device name [" << di->name << "]\n";
        }
    }
}

void AudioInterfaceManager::ListDeviceInformation(const int &device)
{
    const PaDeviceInfo *di = nullptr;
    if (device >= _numDevices)
    {
        std::cout << "err - requested device [" << device << "] exceeds limit [" << _numDevices << "]\n";
    }
    di = Pa_GetDeviceInfo(device);
    if (di)
    {
        std::cout << "device name [" << di->name << "]\n";
        std::cout << "maxInputChannels [" << di->maxInputChannels << "]\n";
        std::cout << "maxOutputChannels [" << di->maxOutputChannels << "]\n";
        std::cout << "defaultSampleRate [" << di->defaultSampleRate << "]\n";
    }
}

void AudioInterfaceManager::HandlePortAudioError(const PaError &err)
{
    if (err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }
}

bool AudioInterfaceManager::SetStreamFromDevice(StreamConfData &scd)
{
    PaStreamParameters inputParameters; //should it be pointer?
    memset(&inputParameters,0, sizeof(inputParameters));
    inputParameters.channelCount = scd._numChannels;
    inputParameters.device = scd._deviceNum;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = scd._sf;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(scd._deviceNum)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    std::cout << "set following stream:\n num channels -" << scd._numChannels << " sample format -"
        << scd._sf <<" latency -" << inputParameters.suggestedLatency << "\n";

    auto err = Pa_OpenStream(
        &scd._stream,
        &inputParameters,
        &inputParameters, //on purpose
        scd._srate,
        scd._framesPerBuffer,
        paNoFlag, //flags that can be used to define dither, clip settings and more
        scd._cb,  //your callback function
        scd._userData); //data to be passed to callback. In C++, it is frequently (void *)this
    _vStreamsData.push_back(scd);
    HandlePortAudioError(err);
    return true;
}

void AudioInterfaceManager::SetStreamConfiguration(const int &deviceNum, const int &numChannels, const SampleFormat &sf,
                                                   const double &sr, PaStreamCallback *cb, const unsigned long &framesPerBuffer, StreamConfData &scfOut,void *userData)
{
    if ((scfOut._deviceNum = deviceNum) > _numDevices)
        throw std::runtime_error(std::string("Bad configruation: deviceNum ") +
                                 std::to_string(deviceNum));

    auto deviceInfo = Pa_GetDeviceInfo(deviceNum);

    if ((scfOut._numChannels = numChannels) <= 0 && numChannels > deviceInfo->maxInputChannels)
        throw std::runtime_error(std::string("Bad configruation: numChannels ") +
                                 std::to_string(numChannels));

    switch (sf)
    {
    case eFloat:
        scfOut._sf = paFloat32;
        break;
    case eInt24:
        scfOut._sf = paInt24;
        break;
    case eInt16:
        scfOut._sf = paInt16;
        break;
    default:
        throw std::runtime_error(std::string("Bad configruation: unsupported sample format ") +
                                 std::to_string((int)sf));
    }

    if (sr == 0)
    {
        scfOut._srate = deviceInfo->defaultSampleRate;
        std::cout << "setting sample rate to device defaults [" << scfOut._srate << "]\n";
    }
    else
        scfOut._srate = sr;

    scfOut._cb = cb;

    scfOut._framesPerBuffer = framesPerBuffer;

    scfOut._userData = userData;
}

bool AudioInterfaceManager::StartStream(StreamConfData &scd)
{
    auto err = Pa_StartStream( scd._stream );
    HandlePortAudioError(err);
    return err == paNoError;
}

bool AudioInterfaceManager::CloseStream(StreamConfData &scd)
{
    auto err = Pa_CloseStream( scd._stream );
    HandlePortAudioError(err);
    return err == paNoError;
}