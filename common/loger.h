#include <iostream>
#include <fstream>
#include <string>

class Loger{
public:
    Loger(std::string path) : file(path, std::ios::app){}
    ~Loger()
    {
        if(file.is_open())
            file.close();
    }

    void log_start_server()
    {
        if(file.is_open())
            file << "Start server." << std::endl;
    }

    void log_connections(const size_t id)
    {
        if(file.is_open())
            file << "Connected id: " << id << std::endl;
    }

    void log_disconnections(size_t id)
    {
        if(file.is_open())
            file << "Disconnected id: " << id << std::endl;
    }
private:
    std::ofstream file;
};