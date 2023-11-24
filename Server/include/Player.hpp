#pragma once

#include <array>
#include <iostream>


constexpr size_t CELLS_IN_FIELD = 100;
constexpr size_t CELLS_FOR_SHIPS = 20;
constexpr size_t CELLS_IN_ROW = 10;


enum class PlayersState {
    NULL_ST = 0,
    MAKING_STEP,
    WHAITING
};


class Player
{
private:
    int player_socket;
    Player* enemy;
    std::array<char, CELLS_IN_FIELD> field = {};
    PlayersState curr_state = PlayersState::NULL_ST;
    size_t ship_defeated_cells = 0;
public:
    Player(int socket, char* buffer): player_socket(socket), enemy(nullptr)
    {
        std::copy(buffer, buffer + field.size(), field.begin());
    }
    ~Player() = default;

    bool operator==(const Player& other) { return this->player_socket == other.player_socket; }

    int getSock() const;
    char* getField() { return field.data(); }
    Player* getEnemy();
    size_t getHittedCells() const { return ship_defeated_cells; }

    void setEnemy(Player& enemy);
    void setPLayersState(PlayersState state) { curr_state = state; }
    void countHittedCells() { ++ship_defeated_cells; }
};