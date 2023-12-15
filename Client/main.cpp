#include "Client.hpp"


int main()
{
    // TODO: Раскидать по потокам прогу так,
    // чтобы 1 поток обрабатывал сообщения, которые отсылаются, а второй, которые принимаются
    // Think about it;



    std::string field = "1001110011100000000010011001000000000000000000000001000100000100000000000000000000010000001000011110";
    // std::string field = "1100111100100000000010010011001000000000000000000000000100010000000001000000000000000010000111100001";
    // std::string field = "1110001000000000100010010010011000001000100000000000000000110001100000010000000000000100000000010001";
    Client client(field);
    client.makeConnectionWithServer();
    int iter = 0;
    std::string responce;
    do
    {
        responce = client.recieveMessage();
        std::cout << responce << std::endl;
        client.recievedMessagesHandler(responce);
    } while (responce.compare("go:") && responce.compare("wait:"));
    while (responce.compare("win:") && responce.compare("lost:"))
    {
        std::cout << "\nDEBUG\n\n";
        std::cin >> responce;
        client.sendAll(client.getSocket(), responce);
        auto answer = client.recieveMessage();
        std::cout << answer << std::endl;
    }
    responce = client.recieveMessage();
    std::cout << responce << std::endl;
}