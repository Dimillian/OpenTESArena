#include <cassert>

#include "WorldData.h"

#include "../Assets/INFFile.h"
#include "../Assets/MIFFile.h"
#include "../Media/TextureManager.h"
#include "../Rendering/Renderer.h"
#include "../Utilities/Bytes.h"

WorldData::TextTrigger::TextTrigger(const std::string &text, bool displayedOnce)
	: text(text)
{
	this->displayedOnce = displayedOnce;
	this->previouslyDisplayed = false;
}

WorldData::TextTrigger::~TextTrigger()
{

}

const std::string &WorldData::TextTrigger::getText() const
{
	return this->text;
}

bool WorldData::TextTrigger::isSingleDisplay() const
{
	return this->displayedOnce;
}

bool WorldData::TextTrigger::hasBeenDisplayed() const
{
	return this->previouslyDisplayed;
}

void WorldData::TextTrigger::setPreviouslyDisplayed(bool previouslyDisplayed)
{
	this->previouslyDisplayed = previouslyDisplayed;
}

WorldData::WorldData(const MIFFile &mif, const INFFile &inf)
	: voxelGrid(mif.getWidth(), 5, mif.getDepth()) // To do: eventually get height from .MIF file.
{
	// Arena's level origins start at the top-right corner of the map, so X increases 
	// going to the left, and Z increases going down. The wilderness uses this same 
	// pattern. Each chunk looks like this:
	// +++++++ <- Origin (0, 0)
	// +++++++
	// +++++++
	// +++++++
	// ^
	// |
	// Max (mapWidth - 1, mapDepth - 1)

	// Empty voxel data (for air).
	const int emptyID = voxelGrid.addVoxelData(VoxelData(0));

	// Lambda for setting a voxel at some coordinate to some ID.
	auto setVoxel = [this](int x, int y, int z, int id)
	{
		char *voxels = this->voxelGrid.getVoxels();
		voxels[x + (y * this->voxelGrid.getWidth()) +
			(z * this->voxelGrid.getWidth() * this->voxelGrid.getHeight())] = id;
	};

	// Load first level in .MIF file.
	const MIFFile::Level &level = mif.getLevels().front();
	const uint8_t *florData = level.flor.data();
	const uint8_t *map1Data = level.map1.data();

	// Indices for voxel data, stepping two bytes at a time.
	int floorIndex = 0;
	int map1Index = 0;

	auto getNextFloorVoxel = [florData, &floorIndex]()
	{
		const uint16_t voxel = Bytes::getLE16(florData + floorIndex);
		floorIndex += 2;
		return voxel;
	};

	auto getNextMap1Voxel = [map1Data, &map1Index]()
	{
		const uint16_t voxel = Bytes::getLE16(map1Data + map1Index);
		map1Index += 2;
		return voxel;
	};

	// Mappings of floor and wall IDs to voxel data indices.
	std::unordered_map<uint16_t, int> floorDataMappings, wallDataMappings;

	// Write the .MIF file's voxel IDs into the voxel grid.
	for (int x = (mif.getWidth() - 1); x >= 0; x--)
	{
		for (int z = (mif.getDepth() - 1); z >= 0; z--)
		{
			const int index = x + (z * mif.getWidth());
			const uint16_t florVoxel = getNextFloorVoxel();
			const uint16_t map1Voxel = getNextMap1Voxel();

			// The floor voxel has a texture if it's not a chasm.
			const int floorTextureID = (florVoxel & 0xFF00) >> 8;
			if ((floorTextureID != MIFFile::DRY_CHASM) &&
				(floorTextureID != MIFFile::WET_CHASM) &&
				(floorTextureID != MIFFile::LAVA_CHASM))
			{
				// Get the voxel data index associated with the floor value, or add it
				// if it doesn't exist yet.
				const int dataIndex = [this, &floorDataMappings, florVoxel, floorTextureID]()
				{
					const auto floorIter = floorDataMappings.find(florVoxel);
					if (floorIter != floorDataMappings.end())
					{
						return floorIter->second;
					}
					else
					{
						// To do: Also assign some "seawall" texture for interiors and exteriors.
						// Retrieve it beforehand from the *...CHASM members and assign here?
						// Not sure how that works.
						const int index = this->voxelGrid.addVoxelData(VoxelData(floorTextureID));
						return floorDataMappings.insert(
							std::make_pair(florVoxel, index)).first->second;
					}
				}();

				setVoxel(x, 0, z, dataIndex);
			}

			if ((map1Voxel & 0x8000) == 0)
			{
				// A voxel of some kind.
				const bool voxelIsEmpty = map1Voxel == 0;

				if (!voxelIsEmpty)
				{
					const uint8_t mostSigByte = (map1Voxel & 0x7F00) >> 8;
					const uint8_t leastSigByte = map1Voxel & 0x007F;
					const bool voxelIsSolid = mostSigByte == leastSigByte;

					if (voxelIsSolid)
					{
						// Regular 1x1x1 wall.
						const int wallTextureID = mostSigByte;

						// Get the voxel data index associated with the wall value, or add it
						// if it doesn't exist yet.
						const int dataIndex = [this, &inf, &wallDataMappings,
							map1Voxel, wallTextureID]()
						{
							const auto wallIter = wallDataMappings.find(map1Voxel);
							if (wallIter != wallDataMappings.end())
							{
								return wallIter->second;
							}
							else
							{
								const double ceilingHeight = 
									static_cast<double>(inf.getCeiling().height) / 
									static_cast<double>(MIFFile::ARENA_UNITS);

								const int index = this->voxelGrid.addVoxelData(VoxelData(
									wallTextureID, wallTextureID, wallTextureID, 0.0,
									ceilingHeight, 0.0, 1.0));
								return wallDataMappings.insert(
									std::make_pair(map1Voxel, index)).first->second;
							}
						}();

						setVoxel(x, 1, z, dataIndex);
					}
					else
					{
						// Raised platform. The height appears to be some fraction of 64,
						// and when it's greater than 64, then that determines the offset?
						const uint8_t capTextureID = (map1Voxel & 0x00F0) >> 4; // To do: get BOXCAP Y texture.
						const uint8_t wallTextureID = map1Voxel & 0x000F; // To do: get BOXSIDE Z texture.
						const double platformHeight = static_cast<double>(mostSigByte) / 
							static_cast<double>(MIFFile::ARENA_UNITS);

						// Get the voxel data index associated with the wall value, or add it
						// if it doesn't exist yet.
						const int dataIndex = [this, &wallDataMappings, map1Voxel,
							capTextureID, wallTextureID, platformHeight]()
						{
							const auto wallIter = wallDataMappings.find(map1Voxel);
							if (wallIter != wallDataMappings.end())
							{
								return wallIter->second;
							}
							else
							{
								// To do: Clamp top V coordinate positive until the correct platform 
								// height calculation is figured out. Maybe the platform height
								// needs to be multiplied by the ratio between the current ceiling
								// height and the default ceiling height (128)? I.e., multiply by
								// the "ceilingHeight" local variable used a couple dozen lines up?
								const double topV = std::max(0.0, 1.0 - platformHeight);
								const double bottomV = 1.0; // To do: should also be a function.

								const int index = this->voxelGrid.addVoxelData(VoxelData(
									wallTextureID, capTextureID, capTextureID,
									0.0, platformHeight, topV, bottomV));
								return wallDataMappings.insert(
									std::make_pair(map1Voxel, index)).first->second;
							}
						}();

						setVoxel(x, 1, z, dataIndex);
					}
				}
			}
			else
			{
				// An object of some kind.
			}
		}
	}

	// Assign text and sound triggers.
	for (const auto &trigger : level.trig)
	{
		// Transform the voxel coordinates from the Arena layout to the new layout.
		// - For some reason, the grid dimensions have a minus one here, whereas
		//   the dimensions for player starting points do not.
		const Int2 voxel = VoxelGrid::arenaVoxelToNewVoxel(
			Int2(trigger.x, trigger.y), mif.getWidth() - 1, mif.getDepth() - 1);

		// There can be a text trigger and sound trigger in the same voxel.
		const bool isTextTrigger = trigger.textIndex != -1;
		const bool isSoundTrigger = trigger.soundIndex != -1;

		// Make sure the text index points to a text value (i.e., not a key or riddle).
		if (isTextTrigger && inf.hasTextIndex(trigger.textIndex))
		{
			const INFFile::TextData &textData = inf.getText(trigger.textIndex);
			this->textTriggers.insert(std::make_pair(
				voxel, TextTrigger(textData.text, textData.displayedOnce)));
		}

		if (isSoundTrigger)
		{
			this->soundTriggers.insert(std::make_pair(voxel, inf.getSound(trigger.soundIndex)));
		}
	}
}

WorldData::WorldData(VoxelGrid &&voxelGrid, EntityManager &&entityManager)
	: voxelGrid(std::move(voxelGrid)), entityManager(std::move(entityManager)) { }

WorldData::~WorldData()
{

}

VoxelGrid &WorldData::getVoxelGrid()
{
	return this->voxelGrid;
}

const VoxelGrid &WorldData::getVoxelGrid() const
{
	return this->voxelGrid;
}

EntityManager &WorldData::getEntityManager()
{
	return this->entityManager;
}

const EntityManager &WorldData::getEntityManager() const
{
	return this->entityManager;
}

WorldData::TextTrigger *WorldData::getTextTrigger(const Int2 &voxel)
{
	const auto textIter = this->textTriggers.find(voxel);
	return (textIter != this->textTriggers.end()) ? (&textIter->second) : nullptr;
}

const std::string *WorldData::getSoundTrigger(const Int2 &voxel) const
{
	const auto soundIter = this->soundTriggers.find(voxel);
	return (soundIter != this->soundTriggers.end()) ? (&soundIter->second) : nullptr;
}
