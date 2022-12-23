#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER
#include <iostream>
#include <memory>
#include <stdio.h>
#include <chrono>
#include "message.pb.h"

inline char *convert_int32_to_str(uint32_t n)
{
    char *str = new char[sizeof(uint32_t)]();
    uint32_t mask = 255;

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
    num += static_cast<uint32_t>(static_cast<u_char>(str[3]));

    return num;
}

    inline auto get_time() {
    using namespace std::chrono;

    time_t ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    
    time_t s = ms / 1000;
    std::tm * tm = std::localtime(&s);
    const size_t size_template = sizeof("YYYYMMDDThhmmss.fff"); 
    char buffer[size_template];
    strftime(buffer, sizeof(size_template), "%Y%m%dT%H%M%S.", tm);
    snprintf(&buffer[size_template - 4], 4, "%03ld", ms % 1000);
    delete tm;    
    return std::string(buffer);
}

inline auto server_fast_response()
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    *to->mutable_fast_response()->mutable_current_date_time() = get_time();
    return std::move(to);
}

inline auto server_slow_response(size_t count_connectios)
{
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    to->mutable_slow_response()->set_connected_client_count(count_connectios);
    return std::move(to);
}

inline auto msg_to_write(TestTask::Messages::WrapperMessage *msg)
{
    std::string str_msg;
    msg->SerializeToString(&str_msg);
    char *size_msg = convert_int32_to_str(str_msg.size());
    str_msg = std::string(size_msg, sizeof(uint32_t)) + str_msg;
    delete[] size_msg;
    return str_msg;
}

inline auto read(const char *data)
{
    auto msg = new TestTask::Messages::WrapperMessage();
    msg->ParseFromString(data);
    return msg;
}

#endif // FUNCTIONS_HEADER