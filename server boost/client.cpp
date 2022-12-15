#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <bitset>
#include "./proto/message.pb.h"

using boost::asio::ip::tcp;
using namespace std;
using namespace boost::asio;
using namespace TestTask::Messages;

char* convert_int32_to_str(uint32_t n)
{
    char *size_char = new char[4]();
    uint32_t mask = 255;

    for(int i = 3; i >= 0; --i)
    {
        size_char[i] = static_cast<char>(n & mask);
        n = n >> 8;
    }

    cout << uint32_t(size_char[0]) << " " << uint32_t(size_char[1]) << " " << uint32_t(size_char[2]) << " " << uint32_t(u_char(size_char[3])) << endl;

    return size_char;
}

enum
{
    max_length = 1024
};

void fastRequest()
{
    WrapperMessage msg;
    RequestForFastResponse fast_msg;
    *msg.mutable_request_for_fast_response() = fast_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 29999));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    char* test = convert_int32_to_str(157);

    delete []test;

    string request;
    msg.SerializeToString(&request);
    string size = bitset<32>(request.size()).to_string();
    request = size + request;
    cout << request << " : " << request.size() << endl;
    write(sock, buffer(request.c_str(), 4));

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
    WrapperMessage msg;
    RequestForSlowResponse slow_msg;
    slow_msg.set_time_in_seconds_to_sleep(10);
    *msg.mutable_request_for_slow_response() = slow_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 25555));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    string request;
    msg.SerializeToString(&request);
    string size = bitset<32>(request.size()).to_string();
    request = size + request;
    cout << request << " : " << request.size() << endl;
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