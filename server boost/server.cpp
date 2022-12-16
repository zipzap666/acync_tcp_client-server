#include "session.h"
class server
{
public:
    server(boost::asio::io_context &io_context, short port)
        : _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          _timer(io_context), _count_connections(new size_t(0)),
          _log_file(new std::ofstream("connections.log", std::ios::app)), _id(0)
    {
        *_log_file << "Start server." << std::endl;
        do_accept();
    }

private:
    void do_accept()
    {
        _acceptor.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Session>(
                        std::move(socket),
                        std::move(_timer),
                        _count_connections,
                        _id,
                        _log_file)
                        ->start();
                    _id++;
                }
                do_accept();
            });
    }

    
    size_t _id;
    std::shared_ptr<std::ofstream> _log_file;
    std::shared_ptr<size_t> _count_connections;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::deadline_timer _timer;
};

int main(int argc, char *argv[])
{
    try
    {
        std::ifstream file;
        file.open("server_port");
        if (!file.is_open())
        {
            return -1;
        }
        int port;
        file >> port;
        file.close();

        boost::asio::io_context io_context;
        server s(io_context, port);

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}