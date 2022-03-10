#pragma once

#include <vector>
#include <string>
#include <array>
#include <cassert>
#include "PvfReader.h"
#include "PvfAnimation.h"
#include "BufferReader.h"
#include "PvfString.h"

PvfAnimation::PvfAnimation(const uint8_t* buffer, int32_t len, PvfReader* reader)
	:buffer(buffer),len(len), reader(reader)
{
	type = PvfScriptType::Animation;
}

auto PvfAnimation::unpack() -> void
{
	BufferReader reader(buffer, len);
	framesCount = reader.read<uint16_t>();
	auto countOfResources = reader.read<uint16_t>();
	frames.resize(framesCount);
	std::vector<std::string> sprites;
	for (auto i = 0; i < countOfResources; i++)
	{
		int32_t len = reader.read<int32_t>();
		sprites.emplace_back(reader.readAsciiString(len));
		PvfString::toLower(sprites.back());
	}

	auto params = reader.read<uint16_t>();
	for (auto j = 0; j < params; j++)
	{
		auto type = reader.read<uint16_t>();
		switch (type)
		{
		case AnimationNodeType::LOOP:
			loop = reader.read<int8_t>();
			break;

		case AnimationNodeType::SHADOW:
			shadow = reader.read<int8_t>();
			break;
		}
	}

	for (auto i = 0; i < framesCount; i++)
	{
		auto& frame = frames[i];

		auto boxes = reader.read<uint16_t>();
		for (auto j = 0; j < boxes; j++)
		{
			auto type = reader.read<uint16_t>();
			assert(type == DAMAGE_BOX || type == ATTACK_BOX);
			auto& box = type == DAMAGE_BOX ? frame.damageBox.emplace_back() : frame.attackBox.emplace_back();
			for (int32_t m = 0; m < 6; m++)
			{
				box[m] = reader.read<int32_t>();
			}
		}
		frame.imgId = reader.read<uint16_t>();
		frame.imgParam = reader.read<uint16_t>();
		frame.path = sprites[frame.imgId];
		assert(frame.imgId >= 0 && frame.imgId < sprites.size());

		frame.x = reader.read<int32_t>();
		frame.y = reader.read<int32_t>();

		int32_t propertyCount = reader.read<uint16_t>();
		for (int32_t m = 0; m < propertyCount; m++)
		{
			AnimationNodeType type = (AnimationNodeType)reader.read<uint16_t>();
			switch (type) {
			case LOOP:
				frame.loop = reader.read<int8_t>();
				break;
			case SHADOW:
				frame.shadow = reader.read<int8_t>();
				break;
			case INTERPOLATION:
				frame.interpolation = reader.read<int8_t>();
				break;
			case 2:
			case 4:
			case 5:
			case 6:
			case DAMAGE_BOX:
			case ATTACK_BOX:
			case SPECTRUM://18
			case 19://18
			case 20:
			case 21:
			case 22:
				break;
			case Ani_COORD:
				frame.coord = reader.read<uint16_t>();
				break;
			case IMAGE_RATE:
				frame.rateX = reader.read<float>();
				frame.rateY = reader.read<float>();
				break;
			case IMAGE_ROTATE:
				frame.rotate = reader.read<int32_t>();
				break;
			case RGBA:
				frame.color = reader.read<uint32_t>();
				break;
			case GRAPHIC_EFFECT:
				frame.itemType = (EffectItem)reader.read<uint16_t>();
				if (frame.itemType == EffectItem::MONOCHROME)
				{
					frame.effectItem.effectColor.r = reader.read<uint8_t>();
					frame.effectItem.effectColor.g = reader.read<uint8_t>();
					frame.effectItem.effectColor.b = reader.read<uint8_t>();
				}
				else if (frame.itemType == SPACEDISTORT)
				{
					frame.effectItem.pos.x = reader.read<uint16_t>();
					frame.effectItem.pos.y = reader.read<uint16_t>();
				}
				break;
			case DELAY://12
				frame.delay = reader.read<int32_t>();
				break;
			case DAMAGE_TYPE:
				frame.damageType = (DamageType)reader.read<uint16_t>();
				break;
			case PLAY_SOUND:
				frame.sound = reader.readAsciiString(reader.read<int32_t>());
				break;
			case PRELOAD:
				break;
			case SET_FLAG:
				frame.setFlag = reader.read<int32_t>();
				break;
			case FLIP_TYPE:
				frame.flipType = (FlipType)reader.read<uint16_t>();
				break;
			case LOOP_START:
				frame.loopStart = true;
				break;
			case LOOP_END:
				frame.loopEnd = reader.read<int32_t>();
				break;
			case CLIP:
				frame.clip[0] = reader.read<int16_t>();
				frame.clip[1] = reader.read<int16_t>();
				frame.clip[2] = reader.read<int16_t>();
				frame.clip[3] = reader.read<int16_t>();
				break;
			default:
				break;
			}
		}
	}
}
