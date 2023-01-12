#ifndef STREAM_PARSER_HEADER
#define STREAM_PARSER_HEADER

#include <list>
#include <vector>
#include <string>
#include <memory>
#include "functions.h"


template <typename Message>
std::shared_ptr<Message> parseDelimited(const void *data, size_t size, size_t *bytesConsumed = 0)
{
    if (size < 4)
    {
        return nullptr;
    }
    std::cout << size << std::endl;

    std::shared_ptr<Message> message = nullptr;
    std::string buffer((const char *)(data));
    size_t length = convert_str_to_int32(buffer.substr(0, 4));
    std::cout << length << std::endl;

    if (length > size - 4)
    {
        return nullptr;
    }

    message = std::make_shared<Message>();
    message->ParseFromString(buffer.substr(4, 4 + length));
    *bytesConsumed = 4 + length;

    return message;
}


template <typename Message>
class DelimitedMessagesStreamParser
{
public:
    typedef std::shared_ptr<const Message> PointerToConstValue;
    std::list<PointerToConstValue> parse(const std::string &data)
    {
        for (const char byte : data)
        {
            m_buffer.push_back(byte);
        }

        std::list<typename DelimitedMessagesStreamParser<Message>::PointerToConstValue> messages;
        size_t bytesConsumed = 0;
        std::shared_ptr<Message> message = parseDelimited<Message>(m_buffer.data(), m_buffer.size(), &bytesConsumed);
        while (message != nullptr)
        {
            messages.push_back(message);
            std::vector<char>(m_buffer.begin() + bytesConsumed, m_buffer.end()).swap(m_buffer);
            bytesConsumed = 0;
            message = parseDelimited<Message>(m_buffer.data(), m_buffer.size(), &bytesConsumed);
        }
        return messages;
    }

private:
    std::vector<char> m_buffer;
};

#endif // STREAM_PARSER_H