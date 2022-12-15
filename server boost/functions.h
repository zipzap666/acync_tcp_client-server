#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <memory>
#include "./proto/message.pb.h"

char *convert_int32_to_str(uint32_t n)
{
    char *str = new char[4]();
    uint32_t mask = 255;

    for (int i = 3; i >= 0; --i)
    {
        str[i] = static_cast<char>(n & mask);
        n = n >> 8;
    }
    return str;
}

uint32_t convert_str_to_int32(char *str)
{
    uint32_t num = 0;

    for (int i = 0; i < 3; ++i)
    {
        num += uint32_t(u_char(str[i]));
        num = num << 8;
    }

    num += uint32_t(u_char(str[3]));
    return num;
}

TestTask::Messages::WrapperMessage *server_fast_response(TestTask::Messages::WrapperMessage *from)
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    TestTask::Messages::FastResponse *fast_msg = new TestTask::Messages::FastResponse();
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    *fast_msg->mutable_current_date_time() = boost::posix_time::to_iso_string(t);
    to->set_allocated_fast_response(std::move(fast_msg));
    return std::move(to);
}

TestTask::Messages::WrapperMessage *server_slow_response(TestTask::Messages::WrapperMessage *from, size_t count_connectios)
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    TestTask::Messages::SlowResponse *slow_msg = new TestTask::Messages::SlowResponse();
    slow_msg->set_connected_client_count(count_connectios);
    to->set_allocated_slow_response(std::move(slow_msg));

    return std::move(to);
}

#endif // FUNCTIONS_HEADER