#pragma once

#include <fstream>

/*
    Реализованный на базе паттерна проектирования
    Singleton - класс Logger предназначен для 
    logg-ирования и предоставления более детальной
    информации о процессах, происходящих на сервере. 
*/
class Logger
{
private:
    std::ofstream logfile;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger() = default;
    ~Logger() = default;

    void openLogFile() { logfile.open("Server/LOGGER.txt"); }
    void closeLogFile() { logfile.close(); }

public:
    static Logger& getLoggerInstance()
    {
        static Logger logger;
        return logger;
    }

    Logger& operator<<(std::string&& string)
    {
        // Открываем файл
        Logger::getLoggerInstance().openLogFile();
        // Пишем в файл
        Logger::getLoggerInstance().logfile << string << std::endl;
        // Закрываем файл
        Logger::getLoggerInstance().closeLogFile();

        return Logger::getLoggerInstance();
    }
};