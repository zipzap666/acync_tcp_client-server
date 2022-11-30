#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

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


    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = std::strlen(request);
    write(sock, buffer(request, request_length));

    char reply[max_length];
    size_t reply_length = read(sock, buffer(reply, request_length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}