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
    std::cout << _log_file->log(std::to_string(_id), levels::CONNECTION)
              << std::endl;

    (*_count_connectios)++;
}

Session::~Session()
{
    std::cout << _log_file->log(std::to_string(_id), levels::DISCONNECTION)
              << std::endl;
    (*_count_connectios)--;
}

void Session::start() { do_read(); }

void Session::do_read()
{
    auto self(shared_from_this());
    char data[1024];
    _socket.async_read_some(boost::asio::buffer(data, 1024),
                            [this, self, data](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {

                                    std:: cout << int(data[0]) << " " << int(data[1]) << " " << int(data[2]) << " " << int(data[3]) << std::endl;
                                    std::cout << length << std::endl;
                                    const auto &parsed_msg = parser.parse(std::string(data, length));
                                    std::cout << parsed_msg.size() << std::endl;
                                    for (const auto &message : parsed_msg)
                                    {
                                        messages.push_back(message);
                                    }
                                    std::cout << messages.size() << std::endl;
                                    create_request();
                                }
                            });
}

void Session::create_request()
{
    for (auto msg : messages)
    {

        if (msg->has_request_for_slow_response())
        {
            slow_response(msg->request_for_slow_response().time_in_seconds_to_sleep());
        }
        else if (msg->has_request_for_fast_response())
        {
            fast_response();
        }
        else
        {
            do_write("Didn`t parsed!!!");
        }
    }
}

void Session::fast_response()
{
    do_write(msg_to_write(server_fast_response()));
}

void Session::slow_response(uint32_t time)
{
    auto self(shared_from_this());
    auto msg = server_slow_response(*_count_connectios);
    _timer.expires_from_now(boost::posix_time::seconds(time));
    _timer.async_wait([this, self, &msg](const boost::system::error_code &error)
                      {
                        if (!error)
                        {
                            do_write(msg_to_write(msg));
                        } });
}

void Session::do_write(const std::string &msg)
{
    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(msg.c_str(), msg.size()),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {
                                     std::cout << "Package received!" << std::endl;
                                 }
                             });
}