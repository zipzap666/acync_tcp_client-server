#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include "../common/functions.h"
#include "../common/message.pb.h"

using boost::asio::ip::tcp;
using namespace std;
using namespace boost::asio;
using namespace TestTask::Messages;

void fastRequest()
{
    WrapperMessage msg;
    RequestForFastResponse fast_msg;
    *msg.mutable_request_for_fast_response() = fast_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 29999));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    string request = msg_to_write(&msg);
    write(sock, buffer(request.c_str(), request.size()));

    char length_str[4];
    size_t length = 0;
    read(sock, buffer(length_str, 4));
    length = convert_str_to_int32(length_str);
    cout << "Size msg is: " << length << endl;
    char *reply = new char[length]();
    read(sock, buffer(reply, length));
    msg.ParseFromArray(reply, length);
    delete[] reply;
    std::cout << "Reply is: ";
    std::cout << msg.fast_response().current_date_time() << endl;
}

void slowRequest()
{
    WrapperMessage msg;
    RequestForSlowResponse slow_msg;
    slow_msg.set_time_in_seconds_to_sleep(10);
    *msg.mutable_request_for_slow_response() = slow_msg;
    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 29999));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    string request = msg_to_write(&msg);
    write(sock, buffer(request.c_str(), request.size()));

    char length_str[4];
    size_t length = 0;
    read(sock, buffer(length_str, 4));
    length = convert_str_to_int32(length_str);
    cout << "Size msg is: " << length << endl;
    char *reply = new char[length]();
    read(sock, buffer(reply, length));
    msg.ParseFromArray(reply, length);
    delete[] reply;
    std::cout << "Reply is: ";
    std::cout << msg.slow_response().connected_client_count() << endl;
}

int main(int argc, char *argv[])
{
    try
    {
        slowRequest();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}