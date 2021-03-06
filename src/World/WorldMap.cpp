#include "../../includes/World/WorldMap.hpp"

WorldMap::WorldMap(Nz::Vector2ui size, Ndk::World& world) : 
	m_size(size), m_worldRef(world), m_terrain(Terrain{ world, size, m_minElevation, m_maxElevation})
{
	for (unsigned int y = 0; y < m_size.y; y++) {
		for (unsigned int x = 0; x < m_size.x; x++) {
			m_tiles.push_back(TileData{ TileType::SIMPLE_TILE, 0});
		}
	}
}

void WorldMap::generateTerrain(SpriteLibrary& spriteLib)
{
	NoiseGenerator generator{ m_size };

	for (unsigned int y = 0; y < m_size.y; y++) {
		for (unsigned int x = 0; x < m_size.x; x++) {
			Nz::Vector2ui position{ x, y };

			TileData& tile = getTile(position);
			tile.groundMaterial = generator.getTile(position);

			int height = generator.getHeight(position);
			m_terrain.setHeight(position, height);

			if (tile.groundMaterial == WATER || tile.groundMaterial == DEEP_WATER) {
				tile.type = TileType::WATER_TILE;
			}

			if (generator.hasEnvTile(position)) {
				addEnvironmentTile(position, generator.getEnvTile(position));
			}
			updateTile(position);
		}
	}
}


TileData& WorldMap::getTile(Nz::Vector2ui position) {
	assert(position.x >= 0 && position.x < m_size.x);
	assert(position.y >= 0 && position.y < m_size.y);

	return m_tiles.at(m_size.x * position.y + position.x);
}

bool WorldMap::deleteEntity(Nz::Vector2ui position)
{
	if (isPositionAvailable(position)) {
		// There is already a tile at this place
		std::cout << "Err: this tile is not occupied" << std::endl;
		return false;
	}

	// Update the tile's data
	TileData& tile = getTile(position);

	/*if (tile.type != TileType::ENV_TILE) {
		std::cout << "Err: this tile is not environment" << std::endl;
		return false;
	}*/

	tile.type = TileType::SIMPLE_TILE;

	// Detach and destroy the entity
	Ndk::EntityHandle entity = m_entities.at(position);
	Ndk::GraphicsComponent &gc = entity->GetComponent<Ndk::GraphicsComponent>();
	gc.Clear();
	entity->Kill();

	// Remove the entity from the map
	m_entities.erase(m_entities.find(position));

	return true;
}

/*
 * Temporary names
 * Return correct cell hovered by the mouse according to the "supposed" cell at elevation 0
 */
Nz::Vector2ui WorldMap::getHoveredCell(Nz::Vector2ui flatCell)
{
	for (int elevation = m_minElevation; elevation < m_maxElevation + 1; elevation++) {
		if (elevation != 0) {
			Nz::Vector2ui possibleCell{ flatCell.x, flatCell.y + 2 * elevation };
			if (isPositionCorrect(possibleCell) && getTileHeight(possibleCell) == elevation)
				return possibleCell;
		}	
	}

	return flatCell;
}

int WorldMap::getTileHeight(Nz::Vector2ui position)
{
	return m_terrain.getHeight(position);
}

void WorldMap::addEnvironmentTile(Nz::Vector2ui position, TileDef env)
{
	if (!isPositionAvailable(position)) {
		std::cout << "Err: tile already occupied" << std::endl;
		return;
	}

	TileData& tile = getTile(position);
	tile.environmentMaterial = env;
	tile.type = TileType::ENVIRONMENT_TILE;
	updateTile(position);
}

void WorldMap::removeEnvironmentTile(Nz::Vector2ui position)
{
	TileData& tile = getTile(position);
	tile.environmentMaterial = VOID;
	tile.type = TileType::SIMPLE_TILE;
	updateTile(position);
}

void WorldMap::addRoad(Nz::Vector2ui position)
{
	if (!isPositionCorrect(position) || !isPositionAvailable(position))
		return;

	TileData& tile = getTile(position);
	tile.type = TileType::ROAD_TILE;
	tile.groundMaterial = ROAD;
	updateTile(position);
}

