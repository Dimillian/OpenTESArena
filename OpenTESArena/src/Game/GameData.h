#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Clock.h"
#include "Date.h"
#include "../Entities/EntityManager.h"
#include "../Entities/Player.h"
#include "../Math/Vector2.h"
#include "../World/Location.h"
#include "../World/WorldData.h"

// Intended to be a container for the player and world data that is currently active 
// while a player is loaded (i.e., not in the main menu).

// The GameData object will be initialized only upon loading of the player, and 
// will be uninitialized when the player goes to the main menu (thus unloading
// the character resources). Whichever entry points into the "game" there are, they
// need to load data into the game data object.

class CharacterClass;
class INFFile;
class MIFFile;
class Renderer;
class TextureManager;

enum class GenderName;

class GameData
{
private:
	// The time scale determines how long or short a real-time second is. If the time 
	// scale is 5.0, then each real-time second is five game seconds, etc..
	static const double TIME_SCALE;

	std::unordered_map<Int2, std::string> textTriggers, soundTriggers;
	Player player;
	WorldData worldData;
	Location location;
	Date date;
	Clock clock;
	double fogDistance;
	// weather...
public:
	GameData(Player &&player, WorldData &&worldData, const Location &location, 
		const Date &date, const Clock &clock, double fogDistance);
	~GameData();

	// Takes a .MIF file with its associated .INF file and writes data into the given 
	// reference parameters. This overwrites parts of the existing game session.
	static void loadFromMIF(const MIFFile &mif, const INFFile &inf, Double3 &playerPosition,
		WorldData &worldData, TextureManager &textureManager, Renderer &renderer);

	// Creates a game data object used for the test world.
	static std::unique_ptr<GameData> createDefault(const std::string &playerName,
		GenderName gender, int raceID, const CharacterClass &charClass,
		int portraitID, TextureManager &textureManager, Renderer &renderer);

	// Creates a game data object with random player data for testing.
	static std::unique_ptr<GameData> createRandomPlayer(TextureManager &textureManager, 
		Renderer &renderer);

	Player &getPlayer();
	WorldData &getWorldData();
	Location &getLocation();
	const Date &getDate() const;
	const Clock &getClock() const;

	// Gets a percentage representing how far along the current day is. 0.0 is 
	// 12:00am and 0.50 is noon.
	double getDaytimePercent() const;

	double getFogDistance() const;

	// Gets the current ambient light percent, based on the current clock time and 
	// the player's location (interior/exterior). This function is intended to match
	// the actual calculation done in Arena.
	double getAmbientPercent() const;

	// A more gradual ambient percent function (maybe useful on the side sometime).
	double getBetterAmbientPercent() const;

	// Ticks the game clock (for the current time of day and date).
	void tickTime(double dt);
};

#endif
