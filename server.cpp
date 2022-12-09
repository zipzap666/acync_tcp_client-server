#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <bitset>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "./proto/message.pb.h"

using namespace std;
using boost::asio::ip::tcp;
using namespace TestTask::Messages;
using namespace TestTask;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, boost::asio::deadline_timer timer)
        : socket_(std::move(socket)), timer_(move(timer)) {}

    void start() { do_read_size(); }

private:
    void do_read_size()
    {
        auto self(shared_from_this());
        char data[32];
        socket_.async_read_some(boost::asio::buffer(data, 32),
                                [this, self, &data](boost::system::error_code ec, std::size_t length)
                                {
                                    count_conectios++;
                                    if (!ec)
                                    {
                                        size_t length = 0;
                                        for (int i = 0; i < 32; i++)
                                        {
                                            length <<= 1;
                                            length += data[i] - '0';
                                        }
                                        cout << "Size msg: " << length << endl;
                                        do_read(length);
                                        return;
                                    }
                                });
    }

    void do_read(size_t size)
    {
        auto self(shared_from_this());
        char *data = new char[size + 1]();
        socket_.async_read_some(boost::asio::buffer(data, size),
                                [this, self, data, size](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        cout << data << endl;
                                        data[32] = '\0';
                                        WrapperMessage *from = new WrapperMessage();
                                        from->ParseFromString(data);
                                        delete data;
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
        else
        {
            fast_response(move(from));
        }
    }

    void fast_response(WrapperMessage *from)
    {

        WrapperMessage *to = new WrapperMessage();
        Messages::FastResponse *fast_msg = new Messages::FastResponse();
        time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        *fast_msg->mutable_current_date_time() = string(ctime(&time));
        cout << fast_msg->current_date_time() << endl;
        to->set_allocated_fast_response(move(fast_msg));

        delete from;
        do_write(move(to));
    }

    void slow_response(WrapperMessage *from)
    {
        auto self(shared_from_this());

        WrapperMessage *to = new WrapperMessage();
        Messages::SlowResponse *slow_msg = new Messages::SlowResponse();
        slow_msg->set_connected_client_count();
        to->set_allocated_slow_response(move(slow_msg));

        int seconds = from->request_for_slow_response().time_in_seconds_to_sleep();
        delete from;
        timer_.expires_from_now(boost::posix_time::seconds(from->request_for_slow_response().time_in_seconds_to_sleep()));
        timer_.async_wait([this, self, to](const boost::system::error_code &error)
                          {
            if (!error)
                {
                    do_write(move(to));
                } });
    }

    void do_write(WrapperMessage *to)
    {
        auto self(shared_from_this());
        string request;
        to->SerializeToString(&request);
        delete to;

        string size = bitset<32>(request.size()).to_string();
        request = size + request;

        boost::asio::async_write(socket_, boost::asio::buffer(request.c_str(), request.size()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         count_conectios--;
                                     }
                                     count_conectios--;
                                 });
    }

    inline static uint32_t count_conectios = 0;
    tcp::socket socket_;
    boost::asio::deadline_timer timer_;
    enum
    {
        max_length = 1024
    };
};

class server
{

public:
    server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          timer_(io_context)
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
                    std::make_shared<session>(move(socket), move(timer_))->start();
                }
                do_accept();
            });
    }

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