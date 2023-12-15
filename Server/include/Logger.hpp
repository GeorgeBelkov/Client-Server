#pragma once

#include <fstream>
#include <filesystem>

static std::filesystem::path filepath = "/home/bigfalby/Desktop/Projects/Client-Server/Server/LOGGER.txt";



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

    void openLogFile() { Logger::getLoggerInstance().logfile.open(filepath, std::ios::app); }
    void closeLogFile() { Logger::getLoggerInstance().logfile.close(); }

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
        Logger::getLoggerInstance().logfile << string;
        // Закрываем файл
        Logger::getLoggerInstance().closeLogFile();

        return Logger::getLoggerInstance();
    }
};