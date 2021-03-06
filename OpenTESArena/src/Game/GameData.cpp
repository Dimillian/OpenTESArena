#include <algorithm>
#include <cassert>
#include <cmath>

#include "SDL.h"

#include "GameData.h"

#include "../Assets/ExeStrings.h"
#include "../Assets/INFFile.h"
#include "../Assets/MIFFile.h"
#include "../Entities/Animation.h"
#include "../Entities/CharacterClassParser.h"
#include "../Entities/Doodad.h"
#include "../Entities/Entity.h"
#include "../Entities/EntityManager.h"
#include "../Entities/GenderName.h"
#include "../Entities/NonPlayer.h"
#include "../Entities/Player.h"
#include "../Items/WeaponType.h"
#include "../Math/Constants.h"
#include "../Math/Random.h"
#include "../Media/PaletteFile.h"
#include "../Media/PaletteName.h"
#include "../Media/TextureManager.h"
#include "../Rendering/Renderer.h"
#include "../Utilities/Debug.h"
#include "../World/ClimateName.h"
#include "../World/LocationType.h"
#include "../World/VoxelGrid.h"

// Arbitrary value for testing. One real second = six game minutes.
// The value used in Arena is one real second = twenty game seconds.
const double GameData::TIME_SCALE = static_cast<double>(Clock::SECONDS_IN_A_DAY) / 240.0;

GameData::GameData(Player &&player, WorldData &&worldData, const Location &location,
	const Date &date, const Clock &clock, double fogDistance)
	: player(std::move(player)), worldData(std::move(worldData)), location(location),
	date(date), clock(clock)
{
	DebugMention("Initializing.");

	this->fogDistance = fogDistance;
}

GameData::~GameData()
{
	DebugMention("Closing.");
}

void GameData::loadFromMIF(const MIFFile &mif, const INFFile &inf, Double3 &playerPosition,
	WorldData &worldData, TextureManager &textureManager, Renderer &renderer)
{
	// Convert start point to new coordinate system and set player's location 
	// (player Y value is arbitrary for now).
	const auto &startPoints = mif.getStartPoints();
	const Double2 startPoint = VoxelGrid::arenaVoxelToNewVoxel(
		startPoints.front(), mif.getWidth(), mif.getDepth());
	playerPosition = Double3(startPoint.x, playerPosition.y, startPoint.y);

	// Clear all entities.
	auto &entityManager = worldData.getEntityManager();
	for (const auto *entity : entityManager.getAllEntities())
	{
		renderer.removeFlat(entity->getID());
		entityManager.remove(entity->getID());
	}

	// Clear software renderer textures (so the .INF file indices are correct).
	//renderer.removeAllWorldTextures(); // To do: Uncomment once .INF files are in use.

	worldData = std::move(WorldData(mif, inf));
}

