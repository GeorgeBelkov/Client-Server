#pragma once

#include <array>
#include <iostream>
#include <algorithm>


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
    std::string field;
    PlayersState curr_state = PlayersState::NULL_ST;
    size_t ship_defeated_cells = 0;

    Player& operator=(const Player& other) = delete;
public:
    Player(int socket, std::string buffer): player_socket(socket), enemy(nullptr), field(buffer) {}
    ~Player() = default;

    bool operator==(const Player& other) { return player_socket == other.player_socket; }

    int getSock() const { return player_socket; }
    std::string& getField() { return field; }
    Player* getEnemy() { return enemy; }
    size_t getHittedCells() const { return ship_defeated_cells; }

    void setEnemy(Player& player) { enemy = &player; }
    void setPLayersState(PlayersState state) { curr_state = state; }
    void countHittedCells() { ++ship_defeated_cells; }
    PlayersState getState() const { return curr_state; }
};