#ifndef SESSION_HEADER
#define SESSION_HEADER
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "../common/loger.h"
#include "../common/functions.h"
#include "../common/message.pb.h"
#include "../common/stream_parser.h"


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

	void do_read();
	void create_request();
	void fast_response();
	void slow_response(const uint32_t time);
	void do_write(const std::string &str);

	std::shared_ptr<Logger> _log_file;
	const size_t _id;
	std::shared_ptr<size_t> _count_connectios;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::deadline_timer _timer;

	DelimitedMessagesStreamParser<TestTask::Messages::WrapperMessage> parser;
	std::vector<std::shared_ptr<const TestTask::Messages::WrapperMessage>> messages;
};

#endif // SESSION_HEADER