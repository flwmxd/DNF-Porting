#pragma once
#include <vector>
#include <string>
#include <array>

#include "PvfScript.h"

class PvfReader;

enum AnimationNodeType
{
	LOOP = 0,
	SHADOW = 1,
	Ani_COORD = 3,
	IMAGE_RATE = 7,
	IMAGE_ROTATE,
	RGBA,
	INTERPOLATION = 10,
	GRAPHIC_EFFECT,
	DELAY,
	DAMAGE_TYPE,
	DAMAGE_BOX,
	ATTACK_BOX,
	PLAY_SOUND,
	PRELOAD,
	SPECTRUM,
	SET_FLAG = 23,
	FLIP_TYPE,
	LOOP_START,
	LOOP_END,
	CLIP,
	OPERATION,
	LENGTH
};

enum EffectItem
{
	NONE,
	DODGE,
	LINEARDODGE,
	DARK,
	XOR,
	MONOCHROME,
	SPACEDISTORT//¿Õ¼ä´íÂÒ
};

enum FlipType
{
	HORIZON = 1,
	VERTICAL,
	ALL
};

enum DamageType
{
	NORMAL,
	SUPERARMOR,
	UNBREAKABLE
};

class PvfAnimation : public PvfScript
{
public:
	PvfAnimation(const uint8_t* buffer, int32_t len,PvfReader * reader);
	auto unpack() -> void override;
	inline auto& getFrames() const { return frames; }
	inline auto isLoop() const { return loop; }
private:

	union NodeData
	{
		int32_t intValue;
		bool flag;
	};

	struct PvfFrame 
	{
		int32_t x;
		int32_t y;
		int32_t imgId;//link to sprites
		std::string path;
		uint16_t imgParam;//unknown what is it
	
		float rateX;//?? scale in here?
		float rateY;
		float rotate;
		uint32_t color = UINT32_MAX;
		EffectItem itemType;
		int32_t delay;
		DamageType damageType;
		std::string sound;
		int32_t setFlag;
		FlipType flipType;
		bool loopStart = false;
		bool shadow = false;
		bool interpolation = false;
		int32_t loopEnd;
		int16_t clip[4];
		int32_t coord;
		union 
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
			}effectColor;

			struct
			{
				uint32_t x;
				uint32_t y;
			}pos;

		}effectItem;

		std::vector<std::array<int32_t, 6>> damageBox;
		std::vector<std::array<int32_t, 6>> attackBox;
		bool loop = false;
	};


	std::vector<PvfFrame> frames;
	int32_t framesCount = 0;
	int32_t len = 0;
	bool loop = false;

	bool shadow = false;
	const uint8_t* buffer = nullptr;
	PvfReader* reader = nullptr;
};
