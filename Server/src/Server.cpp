#include "../include/Server.hpp"


void Server::createAddress()
{
    // Cистема выберет неиспользуемый номер порта
    size_t DEFAULT_PORT = null;

    server_addres.sin_family = AF_INET;
    server_addres.sin_port = htons(DEFAULT_PORT);
    server_addres.sin_addr.s_addr = htonl(INADDR_ANY);
}


int Server::sendAll(int socket, std::string buffer, int flags)
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


Server::Server()
{
    try
    {
        initiators.clear();
        secondaries.clear();
        connected_clients.clear();
        // Создание адреса слушающего сокета сервера
        createAddress();

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
            size_t PORT = 2345;
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


void Server::runServer()
{
    // Задаём задержку
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    std::array<char, BUFFER_SIZE> buffer;
    size_t bytes_read;

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
        if (select(max_sock_val + 1, &readset, NULL, NULL, &timeout) < 0)
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
                bytes_read = recv(client, buffer.data(), BUFFER_SIZE, null);
                EventHandler(buffer.data(), bytes_read, client);
            }
        }
        
    }
}


void Server::makeGameSession(Player& first, Player& second)
{
    first.setEnemy(second);
    second.setEnemy(first);

    sendAll(first.getSock(), "found:", 0);
    sendAll(second.getSock(), "found:", 0);
    sendAll(first.getSock(), "go:", 0);
    sendAll(second.getSock(), "wait:", 0);

    // Set states for players
    first.setPLayersState(PlayersState::MAKING_STEP);
    second.setPLayersState(PlayersState::WHAITING);
}


void Server::closeGameSession(Player& first, Player& second)
{
    // Удаляем инициатора игры из списка
    initiators.remove(first);
    secondaries.remove(second);

    sendAll(first.getSock(), "win:", 0);
    sendAll(first.getSock(), "disconnected:", 0);
    sendAll(second.getSock(), "lost:", 0);
    sendAll(second.getSock(), "disconnected:", 0);

    // Удаляем сокеты клиентов из списка подключенных
    connected_clients.erase(first.getSock());
    connected_clients.erase(second.getSock());
}


void Server::EventHandler(const char* buffer, size_t bytes_read, int client)
{   
    std::string message(buffer, bytes_read - 1);

    // Логгируем
    Logger::getLoggerInstance() << "Player id: " << std::to_string(client) << "\n";
    Logger::getLoggerInstance() << std::to_string(message.length()) << "\n";
    Logger::getLoggerInstance() << "recieved message: " << message.data() << "\n\n";

    if (!message.compare("connect:"))
        sendAll(client, "connected:", 0);
    else if (!message.compare("start:"))
        sendAll(client, "ok:", 0);
    else if (std::regex_match(message, std::regex("field:[0-1]{100}")))
    {
        std::string field = message.substr(std::size("field:") - 1);
        auto temp = field;
        if (isFieldValid(temp.data()))
        {
            Player player(client, field);
            if (initiators.size() == 0 || initiators.back().getEnemy() != nullptr)
                initiators.push_back(player);
            else
            {
                // Создаем соединение между двумя клиентами
                secondaries.push_back(player);
                makeGameSession(initiators.back(), secondaries.back());
            }
        }
        else // Если поле некорректно, отсылаем сообщение клиенту
            sendAll(client, "field_not_valid:", 0);
    }
    else if (std::regex_match(message, std::regex("cell:[0-9]:[0-9]")))
    {
        Player* assaulter = &(initiators.back());
        for (auto& player : initiators)
        {
            if (player.getSock() == client)
            {
                assaulter = &player;
                break;
            }
            else if (player.getEnemy()->getSock() == client)
            {
                assaulter = player.getEnemy();
                break;
            }
            else continue;
        }

        if (assaulter->getState() == PlayersState::WHAITING)
        {
            Logger::getLoggerInstance() << "Player id: " << std::to_string(client) << "\n";
            Logger::getLoggerInstance() << "You must whait!" << "\n\n";
            return;
        }
        
        auto row = std::atoi(message.data() + 5);
        auto column = std::atoi(message.data() + 7);

        // Если у оппонента в клетке: 1, то отсылаем, hit: иначе - miss:
        if (assaulter->getEnemy()->getField().data()[CELLS_IN_ROW * row + column] == '1')
        {
            sendAll(assaulter->getSock(), "hit:", 0);
            assaulter->getEnemy()->countHittedCells();

            // Если атакующий игрок поразил все корабли соперника - завершаем игру
            if (assaulter->getEnemy()->getHittedCells() == 20)
                closeGameSession(*(assaulter), *(assaulter->getEnemy()));
        }
        else
        {
            sendAll(assaulter->getSock(), "miss:", 0);
            assaulter->setPLayersState(PlayersState::WHAITING);
            assaulter->getEnemy()->setPLayersState(PlayersState::MAKING_STEP);
        }
    }
    else
    {
        // Обработка ошибочных сообщений от клиента
        Logger::getLoggerInstance() << "От клиента: "
            << std::to_string(client) << " Полученны некорректные данные: " << message.data() << "\n";
    }
}


