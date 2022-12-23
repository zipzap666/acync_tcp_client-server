#include <iostream>
#include <fstream>
#include <string>

enum levels
{
    START,
    CONNECTION,
    DISCONNECTION,
    EXEPTION,
    STOP
};

class Logger{
public:
    Logger(std::string path) : file(path, std::ios::app){}
    ~Logger()
    {
        if(file.is_open())
            file.close();
    }

    std::string log(std::string msg, levels level)
    {
        std::string final_msg;
        switch (level)
        {
        case levels::START:
            final_msg = "SERVER STARTED. " + msg;
            break;
        case levels::CONNECTION:
            final_msg = "CONNECTION: " + msg;
            break;
        case levels::DISCONNECTION:
            final_msg = "DISCONNECTION: " + msg;
            break;
        case levels::EXEPTION:
            final_msg = "EXEPTION: " + msg;
            break;
        case levels::STOP:
            final_msg = "SERVER STOPED. " + msg;
            break;
        default:
            final_msg = msg;
            break;
        }
        write(final_msg);
        return final_msg;
    }

private:
    void write(std::string msg)
    {
        if(file.is_open())
            file << msg << std::endl;
    }
    std::ofstream file;
};