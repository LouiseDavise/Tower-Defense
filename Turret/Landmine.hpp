#ifndef LANDMINE_HPP
#define LANDMINE_HPP

#include "Engine/Sprite.hpp"

class Landmine : public Engine::Sprite
{
public:
    static const int Cost;
    bool activated = false;
    bool placed = false; 

    Landmine(float x, float y);
    void Update(float deltaTime) override;
};

#endif // LANDMINE_HPP
