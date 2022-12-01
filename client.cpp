#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "./proto/message.pb.h"

using boost::asio::ip::tcp;
using namespace std;
using namespace boost::asio;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <port>\n";
      return 1;
    }

    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), atoi(argv[1])));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    TestTask::Messages::WrapperMessage msg;
    TestTask::Messages::FastResponse fast_msg;

    std::cout << "Enter message: ";
    char request[max_length];
    char test[max_length];
    std::cin.getline(request, max_length);
    *fast_msg.mutable_current_date_time() = request;
    *msg.mutable_fast_response() = fast_msg;
    msg.SerializeToArray(test, 1024);
    write(sock, buffer(test, strlen(test)));

    char reply[max_length];
    size_t reply_length = read(sock, buffer(reply, strlen(test)));
    msg.ParseFromArray(reply, strlen(test));
    std::cout << "Reply is: ";
    std::cout.write(msg.fast_response().current_date_time().c_str(), msg.fast_response().current_date_time().size());
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}