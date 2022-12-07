#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "./proto/message.pb.h"

using boost::asio::ip::tcp;
using namespace std;
using namespace boost::asio;

enum
{
    max_length = 1024
};

void fastRequest()
{
    cout << 0 << endl;
    TestTask::Messages::WrapperMessage msg;
    TestTask::Messages::RequestForFastResponse fast_msg;
    *msg.mutable_request_for_fast_response() = fast_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 25555));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    char request[max_length];
    msg.SerializeToArray(request, 1024);
    write(sock, buffer(request, strlen(request)));

    char reply[max_length];
    size_t reply_length = read(sock, buffer(reply, strlen(request)));
    msg.ParseFromArray(reply, reply_length);
    std::cout << "Reply is: " << reply;
    std::cout << msg.fast_response().current_date_time() << endl;
}

void slowRequest()
{
    TestTask::Messages::WrapperMessage msg;
    TestTask::Messages::RequestForSlowResponse slow_msg;
    slow_msg.set_time_in_seconds_to_sleep(23);
    *msg.mutable_request_for_slow_response() = slow_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 25555));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    string request;
    msg.SerializeToString(&request);
    write(sock, buffer(request, request.size()));

    char reply[max_length];
    size_t reply_length = read(sock, buffer(reply, request.size()));
    msg.ParseFromString(reply);
    std::cout << "Reply is: ";
    std::cout << msg.slow_response().connected_client_count() << endl;
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ./client.out <port>\n";
            return 1;
        }

        slowRequest();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}