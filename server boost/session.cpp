#include "session.h"

void session::do_read_size()
{
    auto self(shared_from_this());
    char *data = new char[sizeof(uint32_t)]();
    socket_.async_read_some(boost::asio::buffer(data, sizeof(uint32_t)),
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

void session::do_read(size_t size)
{
    auto self(shared_from_this());
    char *data = new char[size]();
    socket_.async_read_some(boost::asio::buffer(data, size),
                            [this, self, data, size](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    TestTask::Messages::WrapperMessage *from = new TestTask::Messages::WrapperMessage();
                                    from->ParseFromString(data);
                                    delete[] data;
                                    check_request_msg(std::move(from));
                                }
                                else
                                {
                                    delete[] data;
                                }
                            });
}

void session::check_request_msg(TestTask::Messages::WrapperMessage *from)
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

void session::fast_response(TestTask::Messages::WrapperMessage *from)
{
    TestTask::Messages::WrapperMessage *to = server_fast_response(from);
    delete from;
    do_write(std::move(to));
}

void session::slow_response(TestTask::Messages::WrapperMessage *from)
{
    auto self(shared_from_this());
    timer_.expires_from_now(boost::posix_time::seconds(from->request_for_slow_response().time_in_seconds_to_sleep()));
    timer_.async_wait([this, self, from](const boost::system::error_code &error)
                      {
            if (!error)
                {
                    TestTask::Messages::WrapperMessage *to = server_slow_response(from, *count_connectios_);
                    delete from;
                    do_write(std::move(to));
                } });
}

void session::do_write(TestTask::Messages::WrapperMessage *to)
{
    auto self(shared_from_this());

    std::string response = msg_to_write(to);
    delete to;

    boost::asio::async_write(socket_, boost::asio::buffer(response.c_str(), response.size()),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {
                                     std::cout << "Package received!" << std::endl;
                                 }
                             });
}