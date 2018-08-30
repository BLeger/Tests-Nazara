#include "..\..\includes\World\Terrain.hpp"

Terrain::Terrain(Ndk::World& world, Nz::Vector2ui mapSize, int inferiorLevel, int superiorLevel) :
	m_inferiorLevel(inferiorLevel), m_superiorLevel(superiorLevel), m_mapSize(mapSize)
{
	// Materials
	Nz::MaterialRef tileset_64_32 = Nz::Material::New();
	tileset_64_32->LoadFromFile("tiles/64_32_tileset.png");
	tileset_64_32->EnableBlending(true);
	tileset_64_32->SetDstBlend(Nz::BlendFunc_InvSrcAlpha);
	tileset_64_32->SetSrcBlend(Nz::BlendFunc_SrcAlpha);
	tileset_64_32->EnableDepthWrite(false);

	Nz::MaterialRef tileset_64_64 = Nz::Material::New();
	tileset_64_64->LoadFromFile("tiles/64_64_tileset.png");
	tileset_64_64->EnableBlending(true);
	tileset_64_64->SetDstBlend(Nz::BlendFunc_InvSrcAlpha);
	tileset_64_64->SetSrcBlend(Nz::BlendFunc_SrcAlpha);
	tileset_64_64->EnableDepthWrite(false);

	Nz::MaterialRef tileset_wall = Nz::Material::New();
	tileset_wall->LoadFromFile("tiles/wall_tileset.png");
	tileset_wall->EnableBlending(true);
	tileset_wall->SetDstBlend(Nz::BlendFunc_InvSrcAlpha);
	tileset_wall->SetSrcBlend(Nz::BlendFunc_SrcAlpha);
	tileset_wall->EnableDepthWrite(false);

	int numberOfMaterials = 3;

	// Tilemaps
	for (int level = m_inferiorLevel; level < m_superiorLevel + 1; level++) {
		// Create entity
		m_groundTileMapEntities.insert(std::make_pair(level, world.CreateEntity()));
		Ndk::GraphicsComponent &groundGC = m_groundTileMapEntities[level]->AddComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent &groundNC = m_groundTileMapEntities[level]->AddComponent<Ndk::NodeComponent>();

		m_environmentTileMapEntities.insert(std::make_pair(level, world.CreateEntity()));
		Ndk::GraphicsComponent &envGC = m_environmentTileMapEntities[level]->AddComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent &envNC = m_environmentTileMapEntities[level]->AddComponent<Ndk::NodeComponent>();

		groundNC.SetPosition(Nz::Vector3f{ 0.f, mainTileSizef.y * -level, 0.f });
		envNC.SetPosition(Nz::Vector3f{ 0.f, mainTileSizef.y * -level, 0.f });

		// Create tilemap
		m_groundTileMaps.insert(std::make_pair(level, TileMap::New(m_mapSize, mainTileSizef, numberOfMaterials)));
		m_environmentTileMaps.insert(std::make_pair(level, TileMap::New(m_mapSize, mainTileSizef, numberOfMaterials)));
		
		// Add materials
		addMaterial(level, tileset_64_32, Nz::Vector2f{ 64.f, 32.f }, Nz::Vector2ui{ 1, 1 });
		addMaterial(level, tileset_64_64, Nz::Vector2f{ 64.f, 64.f }, Nz::Vector2ui{ 1, 1 });
		addMaterial(level, tileset_wall, Nz::Vector2f{ 64.f, 70.f }, Nz::Vector2ui{ 1, 1 });
		m_materialCount = 0;

		groundGC.Attach(m_groundTileMaps[level], level + 10);
		envGC.Attach(m_environmentTileMaps[level], level + 10);
	}

	// Height map
	for (int y = 0; y < m_mapSize.y; y++) {
		for (int x = 0; x < m_mapSize.x; x++) {
			m_heightMap.push_back(0);
		}
	}
}

void Terrain::EnableGroundTile(Nz::Vector2ui position, TileDef& tile, Nz::Color color)
{
	EnableTile(m_groundTileMaps.at(getHeight(position)), position, tile, color);
}

void Terrain::EnableEnvironmentTile(Nz::Vector2ui position, TileDef & tile, Nz::Color color)
{
	EnableTile(m_environmentTileMaps.at(getHeight(position)), position, tile, color);
}

void Terrain::DisableGroundTile(Nz::Vector2ui position)
{
	DisableTile(m_groundTileMaps.at(getHeight(position)), position);
}

void Terrain::DisableEnvironmentTile(Nz::Vector2ui position)
{
	DisableTile(m_environmentTileMaps.at(getHeight(position)), position);
}

void Terrain::DisableTiles()
{
	for (int level = m_inferiorLevel; level < m_superiorLevel + 1; level++) {
		DisableTiles(level);
	}
}

void Terrain::DisableTiles(int level)
{
	m_groundTileMaps[level]->DisableTiles();
	m_environmentTileMaps[level]->DisableTiles();
}

void Terrain::setHeight(Nz::Vector2ui position, int height)
{
	assert(height >= m_inferiorLevel && height <= m_superiorLevel);
	assert(position.x >= 0 && position.x < m_mapSize.x);
	assert(position.y >= 0 && position.y < m_mapSize.y);

	m_heightMap[m_mapSize.x * position.y + position.x] = height;
}

int Terrain::getHeight(Nz::Vector2ui position)
{
	assert(position.x >= 0 && position.x < m_mapSize.x);
	assert(position.y >= 0 && position.y < m_mapSize.y);

	return m_heightMap.at(m_mapSize.x * position.y + position.x);
}

void Terrain::scale(float value)
{
	for (int level = m_inferiorLevel; level < m_superiorLevel + 1; level++) {
		Ndk::EntityHandle& ground = m_groundTileMapEntities.at(level);
		Ndk::EntityHandle& environment = m_environmentTileMapEntities.at(level);

		Ndk::NodeComponent &groundNC = ground->GetComponent<Ndk::NodeComponent>();
		Ndk::NodeComponent &envNC = environment->GetComponent<Ndk::NodeComponent>();

		groundNC.SetScale(value);
		envNC.SetScale(value);
	}
}

void Terrain::EnableTile(TileMapRef& tilemap, Nz::Vector2ui position, TileDef & tile, Nz::Color color)
{
	assert(tilemap->GetMaterialCount() > tile.materialIndex);
	tilemap->DisableTile(position);
	tilemap->EnableTile(position, tile.tileIndex, color, tile.materialIndex);
}

void Terrain::DisableTile(TileMapRef & tilemap, Nz::Vector2ui position)
{
	tilemap->DisableTile(position);
}

void Terrain::addMaterial(int level, Nz::MaterialRef& material, Nz::Vector2f imageSize, Nz::Vector2ui tileSize)
{
	m_groundTileMaps[level]->SetMaterial(m_materialCount, material);
	m_groundTileMaps[level]->setMaterialData(m_materialCount, imageSize, tileSize);

	m_environmentTileMaps[level]->SetMaterial(m_materialCount, material);
	m_environmentTileMaps[level]->setMaterialData(m_materialCount, imageSize, tileSize);

	m_materialCount++;
}