#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <algorithm>
#include <set>
#include <iostream>


constexpr size_t null = 0;

// Cистема выберет неиспользуемый номер порта
size_t PORT = 2345;
constexpr int NUMBER_OF_CLIENTS = 100;


enum class ServerExceptions
{
    SOCKET_INIT = 0,
    SOCKET_BIND,
    SOCKET_LISTEN
};


class Server
{
private:
    int server_sock;
    sockaddr_in server_addres;
    std::set<int> connected_clients;

public:
    Server();
    ~Server() = default;

    void createAddres();
    void acceptNewClient();
    void mainLoop();
};


void Server::createAddres()
{
    size_t DEFAULT_PORT = null;

    server_addres.sin_family = AF_INET;
    server_addres.sin_port = htons(DEFAULT_PORT);
    server_addres.sin_addr.s_addr = htonl(INADDR_ANY);
}


Server::Server()
{
    try
    {
        // Создание адреса слушающего сокета сервера
        createAddres();

        // Получение дескриптора сокета
        server_sock = socket(AF_INET, SOCK_STREAM, null);
        if (server_sock < 0)
            throw ServerExceptions::SOCKET_INIT;
        
        // Присвоение сокету статуса "неблокирующий"
        fcntl(server_sock, F_SETFL, O_NONBLOCK);
        
        // Закрепление адреса за сокетом
        if (bind(server_sock, (sockaddr *)&server_addres, sizeof(server_addres)) < 0)
            throw ServerExceptions::SOCKET_BIND;
        
        // Перевод сокета в состояние ожидания на подключение
        if (listen(server_sock, NUMBER_OF_CLIENTS) < 0)
            throw ServerExceptions::SOCKET_LISTEN;
        
    }
    catch(ServerExceptions exception)
    {
        switch (exception)
        {
        case ServerExceptions::SOCKET_INIT:
        {
            perror("socket init func error");
            exit(1);
        }
        case ServerExceptions::SOCKET_BIND:
        {
            while (bind(server_sock, (sockaddr *)&server_addres, sizeof(server_addres)) < 0)
                server_addres.sin_port = htons(PORT++);
            break;
        }
        case ServerExceptions::SOCKET_LISTEN:
        {
            perror("socket listen func error");
            exit(2);
        }
        default:
            break;
        }
    }
}


void Server::acceptNewClient()
{
    /* TODO логика создания игровой сессии для 2х игроков,
       подключившихся к серверу [void makeGameSession()]  */

    try
    {
        int new_client = accept(server_sock, NULL, NULL);
        if (new_client < 0)
            throw ServerExceptions::SOCKET_INIT;
        fcntl(new_client, F_SETFL, O_NONBLOCK);
        connected_clients.insert(new_client);
    }
    catch(ServerExceptions exception)
    {
        perror("socket accept func error");
        exit(3);
    }
}


void Server::mainLoop()
{
    // Задаём задержку
    timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    while (true)
    {
        // Заполняем множество сокетов
        fd_set readset;
        FD_ZERO(&readset);

        // Слушающий сокет + клиенты
        FD_SET(server_sock, &readset);
        for (auto& client : connected_clients)
            FD_SET(client, &readset);

        // Ждём события в одном из сокетов
        int max_sock_val = std::max(server_sock, *std::max_element(connected_clients.begin(), connected_clients.end()));
        if(select(max_sock_val + 1, &readset, NULL, NULL, &timeout) <= 0)
        {
            perror("select");
            exit(3);
        }

        // Обрабатываем запросы, пришедшие на слушающий сокет
        if(FD_ISSET(server_sock, &readset))
            acceptNewClient();
        
        // Обрабатываем сообщения от клиентов, ЕСЛИ они есть
        for (auto& client : connected_clients)
        {
            if(FD_ISSET(client, &readset))
            {
                // TODO логика обработки сообщений от клиента [void EventHandler()]
            }
        }
        
    }
}