void WorldMap::removeRoad(Nz::Vector2ui position)
{
	updateTile(position);
}

bool WorldMap::isRoad(Nz::Vector2ui position)
{
	return getTile(position).type == TileType::ROAD_TILE;
}

void WorldMap::addWall(Nz::Vector2ui position)
{
	if (!isPositionAvailable(position)) {
		std::cout << "Err: tile already occupied" << std::endl;
		return;
	}
	
	Ndk::EntityHandle entity = m_worldRef.CreateEntity();
	entity->AddComponent<WallComponent>(position);
	m_entities.insert(std::make_pair(position, entity));

	getTile(position).type = TileType::WALL_TILE;

	updateSurrondingsWalls(position);
}

void WorldMap::removeWall(Nz::Vector2ui position)
{
	deleteEntity(position);
	updateSurrondingsWalls(position);
	updateTile(position);
}

bool WorldMap::isWall(Nz::Vector2ui position)
{
	if (!isPositionCorrect(position))
		return false;

	if (m_entities.find(position) == m_entities.end())
		return false;

	Ndk::EntityHandle& entity = m_entities.at(position);
	return entity->HasComponent<WallComponent>();
}

void WorldMap::updateSurrondingsWalls(Nz::Vector2ui position)
{
	std::vector<Nz::Vector2ui> surrondings = Isometric::getSurroundingTiles(position);

	for (Nz::Vector2ui pos : surrondings) {
		if (isWall(pos)) {
			Ndk::EntityHandle& entity = m_entities.at(pos);
			WallComponent &wall = entity->GetComponent<WallComponent>();
			wall.m_needsUpdate = true;
		}
	}
}

WalkerComponent& WorldMap::addWalker(Nz::Vector2ui position, Nz::SpriteRef& sprite)
{
	Ndk::EntityHandle entity = m_worldRef.CreateEntity();
	Ndk::NodeComponent &nc = entity->AddComponent<Ndk::NodeComponent>();

	Nz::Vector2ui pixelPosition = Isometric::cellToPixel(position, m_scale);
	nc.SetPosition(m_cameraOffset);

	Ndk::GraphicsComponent &gc = entity->AddComponent<Ndk::GraphicsComponent>();
	WalkerComponent &wc = entity->AddComponent<WalkerComponent>(position);
	AnimationComponent &ac = entity->AddComponent<AnimationComponent>(sprite, Nz::Vector2f{ 43.f, 64.f });

	gc.Attach(ac.getSprite());
	ac.enable();
	m_walkers.push_back(entity);

	return wc;
}

void WorldMap::updateTile(Nz::Vector2ui position)
{
	TileData& tile = getTile(position);

	m_terrain.EnableGroundTile(position, tile.groundMaterial);

	if (tile.environmentMaterial != VOID) {
		m_terrain.EnableEnvironmentTile(position, tile.environmentMaterial);
	}
	else {
		m_terrain.DisableEnvironmentTile(position);
	}
}

bool WorldMap::isPositionCorrect(Nz::Vector2ui position)
{
	return position.x >= 0 && position.y >= 0 && position.x < m_size.x && position.y < m_size.y;
}

bool WorldMap::isPositionAvailable(Nz::Vector2ui position)
{
	assert(isPositionCorrect(position));

	TileData& tile = getTile(position);
	return tile.type == TileType::SIMPLE_TILE;
}

bool WorldMap::arePositionsAvailable(std::vector<Nz::Vector2ui> positions)
{
	for (Nz::Vector2ui pos : positions) {
		if (!isPositionCorrect(pos) || !isPositionAvailable(pos))
			return false;
	}

	return true;
}

