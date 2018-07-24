#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include<portaudio.h>
#include <pa_asio.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;



void on_message(websocketpp::client<websocketpp::config::asio_client> *c, websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg)
{
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

int main()
{
    PaError err;

    std::cout << "Initializing portaudio\n";
    err = Pa_Initialize();
    if( err != paNoError )
    {
        printf( "ERROR: Pa_Initialize returned 0x%x\n", err );
    }


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