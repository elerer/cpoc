#ifndef __AUDIOINTERFACEMANAGER___
#define __AUDIOINTERFACEMANAGER___

#include <portaudio.h>
#include <vector>

class AudioInterfaceManager
{
  public:
    struct StreamConfData
    {
        short _numChannels = 0;
        int _deviceNum = 0;
        PaSampleFormat _sf = 0;
        PaStream *_stream = nullptr;
        double _srate = 0;
        PaStreamCallback *_cb;
        unsigned long _framesPerBuffer;
        void *_userData = nullptr;
    };
    enum SampleFormat
    {
        eFloat = 0,
        eInt32,
        eInt24,
        eInt16,
        eInt8,
        eUInt8
    };
    void InitInterface();
    void ListDevices();
    void ListDeviceInformation(const int &device);
    void Terminate();
    void SetStreamConfiguration(const int &deviceNum, const int &numChannels, const SampleFormat &sf,
                                const double &sr, PaStreamCallback cb, const unsigned long &framesPerBuffer, StreamConfData &scfOut, void *userData);
    //Set the callback function with the appropriate params and user data, cahce the configuration
    //not starting the stream
    bool SetStreamFromDevice(StreamConfData &scd);
    bool StartStream(StreamConfData &scd);
    bool CloseStream(StreamConfData &scd);
    static void HandlePortAudioError(const PaError &err);

  private:
    int _numDevices = 0;
    std::vector<StreamConfData> _vStreamsData;
};
#endif