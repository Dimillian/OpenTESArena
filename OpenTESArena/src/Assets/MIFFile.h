#ifndef MIF_FILE_H
#define MIF_FILE_H

#include <cstdint>
#include <string>
#include <vector>

#include "../Math/Vector2.h"

// A MIF file contains map information. It defines the dimensions of a particular area 
// and which voxels have which IDs, as well as some other data. It is normally paired with 
// an INF file that tells which textures to use, among other things.

// It is composed of a map header and an array of levels.

class MIFFile
{
public:
	struct Level
	{
		struct Lock
		{
			int x, y, lockLevel;
		};

		struct Trigger
		{
			int x, y, textIndex, soundIndex;
		};

		std::string name, info; // Name of level, and associated INF filename.
		int numf; // Number of floor (and wall?) textures.

		// Various data, not always present. FLOR and MAP1 are probably always present.
		std::vector<uint8_t> flor, loot, map1, map2, targ;
		std::vector<MIFFile::Level::Lock> lock;
		std::vector<MIFFile::Level::Trigger> trig;

		// Primary method for decoding .MIF level tag data. This method calls all the lower-
		// level loading methods for each tag as needed. The return value is the offset from 
		// the current LEVL tag to where the next LEVL tag would be.
		int load(const uint8_t *levelStart);

		// Loading methods for each .MIF level tag (FLOR, MAP1, etc.), called by Level::load(). 
		// The return value is the offset from the current tag to where the next tag would be.
		static int loadFLOR(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadINFO(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadLOCK(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadLOOT(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadMAP1(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadMAP2(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadNAME(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadNUMF(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadTARG(MIFFile::Level &level, const uint8_t *tagStart);
		static int loadTRIG(MIFFile::Level &level, const uint8_t *tagStart);
	};
private:
	int width, depth;
	int startingLevelIndex;
	std::vector<Double2> startPoints; // Entrance locations for the level.
	std::vector<MIFFile::Level> levels;
	// Should a vector of levels be exposed, or does the caller want a nicer format?
	// VoxelGrid? Array of VoxelData?
public:
	MIFFile(const std::string &filename);
	~MIFFile();

	// Identifiers for various chasms in Arena's voxel data.
	static const uint8_t DRY_CHASM;
	static const uint8_t WET_CHASM;
	static const uint8_t LAVA_CHASM;

	// This value is used for transforming .MIF coordinates to voxel coordinates. For example, 
	// if the values in the .MIF files are "centimeters", then dividing by this value converts 
	// them to voxel coordinates (including decimal values; i.e., X=1.5 means the middle of the 
	// voxel at X coordinate 1).
	static const double ARENA_UNITS;

	// Gets the dimensions of the map. They are constant for all levels in a map.
	int getWidth() const;
	int getDepth() const;

	// Gets the starting level when the player enters the area.
	int getStartingLevelIndex() const;

	// Starting points for the player. The .MIF values require a division by 128 in order
	// to become "voxel units" (including the decimal value).
	const std::vector<Double2> &getStartPoints() const;

	// -- temp -- Get the levels associated with the .MIF file (I think we want the data 
	// to be in a nicer format before handing it over to the rest of the program).
	const std::vector<MIFFile::Level> &getLevels() const;

	// To do:
	// - Voxel data per floor (flor, map1, map2), per level.
	// - Name and .INF of a level.
	// - Texture count per level (numf)
	// - Lock locations and lock levels per level.
	// - Trigger locations per level.
	// - Target(?), Loot(loot piles? Quest items?).
};

#endif
