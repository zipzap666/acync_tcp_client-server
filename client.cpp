#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <bitset>
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
    TestTask::Messages::WrapperMessage msg;
    TestTask::Messages::RequestForFastResponse fast_msg;
    *msg.mutable_request_for_fast_response() = fast_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 25555));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    string request;
    msg.SerializeToString(&request);
    string size = bitset<32>(request.size()).to_string();
    request = size + request;
    cout << request << endl;
    write(sock, buffer(request.c_str(), request.size()));

    char length_str[33];
    size_t length = 0;
    read(sock, buffer(length_str, 32));
    length_str[32] = '\0';
    for (int i = 0; i < 32; i++)
    {
        length <<= 1;
        length += length_str[i] - '0';
    }

    cout << "Size msg is: " << length << endl;
    cout << length_str << endl;
    char reply[max_length];
    size_t reply_length = read(sock, buffer(reply, length));
    msg.ParseFromArray(reply, reply_length);
    std::cout << "Reply is: ";
    std::cout << msg.fast_response().current_date_time() << endl;
}

void slowRequest()
{
    TestTask::Messages::WrapperMessage msg;
    TestTask::Messages::RequestForSlowResponse slow_msg;
    slow_msg.set_time_in_seconds_to_sleep(5);
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
    std::cout << "Reply is: " << strlen(reply);
    std::cout << msg.slow_response().connected_client_count() << endl;
}

int main(int argc, char *argv[])
{
    try
    {
        fastRequest();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}