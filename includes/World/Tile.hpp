#ifndef TILE_H
#define TILE_H

#include <NDK/Application.hpp>
#include <NDK/Entity.hpp>
#include <NDK/State.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/World.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Utility.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>

enum TileType {
	Tile_1x1,
	Object_NxN_Attach,
	Object_NxN_Body
};

class Tile {

public:
	Tile() {};
	Tile(TileType type);

	Nz::MaterialRef getMaterial();

private:
	TileType m_type;
};

#endif // !TILE_H