std::unique_ptr<GameData> GameData::createDefault(const std::string &playerName,
	GenderName gender, int raceID, const CharacterClass &charClass, int portraitID,
	TextureManager &textureManager, Renderer &renderer)
{
	// Create some dummy data for the test world.

	// Some arbitrary player values.
	const Double3 position = Double3(1.50, 1.70, 12.50);
	const Double3 direction = Double3(1.0, 0.0, 0.0).normalized();
	const Double3 velocity = Double3(0.0, 0.0, 0.0);
	const double maxWalkSpeed = 2.0;
	const double maxRunSpeed = 8.0;
	const WeaponType weaponType = []()
	{
		// Pick a random weapon type for testing.
		const std::vector<WeaponType> types =
		{
			WeaponType::BattleAxe,
			WeaponType::Broadsword,
			WeaponType::Fists,
			WeaponType::Flail,
			WeaponType::Mace,
			WeaponType::Staff,
			WeaponType::Warhammer
		};

		Random random;
		int index = random.next(static_cast<int>(types.size()));
		return types.at(index);
	}();

	Player player(playerName, gender, raceID, charClass, portraitID,
		position, direction, velocity, maxWalkSpeed, maxRunSpeed, weaponType);

	// Add some wall textures.
	textureManager.setPalette(PaletteFile::fromName(PaletteName::Default));
	std::vector<const SDL_Surface*> surfaces = {
		// Texture indices:
		// 0: city wall
		textureManager.getSurface("CITYWALL.IMG"),

		// 1: sea wall
		textureManager.getSurface("SEAWALL.IMG"),

		// 2-4: grounds
		textureManager.getSurfaces("NORM1.SET").at(0),
		textureManager.getSurfaces("NORM1.SET").at(1),
		textureManager.getSurfaces("NORM1.SET").at(2),

		// 5-6: gates
		textureManager.getSurface("DLGT.IMG"),
		textureManager.getSurface("DRGT.IMG"),

		// 7-10: tavern + door
		textureManager.getSurfaces("MTAVERN.SET").at(0),
		textureManager.getSurfaces("MTAVERN.SET").at(1),
		textureManager.getSurfaces("MTAVERN.SET").at(2),
		textureManager.getSurface("DTAV.IMG"),

		// 11-16: temple + door
		textureManager.getSurfaces("MTEMPLE.SET").at(0),
		textureManager.getSurfaces("MTEMPLE.SET").at(1),
		textureManager.getSurfaces("MTEMPLE.SET").at(2),
		textureManager.getSurfaces("MTEMPLE.SET").at(3),
		textureManager.getSurfaces("MTEMPLE.SET").at(4),
		textureManager.getSurface("DTEP.IMG"),

		// 17-22: Mage's Guild + door
		textureManager.getSurfaces("MMUGUILD.SET").at(0),
		textureManager.getSurfaces("MMUGUILD.SET").at(1),
		textureManager.getSurfaces("MMUGUILD.SET").at(2),
		textureManager.getSurfaces("MMUGUILD.SET").at(3),
		textureManager.getSurfaces("MMUGUILD.SET").at(4),
		textureManager.getSurface("DMU.IMG"),

		// 23-26: Equipment store + door
		textureManager.getSurfaces("MEQUIP.SET").at(0),
		textureManager.getSurfaces("MEQUIP.SET").at(1),
		textureManager.getSurfaces("MEQUIP.SET").at(2),
		textureManager.getSurface("DEQ.IMG"),

		// 27-31: Low house + door
		textureManager.getSurfaces("MBS1.SET").at(0),
		textureManager.getSurfaces("MBS1.SET").at(1),
		textureManager.getSurfaces("MBS1.SET").at(2),
		textureManager.getSurfaces("MBS1.SET").at(3),
		textureManager.getSurface("DBS1.IMG"),

		// 32-35: Medium house + door
		textureManager.getSurfaces("MBS3.SET").at(0),
		textureManager.getSurfaces("MBS3.SET").at(1),
		textureManager.getSurfaces("MBS3.SET").at(2),
		textureManager.getSurface("DBS3.IMG"),

		// 36-39: Noble house + door
		textureManager.getSurfaces("MNOBLE.SET").at(0),
		textureManager.getSurfaces("MNOBLE.SET").at(1),
		textureManager.getSurfaces("MNOBLE.SET").at(2),
		textureManager.getSurface("DNB1.IMG"),

		// 40: Hedge
		textureManager.getSurface("HEDGE.IMG"),

		// 41-42: Bridge
		textureManager.getSurface("TTOWER.IMG"),
		textureManager.getSurface("NBRIDGE.IMG"),
	};

	for (const auto *surface : surfaces)
	{
		renderer.addTexture(static_cast<uint32_t*>(surface->pixels),
			surface->w, surface->h);
	}

	// Make an empty voxel grid with some arbitrary dimensions.
	const int gridWidth = 24;
	const int gridHeight = 5;
	const int gridDepth = 24;
	VoxelGrid voxelGrid(gridWidth, gridHeight, gridDepth);

	// Add some voxel data for the voxel grid's IDs to refer to. The first voxel data 
	// is a placeholder for "empty voxels", so subtract 1 from the wall ID to get the
	// texture index.
	// - A wall/floor/ceiling ID of 0 indicates air. The "empty voxel" is defined
	//   as having air for each voxel face, and is ignored during rendering.
	const int emptyID = voxelGrid.addVoxelData(VoxelData(0));

	// City wall.
	const int cityWallID = voxelGrid.addVoxelData(VoxelData(1));

	// Ground (with sea wall).
	const int gravelID = voxelGrid.addVoxelData(VoxelData(2, 0, 3));
	const int roadID = voxelGrid.addVoxelData(VoxelData(2, 0, 4));
	const int grassID = voxelGrid.addVoxelData(VoxelData(2, 0, 5));

	// Tavern.
	const int tavern1ID = voxelGrid.addVoxelData(VoxelData(8));
	const int tavern2ID = voxelGrid.addVoxelData(VoxelData(9));
	const int tavern3ID = voxelGrid.addVoxelData(VoxelData(10));
	const int tavernDoorID = voxelGrid.addVoxelData(VoxelData(11));

	// Temple.
	const int temple1ID = voxelGrid.addVoxelData(VoxelData(12));
	const int temple2ID = voxelGrid.addVoxelData(VoxelData(13));
	const int temple3ID = voxelGrid.addVoxelData(VoxelData(14));
	const int temple4ID = voxelGrid.addVoxelData(VoxelData(15));
	const int temple5ID = voxelGrid.addVoxelData(VoxelData(16));
	const int templeDoorID = voxelGrid.addVoxelData(VoxelData(17));

	// Mage's guild.
	const int mages1ID = voxelGrid.addVoxelData(VoxelData(18));
	const int mages2ID = voxelGrid.addVoxelData(VoxelData(19));
	const int mages3ID = voxelGrid.addVoxelData(VoxelData(20));
	const int mages4ID = voxelGrid.addVoxelData(VoxelData(21));
	const int mages5ID = voxelGrid.addVoxelData(VoxelData(22));
	const int magesDoorID = voxelGrid.addVoxelData(VoxelData(23));

	// Equipment store.
	const int equip1ID = voxelGrid.addVoxelData(VoxelData(24));
	const int equip2ID = voxelGrid.addVoxelData(VoxelData(25));
	const int equip3ID = voxelGrid.addVoxelData(VoxelData(26));
	const int equipDoorID = voxelGrid.addVoxelData(VoxelData(27));

	// Low house.
	const int lowHouse1ID = voxelGrid.addVoxelData(VoxelData(28));
	const int lowHouse2ID = voxelGrid.addVoxelData(VoxelData(29));
	const int lowHouse3ID = voxelGrid.addVoxelData(VoxelData(30));
	const int lowHouse4ID = voxelGrid.addVoxelData(VoxelData(31));
	const int lowHouseDoorID = voxelGrid.addVoxelData(VoxelData(32));

	// Medium house.
	const int medHouse1ID = voxelGrid.addVoxelData(VoxelData(33));
	const int medHouse2ID = voxelGrid.addVoxelData(VoxelData(34));
	const int medHouse3ID = voxelGrid.addVoxelData(VoxelData(35));
	const int medHouseDoorID = voxelGrid.addVoxelData(VoxelData(36));

	// Noble house.
	const int noble1ID = voxelGrid.addVoxelData(VoxelData(37));
	const int noble2ID = voxelGrid.addVoxelData(VoxelData(38));
	const int noble3ID = voxelGrid.addVoxelData(VoxelData(39));
	const int nobleDoorID = voxelGrid.addVoxelData(VoxelData(40));

	// Hedge.
	const int hedgeID = voxelGrid.addVoxelData(VoxelData(41, 0, 0));

	// Bridge.
	const int bridge1ID = voxelGrid.addVoxelData(VoxelData(42, 43, 43, 0.0, 0.125, 0.875, 1.0));
	const int bridge2ID = voxelGrid.addVoxelData(VoxelData(42, 43, 43, 0.10, 0.125, 0.775, 0.90));

	// Lambda for setting a voxel at some coordinate to some ID.
	auto setVoxel = [&voxelGrid](int x, int y, int z, int id)
	{
		char *voxels = voxelGrid.getVoxels();
		voxels[x + (y * voxelGrid.getWidth()) +
			(z * voxelGrid.getWidth() * voxelGrid.getHeight())] = id;
	};

	// Set voxel IDs with indices into the voxel data.
	// City walls.
	for (int j = 0; j < (gridHeight - 1); ++j)
	{
		for (int k = 0; k < gridDepth; ++k)
		{
			setVoxel(0, j, k, cityWallID);
			setVoxel(gridWidth - 1, j, k, cityWallID);
		}
	}

	for (int j = 0; j < (gridHeight - 1); ++j)
	{
		for (int i = 0; i < gridWidth; ++i)
		{
			setVoxel(i, j, 0, cityWallID);
			setVoxel(i, j, gridDepth - 1, cityWallID);
		}
	}

	// Grass fill.
	for (int k = 1; k < (gridDepth - 1); ++k)
	{
		for (int i = 1; i < (gridWidth - 1); ++i)
		{
			setVoxel(i, 0, k, grassID);
		}
	}

	// Road.
	for (int i = 1; i < (gridWidth - 1); ++i)
	{
		setVoxel(i, 0, 11, roadID);
		setVoxel(i, 0, 12, roadID);
		setVoxel(i, 0, 13, roadID);
	}

	// Trench (with water eventually).
	for (int k = 1; k < (gridDepth - 1); ++k)
	{
		setVoxel(11, 0, k, emptyID);
		setVoxel(12, 0, k, emptyID);
	}

	// Random number generator with an arbitrary seed.
	Random random(0);

	// Tavern.
	for (int k = 5; k < 10; ++k)
	{
		for (int j = 1; j < 3; ++j)
		{
			for (int i = 2; i < 6; ++i)
			{
				setVoxel(i, j, k, tavern1ID + random.next(3));
			}
		}
	}

	// Tavern door.
	setVoxel(3, 1, 9, tavernDoorID);

	// Tavern gravel.
	setVoxel(3, 0, 10, gravelID);

	// Temple.
	for (int k = 2; k < 10; ++k)
	{
		for (int j = 1; j < 4; ++j)
		{
			for (int i = 7; i < 10; ++i)
			{
				setVoxel(i, j, k, temple1ID + random.next(5));
			}
		}
	}

	// Temple door.
	setVoxel(8, 1, 9, templeDoorID);

	// Temple gravel.
	setVoxel(8, 0, 10, gravelID);

	// Mages' guild.
	for (int k = 15; k < 20; ++k)
	{
		for (int j = 1; j < 3; ++j)
		{
			for (int i = 7; i < 10; ++i)
			{
				setVoxel(i, j, k, mages1ID + random.next(5));
			}
		}
	}

	// Mages' guild door.
	setVoxel(8, 1, 15, magesDoorID);

	// Mages' guild gravel.
	setVoxel(8, 0, 14, gravelID);

	// Equipment store.
	for (int k = 15; k < 19; ++k)
	{
		for (int j = 1; j < 2; ++j)
		{
			for (int i = 2; i < 5; ++i)
			{
				setVoxel(i, j, k, equip1ID + random.next(3));
			}
		}
	}

	// Equipment store door.
	setVoxel(3, 1, 15, equipDoorID);

	// Equipment store gravel.
	setVoxel(3, 0, 14, gravelID);

	// Low house.
	for (int k = 15; k < 20; ++k)
	{
		for (int j = 1; j < 2; ++j)
		{
			for (int i = 14; i < 18; ++i)
			{
				setVoxel(i, j, k, lowHouse1ID + random.next(4));
			}
		}
	}

	// Low house door.
	setVoxel(15, 1, 15, lowHouseDoorID);

	// Low house gravel.
	setVoxel(15, 0, 14, gravelID);

	// Medium house.
	for (int k = 15; k < 19; ++k)
	{
		for (int j = 1; j < 3; ++j)
		{
			for (int i = 19; i < 22; ++i)
			{
				setVoxel(i, j, k, medHouse1ID + random.next(3));
			}
		}
	}

	// Medium house door.
	setVoxel(20, 1, 15, medHouseDoorID);

	// Medium house gravel.
	setVoxel(20, 0, 14, gravelID);

	// Noble house.
	for (int k = 4; k < 9; ++k)
	{
		for (int j = 1; j < 3; ++j)
		{
			for (int i = 16; i < 20; ++i)
			{
				setVoxel(i, j, k, noble1ID + random.next(3));
			}
		}
	}

	// Noble house door.
	setVoxel(17, 1, 8, nobleDoorID);

	// Noble house gravel.
	setVoxel(17, 0, 9, gravelID);
	setVoxel(17, 0, 10, gravelID);

	// Noble house hedges.
	for (int k = 2; k < 10; ++k)
	{
		setVoxel(14, 1, k, hedgeID);
		setVoxel(21, 1, k, hedgeID);
	}

	for (int i = 15; i < 21; ++i)
	{
		setVoxel(i, 1, 2, hedgeID);
	}

	// Bridge.
	for (int k = 11; k < 14; ++k)
	{
		setVoxel(10, 1, k, bridge1ID);
		setVoxel(11, 1, k, bridge2ID);
		setVoxel(12, 1, k, bridge2ID);
		setVoxel(13, 1, k, bridge1ID);
	}

	// Lambdas for adding a new texture to the renderer and returning the assigned ID.
	auto addTexture = [&textureManager, &renderer](const std::string &filename)
	{
		const SDL_Surface *surface = textureManager.getSurface(filename);
		return renderer.addTexture(static_cast<const uint32_t*>(surface->pixels),
			surface->w, surface->h);
	};

	auto addTextures = [&textureManager, &renderer](const std::string &filename)
	{
		const std::vector<SDL_Surface*> &surfaces = textureManager.getSurfaces(filename);

		std::vector<int> textureIDs;
		for (const auto *surface : surfaces)
		{
			const int textureID = renderer.addTexture(
				static_cast<const uint32_t*>(surface->pixels), surface->w, surface->h);
			textureIDs.push_back(textureID);
		}

		return textureIDs;
	};

	// Flat texture properties.
	const int tree1TextureID = addTexture("NPINE1.IMG");
	const int tree2TextureID = addTexture("NPINE4.IMG");
	const int statueTextureID = addTexture("NSTATUE1.IMG");
	const std::vector<int> lampPostTextureIDs = addTextures("NLAMP1.DFA");
	const std::vector<int> womanTextureIDs = addTextures("FMGEN01.CFA"); // To do: Allow sub-ranges.
	const std::vector<int> manTextureIDs = addTextures("MLGEN01W.CFA");

	const double tree1Scale = 2.0;
	const double tree2Scale = 2.0;
	const double statueScale = 1.0;
	const double lampPostScale = 0.90;
	const double womanScale = 0.80;
	const double manScale = 0.80;

	// Lambdas for adding entities to the entity manager and renderer (they can have
	// more parameters in the future as entities grow more complex).
	EntityManager entityManager;

	auto addDoodad = [&entityManager, &renderer](const Double3 &position, double width,
		double height, const std::vector<int> &textureIDs)
	{
		const double timePerFrame = 0.10;
		Animation animation(textureIDs, timePerFrame, true);

		std::unique_ptr<Doodad> doodad(new Doodad(animation, position, entityManager));

		// Assign the entity ID with the first texture.
		renderer.addFlat(doodad->getID(), position, Double2::UnitX,
			width, height, textureIDs.at(0));

		entityManager.add(std::move(doodad));
	};

	auto addNonPlayer = [&entityManager, &renderer](const Double3 &position,
		const Double2 &direction, double width, double height,
		const std::vector<int> &idleIDs, const std::vector<int> &moveIDs,
		const std::vector<int> &attackIDs, const std::vector<int> &deathIDs)
	{
		// Eventually, "idleIDs" and "moveIDs" should be vector<vector<int>>.
		std::vector<Animation> idleAnimations, moveAnimations;

		const double timePerFrame = 0.33;
		idleAnimations.push_back(Animation(idleIDs, timePerFrame, true));
		moveAnimations.push_back(Animation(moveIDs, timePerFrame, true));

		Animation attackAnimation(attackIDs, timePerFrame, false);
		Animation deathAnimation(deathIDs, timePerFrame, false);

		std::unique_ptr<NonPlayer> nonPlayer(new NonPlayer(
			position, direction, idleAnimations, moveAnimations, attackAnimation,
			deathAnimation, entityManager));

		// Assign the entity ID with the first texture.
		renderer.addFlat(nonPlayer->getID(), position, direction,
			width, height, idleIDs.at(0));

		entityManager.add(std::move(nonPlayer));
	};

	// Add entities.
	addDoodad(Double3(2.50, 1.0, 21.50), 0.88 * tree1Scale, 1.37 * tree1Scale, { tree1TextureID });
	addDoodad(Double3(9.50, 1.0, 21.50), 0.66 * tree2Scale, 1.32 * tree2Scale, { tree2TextureID });
	addDoodad(Double3(2.50, 1.0, 2.50), 0.66 * tree2Scale, 1.32 * tree2Scale, { tree2TextureID });
	addDoodad(Double3(20.50, 1.0, 21.50), 0.88 * tree1Scale, 1.37 * tree1Scale, { tree1TextureID });
	addDoodad(Double3(6.50, 1.0, 12.50), 0.74 * statueScale, 1.38 * statueScale, { statueTextureID });
	addDoodad(Double3(5.50, 1.0, 10.50), 0.64 * lampPostScale, 1.03 * lampPostScale, lampPostTextureIDs);
	addDoodad(Double3(9.50, 1.0, 14.50), 0.64 * lampPostScale, 1.03 * lampPostScale, lampPostTextureIDs);
	addDoodad(Double3(18.50, 1.0, 9.50), 0.64 * lampPostScale, 1.03 * lampPostScale, lampPostTextureIDs);
	addDoodad(Double3(17.50, 1.0, 14.50), 0.64 * lampPostScale, 1.03 * lampPostScale, lampPostTextureIDs);

	addNonPlayer(Double3(4.50, 1.0, 13.50), Double2(1.0, 0.0),
		0.44 * womanScale, 1.04 * womanScale, womanTextureIDs, womanTextureIDs, {}, {});
	addNonPlayer(Double3(4.50, 1.0, 11.50), Double2(1.0, 0.0),
		0.52 * manScale, 0.99 * manScale, manTextureIDs, manTextureIDs, {}, {});

	// Fog distance is changed infrequently, so it can go here in scene creation.
	// It's not an expensive operation for the software renderer.
	const double fogDistance = 18.0;
	renderer.setFogDistance(fogDistance);

	// The sky palette is used to color the sky and fog. The renderer chooses
	// which color to use based on the time of day. Interiors should just have
	// one pixel as the sky palette (usually black).
	const std::vector<uint32_t> fullSkyPalette = [&textureManager]()
	{
		// The palettes in the data files only cover half of the day, so some added
		// darkness is needed for the other half.
		const SDL_Surface *skyPalette = textureManager.getSurface("DAYTIME.COL");
		const uint32_t *skyPalettePixels = static_cast<const uint32_t*>(skyPalette->pixels);
		const int skyPaletteSize = skyPalette->w * skyPalette->h;

		std::vector<uint32_t> fullPalette(skyPaletteSize * 2);

		// Fill with darkness.
		const uint32_t darkness = skyPalettePixels[0];
		std::fill(fullPalette.begin(), fullPalette.end(), darkness);

		// Copy the sky palette over the center of the full palette.
		std::copy(skyPalettePixels, skyPalettePixels + skyPaletteSize,
			fullPalette.data() + (fullPalette.size() / 4));

		return fullPalette;
	}();

	renderer.setSkyPalette(fullSkyPalette.data(), static_cast<int>(fullSkyPalette.size()));

	Location location("Test City", player.getRaceID(),
		LocationType::CityState, ClimateName::Cold);

	// Start the date on the first day of the first month.
	const int month = 0;
	const int day = 0;
	Date date(month, day);

	// Start the clock at 5:00am.
	Clock clock(5, 0, 0);

	return std::unique_ptr<GameData>(new GameData(
		std::move(player), WorldData(std::move(voxelGrid), std::move(entityManager)),
		location, date, clock, fogDistance));
}

