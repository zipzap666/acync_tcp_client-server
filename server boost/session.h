#ifndef SESSION_HEADER
#define SESSION_HEADER
#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "../common/loger.h"
#include "../common/functions.h"
#include "../common/message.pb.h"


class Session
    : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket,
            boost::asio::deadline_timer timer,
            std::shared_ptr<size_t> &count_connections,
            const size_t id, std::shared_ptr<Logger> &log_file);

    void start();

    ~Session();

private:
    void do_read_size();

    void do_read(size_t size);

    void check_request_msg(TestTask::Messages::WrapperMessage *from);

    void fast_response(TestTask::Messages::WrapperMessage *from);

    void slow_response(TestTask::Messages::WrapperMessage *from);

    void do_write(TestTask::Messages::WrapperMessage *to);

    std::shared_ptr<Logger> _log_file;
    const size_t _id;
    std::shared_ptr<size_t> _count_connectios;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _timer;
};

#endif // SESSION_HEADER