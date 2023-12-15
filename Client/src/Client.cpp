#include "Client.hpp"


constexpr uint16_t PORT = 1234;


void Client::createClientAdress()
{
    // Cистема выберет неиспользуемый номер порта если установить NULL
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}


int Client::sendAll(int socket, std::string buffer, int flags)
{
    auto message_size = buffer.length();
    int bytes = 0, sended_bytes = 0;

    while (sended_bytes < message_size)
    {
        bytes = send(socket, buffer.data() + sended_bytes, (message_size - sended_bytes + 1), flags);
        if (bytes == -1)
            break;
        sended_bytes += bytes; 
    }

    return (bytes == -1 ? -1 : sended_bytes); 
}


Client::Client(std::string& field): game_field(field)
{
    try
    {
        // Создание адреса сокета
        createClientAdress();

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0)
            throw ClientExceptions::SOCKET_INIT;
    }
    catch(ClientExceptions exception)
    {
        switch (exception)
        {
        case ClientExceptions::SOCKET_INIT:
        {
            perror("socket init func error");
            exit(1);
        }
        default:
            break;
        }
    }
    
}


void Client::makeConnectionWithServer()
{
    try
    {
        if (connect(client_socket, (sockaddr*)&address, sizeof(address)) < 0)
            throw ClientExceptions::SOCKET_CONNECTION;
    }
    catch(ClientExceptions exception)
    {
        switch (exception)
        {
        case ClientExceptions::SOCKET_CONNECTION:
        {
            uint16_t port = PORT + 1;
            while (connect(client_socket, (sockaddr*)&address, sizeof(address)) < 0)
            {
                address.sin_port = htons(port);
                port++;
            }
        }
        default:
            break;
        }
    }
    sendAll(client_socket, "connect:", 0);
}


void Client::sendMessage(std::string& buffer)
{
    sendAll(client_socket, buffer, 0);
}


std::string Client::recieveMessage()
{
    recieved_bytes = recv(client_socket, buffer.data(), sizeof(buffer), 0);
    std::string message(buffer.data(), recieved_bytes - 1);
    return message;
}


void Client::recievedMessagesHandler(std::string& msg)
{
    if (!msg.compare("connected:"))
        sendAll(client_socket, "start:", 0);
    else if (!msg.compare("ok:"))
    {
        std::string field = "field:" + game_field;
        std::cout << field << "\n";
        // field_size + 1, потому что нумерация происходит с нуля
        sendAll(client_socket, field, 0);
    }
    // TODO: field_not_valid ...
    else if (!msg.compare("found:"))
        true; // zaglushka !!!!!!!!!!!!!!!!!!
    else if (!msg.compare("go:"))
        state = GameState::MAKING_STEP;
    else if (!msg.compare("wait:"))
        state = GameState::WHAITING;
    else if (std::regex_match(msg, std::regex("cell:[0-9]:[0-9]")))
    {
        // TODO: Логика обработки и изменения поля соперника и самого игрока
        std::cout << msg << "\n";
    }
    else if (!msg.compare("lose:"))
    {
        std::cout << "You LOOSE!!!\n";
    }
    else if (!msg.compare("win:"))
    {
        std::cout << "You WIN!!!\n";
    }
    else
    {
        std::cout << "uncknown command!: " << msg << "\n";
    }   
}


void Client::gameSession()
{
    // TODO: game ...
}