#include "../include/Player.hpp"


void Player::setEnemy(Player& enemy)
{
    this->enemy = &enemy;
    enemy.enemy = this;
}


Player* Player::getEnemy()
{
    return enemy;
}


int Player::getSock() const
{
    return player_socket;
}