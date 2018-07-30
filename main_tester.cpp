#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <portaudio.h>
#include <pa_asio.h>
#include<AudioInterfaceManager.h>
#include<SimpleBufferedProxy.h>

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

SimpleBufferedProxy<char, 176400> _buffer;

void on_message(websocketpp::client<websocketpp::config::asio_client> *c, websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg)
{
    if(msg->get_opcode() == websocketpp::frame::opcode::value::BINARY )
    {
        _buffer.Push(msg->get_payload().c_str(),msg->get_payload().size());
        return;
    }
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
}

class wsclient
{
  public:
    void init(const std::string uri)
    {
        client.init_asio();
        client.set_message_handler(bind(&on_message, &client, ::_1, ::_2));

        websocketpp::lib::error_code ec;
        con = client.get_connection(uri, ec);
        if (ec)
        {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }
        m_hdl = con->get_handle();
    }
    void connect()
    {
        client.connect(con);
        client.run();
    }

    void sendMsg(const std::string &msg)
    {
        std::cout << "send msg [" << msg << "]\n";
        websocketpp::lib::error_code ec;
        client.send(m_hdl, msg.c_str(), websocketpp::frame::opcode::text, ec);
        if (ec)
        {
            std::cout << "could not send msg because: " << ec.message() << std::endl;
        }
    }

  private: //members
    websocketpp::client<websocketpp::config::asio_client> client;
    websocketpp::client<websocketpp::config::asio_client>::connection_ptr con;
    websocketpp::connection_hdl m_hdl;
};

//*************************************PORTAUDIO*********************************************//

static void PrintSupportedStandardSampleRates(
    const PaStreamParameters *inputParameters,
    const PaStreamParameters *outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int i, printCount;
    PaError err;
        printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++)
    {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported)
        {
            if (printCount == 0)
            {
                printf("\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            }
            else if (printCount == 4)
            {
                printf(",\n\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            }
            else
            {
                printf(", %8.2f", standardSampleRates[i]);
                ++printCount;
            }
        }
    }
    if (!printCount) printf("None\n");
    else {printf("\n");}
}

void listdevices()
{
    int defaultDisplayed;
    const PaDeviceInfo *deviceInfo;
    PaStreamParameters inputParameters, outputParameters;

    printf("PortAudio version: 0x%08X\n", Pa_GetVersion());
    printf("Version text: '%s'\n", Pa_GetVersionInfo()->versionText);

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0)
    {
        printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
        return;
    }

    printf("Number of devices = %d\n", numDevices);
    for (int i = 0; i < numDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("--------------------------------------- device #%d\n", i);

        /* Mark global and API specific default devices */
        defaultDisplayed = 0;
        if (i == Pa_GetDefaultInputDevice())
        {
            printf("[ Default Input");
            defaultDisplayed = 1;
        }
        else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice)
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
            printf("[ Default %s Input", hostInfo->name);
            defaultDisplayed = 1;
        }

        if (i == Pa_GetDefaultOutputDevice())
        {
            printf((defaultDisplayed ? "," : "["));
            printf(" Default Output");
            defaultDisplayed = 1;
        }
        else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice)
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
            printf((defaultDisplayed ? "," : "["));
            printf(" Default %s Output", hostInfo->name);
            defaultDisplayed = 1;
        }

        if (defaultDisplayed)
            printf(" ]\n");

        /* print device info fields */
        { /* Use wide char on windows, so we can show UTF-8 encoded device names */
            wchar_t wideName[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, deviceInfo->name, -1, wideName, MAX_PATH - 1);
            wprintf(L"Name                        = %s\n", wideName);
        }

        printf("Host API                    = %s\n", Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
        printf("Max inputs = %d", deviceInfo->maxInputChannels);
        printf(", Max outputs = %d\n", deviceInfo->maxOutputChannels);

        printf("Default low input latency   = %8.4f\n", deviceInfo->defaultLowInputLatency);
        printf("Default low output latency  = %8.4f\n", deviceInfo->defaultLowOutputLatency);
        printf("Default high input latency  = %8.4f\n", deviceInfo->defaultHighInputLatency);
        printf("Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);

        /* ASIO specific latency information */
        if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paASIO)
        {
            long minLatency, maxLatency, preferredLatency, granularity;

            PaAsio_GetAvailableLatencyValues(i,
                                             &minLatency, &maxLatency, &preferredLatency, &granularity);

            printf("ASIO minimum buffer size    = %ld\n", minLatency);
            printf("ASIO maximum buffer size    = %ld\n", maxLatency);
            printf("ASIO preferred buffer size  = %ld\n", preferredLatency);

            if (granularity == -1)
                printf("ASIO buffer granularity     = power of 2\n");
            else
                printf("ASIO buffer granularity     = %ld\n", granularity);
        }

        printf("Default sample rate         = %8.2f\n", deviceInfo->defaultSampleRate);

        /* poll for standard sample rates */
        inputParameters.device = i;
        inputParameters.channelCount = deviceInfo->maxInputChannels;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParameters.hostApiSpecificStreamInfo = NULL;

        outputParameters.device = i;
        outputParameters.channelCount = deviceInfo->maxOutputChannels;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParameters.hostApiSpecificStreamInfo = NULL;

        if (inputParameters.channelCount > 0)
        {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel input = \n",
                   inputParameters.channelCount);
            PrintSupportedStandardSampleRates(&inputParameters, NULL);
        }

        if (outputParameters.channelCount > 0)
        {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel output = \n",
                   outputParameters.channelCount);
            PrintSupportedStandardSampleRates(NULL, &outputParameters);
        }

        if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0)
        {
            printf("Supported standard sample rates\n for full-duplex 16 bit %d channel input, %d channel output = \n",
                   inputParameters.channelCount, outputParameters.channelCount);
            PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
        }
    }

    Pa_Terminate();

    printf("----------------------------------------------\n");
}

