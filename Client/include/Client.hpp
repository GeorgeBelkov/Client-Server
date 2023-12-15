#pragma once

#include <iostream>
#include <string>
#include <regex>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


constexpr static size_t BUFFER_SIZE = 1024;


enum class GameState {
    NULL_ST = 0,
    MAKING_STEP,
    WHAITING
};


enum class ClientExceptions
{
    SOCKET_INIT = 0,
    SOCKET_CONNECTION
};


class Client
{
private:
    int client_socket;
    sockaddr_in address;
    std::string game_field;
    size_t recieved_bytes;
    std::array<char, BUFFER_SIZE> buffer;
    GameState state = GameState::NULL_ST;

public:
    Client() = default;
    explicit Client(std::string& field);
    ~Client() = default;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void createClientAdress();
    void makeConnectionWithServer();

    void sendMessage(std::string& buffer);
    std::string recieveMessage();
    void recievedMessagesHandler(std::string& msg);
    int sendAll(int socket, std::string buffer, int flags = 0);

    int getSocket() const { return client_socket; }

    void gameSession();
};