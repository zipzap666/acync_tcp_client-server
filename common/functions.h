#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER
#include <iostream>
#include <memory>
#include <stdio.h>
#include <chrono>
#include "message.pb.h"

inline std::string convert_int32_to_str(uint32_t n)
{
    char str[sizeof(uint32_t)];
    uint32_t mask = 255;

    for (int i = 3; i >= 0; --i)
    {
        str[i] = static_cast<char>(n & mask);
        n = n >> 8;
    }
    return std::string(str,4);
}

inline uint32_t convert_str_to_int32(const char *str)
{
    uint32_t num = 0;
    for (int i = 0; i < 3; ++i)
    {
        num += u_char(str[i]);
        num = num << 8;
        std:: cout << int(str[i]) << " ";
    }
    num += static_cast<uint32_t>(static_cast<u_char>(str[3]));
    std:: cout << int(str[3]) << std::endl;

    return num;
}

inline uint32_t convert_str_to_int32(std::string str)
{
    return convert_str_to_int32(str.c_str());
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
    TestTask::Messages::WrapperMessage msg;
    msg.mutable_fast_response()->set_current_date_time(get_time());
    return msg;
}

inline auto server_slow_response(size_t count_connectios)
{
    TestTask::Messages::WrapperMessage msg;
    msg.mutable_slow_response()->set_connected_client_count(count_connectios);
    return msg;
}

inline auto msg_to_write(const TestTask::Messages::WrapperMessage &msg)
{
    std::string str_msg;
    msg.SerializeToString(&str_msg);
    str_msg = convert_int32_to_str(str_msg.size()) + str_msg;
    return str_msg;
}

inline auto read(const char *data)
{
    auto msg = new TestTask::Messages::WrapperMessage();
    msg->ParseFromString(data);
    return msg;
}

#endif // FUNCTIONS_HEADER