bool Server::isFieldValid(char* field)
{
    std::array<int, 4> ships_counter = {0};
    size_t cells_counter = 0;
    bool incorrect_ship_flag = true;

    /*
        Двигаемся вдоль поля. Если ячейка непуста (часть корабля или однопалубный)
        То увеличиваем счетчик непустых ячеек (их должно оказаться ровно 20).
        Если справа от этой ячейки непусто - идем вправо пока не будет пусто
        (или стенка), параллельно считая палубы (счетчик k = 1). Если в конце
        (когда сработает одно из вышеупомянутых условий) счетчик k будет больше 4х,
        то флаг - incorrect_ship установится в true.
        Для столбцов проверка аналогична. Таким образом в конце, поле проходит
        валидацию:
        
        1) Количество заполненных ячеек соответсвует CELLS_FOR_SHIPS.
        2) Количество кораблей каждого типа совпадает с требуемым.
    */
    for (size_t i = 0, CELLS_IN_ROW = CELLS_IN_FIELD / 10; i < CELLS_IN_ROW; i++)
    {
        for (size_t j = 0, CELLS_IN_COLUMN = CELLS_IN_ROW; j < CELLS_IN_COLUMN; j++)
        {
            if (field[CELLS_IN_ROW * i + j] == '1')
            {
                cells_counter++;
                if (j < 9 && field[CELLS_IN_ROW * i + (j + 1)] == '1')
                {
                    int k = 1;
                    while (field[CELLS_IN_ROW * i + (j + k)] == '1')
                    {
                        field[CELLS_IN_ROW * i + (j + k)] = '0';
                        cells_counter++;
                        k++;
                    }
                    k < 5 ? ships_counter[k - 1]++ : incorrect_ship_flag = false;
                }
                else if (i < 9 && field[CELLS_IN_ROW * (i + 1) + j] == '1')
                {
                    int k = 1;
                    while (field[CELLS_IN_ROW * (i + k) + j] == '1')
                    {
                        field[CELLS_IN_ROW * (i + k) + j] = '0';
                        cells_counter++;
                        k++;
                    }
                    k < 5 ? ships_counter[k - 1]++ : incorrect_ship_flag = false;
                }
                else
                {
                    ships_counter[0]++;
                }
                if (!incorrect_ship_flag)
                {
                    return false;
                    std::cout << "debug1\n";
                }
            }
            else continue;
        }
    }
    if (cells_counter == CELLS_FOR_SHIPS)
    {
        if (ships_counter[0] == 4 &&
            ships_counter[1] == 3 &&
            ships_counter[2] == 2 &&
            ships_counter[3] == 1)
        {
            return true;
        }
    }
    return false;
}