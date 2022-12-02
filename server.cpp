#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime> 
#include <boost/asio.hpp>
#include "./proto/message.pb.h"

using namespace std;
using boost::asio::ip::tcp;
using namespace TestTask::Messages;
using namespace TestTask;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)) {}

    void start() { do_read(); }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length)
                                {
                                    count_conectios++;
                                    if (!ec)
                                    {
                                        WrapperMessage* from = new WrapperMessage();
                                        from->ParseFromString(data_);
                                        check_msg(move(from));
                                        cout << from->request_for_slow_response().time_in_seconds_to_sleep() << endl;
                                    }
                                });
    }

    void check_msg(WrapperMessage* from)
    {
        
        if(from->request_for_slow_response().IsInitialized())
        {
            cout << 2 << endl;
            slow_response(move(from));
        }
        else if(from->request_for_fast_response().IsInitialized())
        {
            cout << 1 << endl;
            fast_response(move(from));
        }
        else{
            do_write(move(from));
        }
    }

    void fast_response(WrapperMessage* from)
    {
        WrapperMessage* to = new WrapperMessage();
        Messages::FastResponse* fast_msg = new Messages::FastResponse();
        time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        *fast_msg->mutable_current_date_time() = string(ctime(&time));
        cout << fast_msg->current_date_time() << endl;
        to->set_allocated_fast_response(move(fast_msg));
        
        delete from;
        do_write(move(to));
    }

    void slow_response(WrapperMessage* from)
    {
        auto wait = async(sleep, from->request_for_slow_response().time_in_seconds_to_sleep());
        wait.get();
        
        WrapperMessage* to = new WrapperMessage();
        Messages::SlowResponse* slow_msg = new Messages::SlowResponse();
        slow_msg->set_connected_client_count(count_conectios);
        cout << slow_msg->connected_client_count() << endl;
        to->set_allocated_slow_response(move(slow_msg));
        
        delete from;
        do_write(move(to));        
    }

    void do_write(WrapperMessage* to)
    {
        auto self(shared_from_this());
        char request[max_length];
        to->SerializeToArray(request, 1024);
        delete to;
        cout << request << endl;
        boost::asio::async_write(socket_, boost::asio::buffer(request, strlen(request)),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         do_read();
                                         count_conectios--;
                                     }
                                     count_conectios--;
                                 });
    }


    inline static uint32_t count_conectios = 0;
    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

class server
{

public:
    server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(socket))->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char *argv[])
{
    try
    {
        ifstream file;
        file.open("server_port");
        if (!file.is_open())
        {
            return -1;
        }
        int port;
        file >> port;
        file.close();

        boost::asio::io_context io_context;

        server s(io_context, port);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}