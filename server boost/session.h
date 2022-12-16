#ifndef SESSION_HEADER
#define SESSION_HEADER
#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "functions.h"
#include "./proto/message.pb.h"


class session
    : public std::enable_shared_from_this<session>
{
public:
    session(boost::asio::ip::tcp::socket socket,
            boost::asio::deadline_timer timer,
            std::shared_ptr<size_t> &count_connections,
            size_t id, std::shared_ptr<std::ofstream> &file);

    void start();

    ~session();

private:
    void do_read_size();

    void do_read(size_t size);

    void check_request_msg(TestTask::Messages::WrapperMessage *from);

    void fast_response(TestTask::Messages::WrapperMessage *from);

    void slow_response(TestTask::Messages::WrapperMessage *from);

    void do_write(TestTask::Messages::WrapperMessage *to);

    std::shared_ptr<std::ofstream> file_;
    const size_t id_;
    std::shared_ptr<size_t> count_connectios_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::deadline_timer timer_;
};

#endif // SESSION_HEADER