void WorldMap::previewEntity(Nz::Vector2ui position, TileDef tile)
{
	TileData& t = getTile(position);

	if (t.type == TileType::BUILDING_BODY) {
		// If the tile is part of a building, we need to get the position of it's root to display it red
		BuildingComponent building = *(t.building);
		Nz::Vector2ui buildingRootPosition = building.getPosition();

		TileData& rootTile = getTile(buildingRootPosition);
		m_terrain.EnableEnvironmentTile(buildingRootPosition, rootTile.environmentMaterial, Nz::Color::Red);
		m_previewPositions.push_back(buildingRootPosition);

	} else if (t.environmentMaterial == VOID) {

		// If the tiles takes several cells, we need to check them all
		if (tile.tileSize != Nz::Vector2ui{ 1, 1 }) {
			std::vector<Nz::Vector2ui> cells = Isometric::square(position, tile.tileSize);
			if (!arePositionsAvailable(cells)) {
				m_terrain.EnableEnvironmentTile(position, tile, Nz::Color::Red);
				m_previewPositions.push_back(position);
				return;
			}
		}

		// If tile is empty, preview the possible tile in green
		m_terrain.EnableEnvironmentTile(position, tile, Nz::Color::Green);
		m_previewPositions.push_back(position);
	}
	else {
		// If the tile is occupied, display the curent tile in red
		m_terrain.EnableEnvironmentTile(position, t.environmentMaterial, Nz::Color::Red);
		m_previewPositions.push_back(position);
	}
}

void WorldMap::resetPreview()
{
	for (Nz::Vector2ui position : m_previewPositions) {
		TileData& t = getTile(position);

		if (t.environmentMaterial == VOID) {
			m_terrain.DisableEnvironmentTile(position);
		}
		else {
			m_terrain.EnableEnvironmentTile(position, t.environmentMaterial);
		}
	}

	m_previewPositions.clear();
}

void WorldMap::setTileDef(Nz::Vector2ui position, TileDef tile)
{
	TileData& t = getTile(position);

	// TEMPORARY
	//assert(t.type == TileType::WALL_TILE);
	t.environmentMaterial = tile;
}

void WorldMap::addBuilding(Nz::Vector2ui position, const TileDef tile) 
{
	// Cells occupied by the building
	std::vector<Nz::Vector2ui> cells = Isometric::square(position, tile.tileSize);
	
	if (!arePositionsAvailable(cells)) {
		std::cout << "Err: tile already occupied" << std::endl;
		return;
	}
	
	// Create building entity
	Ndk::EntityHandle entity = m_worldRef.CreateEntity();

	BuildingComponent &building = entity->AddComponent<BuildingComponent>(position, tile);
	entity->AddComponent<ResidentialBuildingComponent>();
	m_buildings.insert(std::make_pair(position, entity));
	setTileDef(building.getPosition(), building.getTileDef());
	
	// Update the data of all the tiles below the building
	for (Nz::Vector2ui pos : cells) {
		TileData& tile = getTile(pos);
		
		tile.type = TileType::BUILDING_BODY;
		tile.building = &building;
	}

	getTile(position).type = TileType::BUILDING_ROOT;
}

void WorldMap::removeBuilding(Nz::Vector2ui position)
{
	TileData& tile = getTile(position);

	// If this tile is in the body of a building but not it's root
	// We need to get the root position in order to delete the entity
	if (tile.type == TileType::BUILDING_BODY) {
		BuildingComponent* building = tile.building;
		position = building->getPosition();
		tile = getTile(position);
	}

	if (tile.type != TileType::BUILDING_ROOT) {
		std::cout << "Err: This is not a building" << std::endl;
		return;
	}

	// Delete the entity
	Ndk::EntityHandle& entity = m_buildings.at(position);
	entity->Kill();

	m_buildings.erase(m_buildings.find(position));

	// Delete the tile on tilemap
	m_terrain.DisableEnvironmentTile(position);

	// Update all tile's data
	for (Nz::Vector2ui pos : Isometric::square(position, tile.environmentMaterial.tileSize)) {
		TileData& t = getTile(pos);
		t.type = TileType::SIMPLE_TILE;
		t.environmentMaterial = VOID;
		t.building = nullptr;
	}	
}

float WorldMap::getScale()
{
	return m_scale;
}

void WorldMap::zoom(float delta)
{
	m_scale += delta * 0.1f;

	if (m_scale > m_maxScale)
		m_scale = m_maxScale;

	if (m_scale < m_minScale)
		m_scale = m_minScale;

	m_terrain.scale(m_scale);
}

void WorldMap::setCameraOffset(Nz::Vector2f offset)
{
	m_cameraOffset = offset;
}

Nz::Vector2f WorldMap::getCameraOffset()
{
	return m_cameraOffset;
}