int Int16ProxyInputSource(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{

    auto *out = (char *)outputBuffer;

    (void)timeInfo; /* Prevent unused variable warnings. */
    (void)statusFlags;
    (void)inputBuffer;

    unsigned long avail = _buffer.ReadAvailable();
    int numBytes = framesPerBuffer*2*sizeof(int16_t);//2 means stereo
    auto toFill = numBytes - avail;
    if(toFill <= 0)
    {
        _buffer.Pop(out,numBytes);
    }
    else
    {
        _buffer.Pop(out,avail);
        out += avail;
        memset(out,0,toFill);//should be more clever like loop the avail
        //std::cout << "ERRRRRRRRRRRRR - Not enough samples\n"; //should be log
    }
    return paContinue;
}

//*************************************PORTAUDIO*********************************************//

int main()
{
    std::cout << "Initializing portaudio\n";
    
    AudioInterfaceManager ai;
    ai.InitInterface();
    //listdevices();

    ai.ListDevices();
    std::cout << "Which device do you want to query [index num] -\n";
    std::string devStrNum;
    getline(std::cin, devStrNum);
    int deviceNum = std::stoi(devStrNum);
    ai.ListDeviceInformation(deviceNum);

    AudioInterfaceManager::StreamConfData scd;
    ai.SetStreamConfiguration(deviceNum, 2, AudioInterfaceManager::eInt16, 44100, Int16ProxyInputSource, 1024, scd, nullptr);
    ai.SetStreamFromDevice(scd);
    ai.StartStream(scd);

    
    std::cout << "enter 1 for websocket, 2 for UDP\n";
    std::string cm;
    getline(std::cin, cm);
    int icm = std::stoi(cm);
    wsclient cl;
    std::string conurl;

    switch (icm)
    {
    case 1:
        std::cout << "Websocket url- \n";
        getline(std::cin, conurl);
        cl.init(conurl);
        {
            websocketpp::lib::thread asio_thread(&wsclient::connect, &cl);
            bool contin_u = true;
            while (contin_u)
            {
                std::cout << "enter msg\n";
                std::string msg;
                getline(std::cin, msg);
                if (msg == "break")
                {
                    return 0;
                }
                cl.sendMsg(msg);
            }
            asio_thread.join();
        }

        break;
    case 2:
        std::cout << "UDP not supported yet\n";
        break;
    default:
        break;
    }
}