#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <memory>
#include "./proto/message.pb.h"

inline char *convert_int32_to_str(uint32_t n)
{
    char *str = new char[sizeof(uint32_t)]();
    uint8_t mask = 255;

    for (int i = 3; i >= 0; --i)
    {
        str[i] = static_cast<char>(n & mask);
        n = n >> 8;
    }
    return str;
}

inline uint32_t convert_str_to_int32(char str[sizeof(uint32_t)])
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

inline TestTask::Messages::WrapperMessage *server_fast_response()
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    *to->mutable_fast_response()->mutable_current_date_time() = 
    std::string(boost::posix_time::to_iso_string(t).c_str(), sizeof("YYYYMMDDThhmmss.fff") - 1);
    return std::move(to);
}

inline TestTask::Messages::WrapperMessage *server_slow_response(size_t count_connectios)
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    to->mutable_slow_response()->set_connected_client_count(count_connectios);
    return std::move(to);
}

inline std::string msg_to_write(TestTask::Messages::WrapperMessage *msg)
{
    std::string str_msg;
    msg->SerializeToString(&str_msg);
    char *size_msg = convert_int32_to_str(str_msg.size());
    str_msg = std::string(size_msg, sizeof(uint32_t)) + str_msg;
    delete[] size_msg;
    return str_msg;
}

#endif // FUNCTIONS_HEADER