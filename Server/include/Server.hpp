#pragma once

#include <algorithm>
#include <set>
#include <list>
#include <regex>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "Player.hpp"
#include "Logger.hpp"


constexpr size_t null = 0;
constexpr int NUMBER_OF_CLIENTS = 100;
constexpr size_t BUFFER_SIZE = 1024;


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
    std::list<Player> initiators;
    std::list<Player> secondaries;
    
public:
    Server();
    ~Server() = default;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void createAddress();
    void acceptNewClient();
    void runServer();
    
    void makeGameSession(Player& first, Player& second);
    void closeGameSession(Player& first, Player& second);
    void EventHandler(const char* buffer, size_t bytes_read, int client);
    
    bool isFieldValid(char* field);
    int sendAll(int socket, std::string buffer, int flags = 0);
};