std::unique_ptr<GameData> GameData::createRandomPlayer(TextureManager &textureManager,
	Renderer &renderer)
{
	Random random;
	const std::string playerName = "Player";
	const GenderName gender = (random.next(2) == 0) ? GenderName::Male : GenderName::Female;
	const int raceID = random.next(8);

	const auto charClasses = CharacterClassParser::parse();
	const CharacterClass &charClass = charClasses.at(
		random.next() % static_cast<int>(charClasses.size()));

	const int portraitID = random.next(10);

	return GameData::createDefault(playerName, gender, raceID, charClass,
		portraitID, textureManager, renderer);
}

Player &GameData::getPlayer()
{
	return this->player;
}

WorldData &GameData::getWorldData()
{
	return this->worldData;
}

Location &GameData::getLocation()
{
	return this->location;
}

const Date &GameData::getDate() const
{
	return this->date;
}

const Clock &GameData::getClock() const
{
	return this->clock;
}

double GameData::getDaytimePercent() const
{
	return this->clock.getPreciseTotalSeconds() /
		static_cast<double>(Clock::SECONDS_IN_A_DAY);
}

double GameData::getFogDistance() const
{
	return this->fogDistance;
}

double GameData::getAmbientPercent() const
{
	if (this->location.getClimateName() == ClimateName::Interior)
	{
		// Completely dark indoors (some places might be an exception to this, and those
		// would be handled eventually).
		return 0.0;
	}
	else
	{
		// The ambient light outside depends on the clock time.
		const double clockPreciseSeconds = this->clock.getPreciseTotalSeconds();

		// Time ranges where the ambient light changes. The start times are inclusive,
		// and the end times are exclusive.
		const double startBrighteningTime =
			Clock::AmbientStartBrightening.getPreciseTotalSeconds();
		const double endBrighteningTime =
			Clock::AmbientEndBrightening.getPreciseTotalSeconds();
		const double startDimmingTime =
			Clock::AmbientStartDimming.getPreciseTotalSeconds();
		const double endDimmingTime =
			Clock::AmbientEndDimming.getPreciseTotalSeconds();

		// In Arena, the min ambient is 0 and the max ambient is 1, but we're using
		// some values here that make testing easier.
		const double minAmbient = 0.20;
		const double maxAmbient = 0.90;

		if ((clockPreciseSeconds >= endBrighteningTime) &&
			(clockPreciseSeconds < startDimmingTime))
		{
			// Daytime ambient.
			return maxAmbient;
		}
		else if ((clockPreciseSeconds >= startBrighteningTime) &&
			(clockPreciseSeconds < endBrighteningTime))
		{
			// Interpolate brightening light (in the morning).
			const double timePercent = (clockPreciseSeconds - startBrighteningTime) /
				(endBrighteningTime - startBrighteningTime);
			return minAmbient + ((maxAmbient - minAmbient) * timePercent);
		}
		else if ((clockPreciseSeconds >= startDimmingTime) &&
			(clockPreciseSeconds < endDimmingTime))
		{
			// Interpolate dimming light (in the evening).
			const double timePercent = (clockPreciseSeconds - startDimmingTime) /
				(endDimmingTime - startDimmingTime);
			return maxAmbient + ((minAmbient - maxAmbient) * timePercent);
		}
		else
		{
			// Night ambient.
			return minAmbient;
		}
	}
}

double GameData::getBetterAmbientPercent() const
{
	const double daytimePercent = this->getDaytimePercent();
	const double minAmbient = 0.20;
	const double maxAmbient = 0.90;
	const double diff = maxAmbient - minAmbient;
	const double center = minAmbient + (diff / 2.0);
	return center + ((diff / 2.0) * -std::cos(daytimePercent * (2.0 * PI)));
}

void GameData::tickTime(double dt)
{
	assert(dt >= 0.0);

	// Tick the game clock.
	const int oldHour = this->clock.getHours24();
	this->clock.tick(dt * GameData::TIME_SCALE);
	const int newHour = this->clock.getHours24();

	// Check if the clock hour looped back around.
	if (newHour < oldHour)
	{
		// Increment the day.
		this->date.incrementDay();
	}
}
