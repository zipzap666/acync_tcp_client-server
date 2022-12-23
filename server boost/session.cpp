#include "session.h"


Session::Session(boost::asio::ip::tcp::socket socket,
                 boost::asio::deadline_timer timer,
                 std::shared_ptr<size_t> &count_connections,
                 const size_t id, std::shared_ptr<Logger> &log_file)
    : _socket(std::move(socket)),
      _timer(std::move(timer)),
      _count_connectios(count_connections),
      _id(id), _log_file(log_file)
{
    std::cout << 
        _log_file->log(std::to_string(_id), levels::CONNECTION)
    << std::endl;
    
    (*_count_connectios)++;
}

Session::~Session()
{
    std::cout << 
        _log_file->log(std::to_string(_id), levels::DISCONNECTION)
    << std::endl;
    (*_count_connectios)--;
}

void Session::start() { do_read_size(); }

void Session::do_read_size()
{
    auto self(shared_from_this());
    char *size = new char[sizeof(uint32_t)]();
    _socket.async_read_some(boost::asio::buffer(size, sizeof(uint32_t)),
                            [this, self, size](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    do_read(convert_str_to_int32(size));
                                    return;
                                }
                                delete[] size;
                            });
}

void Session::do_read(size_t size)
{
    auto self(shared_from_this());
    char *data = new char[size]();
    _socket.async_read_some(boost::asio::buffer(data, size),
                            [this, self, data, size](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    check_request_msg(std::move(read(data)));
                                }
                                delete[] data;
                            });
    
}

void Session::check_request_msg(TestTask::Messages::WrapperMessage *from)
{
    if (from->has_request_for_slow_response())
    {
        slow_response(std::move(from));
    }
    else if (from->has_request_for_fast_response())
    {
        fast_response(std::move(from));
    }
}

void Session::fast_response(TestTask::Messages::WrapperMessage *from)
{
    TestTask::Messages::WrapperMessage *to = server_fast_response();
    delete from;
    do_write(std::move(to));
}

void Session::slow_response(TestTask::Messages::WrapperMessage *from)
{
    auto self(shared_from_this());
    TestTask::Messages::WrapperMessage *to = server_slow_response(*_count_connectios);
    _timer.expires_from_now(boost::posix_time::seconds(from->request_for_slow_response().time_in_seconds_to_sleep()));
    _timer.async_wait([this, self, from, to](const boost::system::error_code &error)
                      {
                        if (!error)
                        {
                            delete from;
                            do_write(std::move(to));
                        } });
}

void Session::do_write(TestTask::Messages::WrapperMessage *to)
{
    auto self(shared_from_this());

    std::string response = msg_to_write(to);
    delete to;

    boost::asio::async_write(_socket, boost::asio::buffer(response.c_str(), response.size()),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {
                                     std::cout << "Package received!" << std::endl;
                                 }
                             });
}