#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "functions.h"
#include "./proto/message.pb.h"

using namespace std;
using boost::asio::ip::tcp;
using namespace TestTask::Messages;
using namespace TestTask;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket,
            boost::asio::deadline_timer timer,
            shared_ptr<size_t> &count_connections,
            size_t id, shared_ptr<ofstream> &file)
        : socket_(move(socket)),
          timer_(move(timer)),
          count_connectios_(count_connections),
          id_(id), file_(file)
    {
        cout << "Connected " << endl;
        *file_ << "Connected id: " << id_ << endl;
        (*count_connectios_)++;
    }

    void start() { do_read_size(); }

    ~session()
    {
        cout << "Disconnected" << endl;
        *file_ << "Disconnected id: " << id_ << endl;
        (*count_connectios_)--;
    }

private:
    void do_read_size()
    {
        auto self(shared_from_this());
        char *data = new char[4]();
        socket_.async_read_some(boost::asio::buffer(data, 4),
                                [this, self, data](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        size_t length = convert_str_to_int32(data);
                                        delete[] data;
                                        do_read(length);
                                        return;
                                    }
                                });
    }

    void do_read(size_t size)
    {
        auto self(shared_from_this());
        char *data = new char[size]();
        socket_.async_read_some(boost::asio::buffer(data, size),
                                [this, self, data, size](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        WrapperMessage *from = new WrapperMessage();
                                        from->ParseFromString(data);
                                        delete[] data;
                                        check_msg(move(from));
                                        return;
                                    }
                                });
    }

    void check_msg(WrapperMessage *from)
    {
        if (from->has_request_for_slow_response())
        {
            slow_response(move(from));
        }
        else if (from->has_request_for_fast_response())
        {
            fast_response(move(from));
        }
    }

    void fast_response(WrapperMessage *from)
    {
        WrapperMessage *to = server_fast_response(from);
        delete from;
        do_write(move(to));
    }

    void slow_response(WrapperMessage *from)
    {
        auto self(shared_from_this());
        timer_.expires_from_now(boost::posix_time::seconds(from->request_for_slow_response().time_in_seconds_to_sleep()));
        timer_.async_wait([this, self, from](const boost::system::error_code &error)
                          {
            if (!error)
                {
                    WrapperMessage *to = server_slow_response(from, *count_connectios_);
                    delete from;
                    do_write(move(to));
                } });
    }

    void do_write(WrapperMessage *to)
    {
        auto self(shared_from_this());
        string response;
        to->SerializeToString(&response);
        delete to;

        char *size_response = convert_int32_to_str(response.size());
        response = string(size_response, 4) + response;

        delete[] size_response;

        boost::asio::async_write(socket_, boost::asio::buffer(response.c_str(), response.size()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         cout << "Package received!" << endl;
                                     }
                                 });
    }

    shared_ptr<ofstream> file_;
    size_t id_;
    shared_ptr<size_t> count_connectios_;
    tcp::socket socket_;
    boost::asio::deadline_timer timer_;
};

class server
{

public:
    server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          timer_(io_context), count_connections_(new size_t(0)),
          log_file_(new ofstream("connections.log", ios::app)), id_(0)
    {
        *log_file_ << "Start server." << endl;
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
                    std::make_shared<session>(
                        move(socket),
                        move(timer_),
                        count_connections_,
                        id_,
                        log_file_)->start();
                    id_++;
                }
                do_accept();
            });
    }

    size_t id_;
    shared_ptr<ofstream> log_file_;
    shared_ptr<size_t> count_connections_;
    tcp::acceptor acceptor_;
    boost::asio::deadline_timer timer_;
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