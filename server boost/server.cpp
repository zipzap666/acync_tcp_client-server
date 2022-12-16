#include "session.h"
class server
{
public:
    server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          timer_(io_context), count_connections_(new size_t(0)),
          log_file_(new std::ofstream("connections.log", std::ios::app)), id_(0)
    {
        *log_file_ << "Start server." << std::endl;
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(
                        std::move(socket),
                        std::move(timer_),
                        count_connections_,
                        id_,
                        log_file_)
                        ->start();
                    id_++;
                }
                do_accept();
            });
    }

    size_t id_;
    std::shared_ptr<std::ofstream> log_file_;
    std::shared_ptr<size_t> count_connections_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::deadline_timer timer_;
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