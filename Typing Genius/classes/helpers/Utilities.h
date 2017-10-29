//
//  Utilities.h
//  Typing Genius
//
//  Created by Aldrich Co on 8/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "cocos2d.h"
#include <boost/random.hpp>

namespace ac {

	namespace utilities {
		
		using std::string;

		static boost::random::mt19937 rng((const unsigned int)std::time(0));

		struct RGBByte { unsigned char r; unsigned char g; unsigned char b; };


		struct RGBFloat {
			double r;       // percent
			double g;       // percent
			double b;       // percent
		};


		struct HSVFloat {
			double h;       // angle in degrees
			double s;       // percent
			double v;       // percent
		};
		
		
		inline const string getFullPathForFilename(const string &fileName) {
			return cocos2d::CCFileUtils::sharedFileUtils()->fullPathForFilename(fileName.c_str());
		}

		const RGBByte randomRGBByte();
		const RGBByte nextRGBByteHueShift(const RGBByte &rgb, float hueShift);
		
		inline cocos2d::ccColor3B ccc3FromRGB(const RGBByte &rgb)
		{
			return cocos2d::ccc3(rgb.r, rgb.g, rgb.b);
		}


		inline RGBByte rgbByteFromCcc3(const cocos2d::ccColor3B &ccc)
		{
			return { ccc.r, ccc.g, ccc.b };
		}
		
		
		inline std::ostream& operator<<(std::ostream& out, const RGBByte& rgb)
		{
			out << "rgb(" << (int)rgb.r << "," << (int)rgb.g << "," << (int)rgb.b << ")";
			return out;
		}

		inline bool randomChance(float probability)
		{
			static boost::random::uniform_real_distribution<> rRepeatChance(0, 1); // results in a number between [0..1)
			if (rRepeatChance(utilities::rng) < probability) return true;
			return false;
		}


		inline RGBFloat rgbByteToFloat(const RGBByte &rgbByte)
		{
			return { (float)rgbByte.r / 255, (float)rgbByte.g / 255, (float)rgbByte.b / 255 };
		}


		inline RGBByte rgbFloatToByte(const RGBFloat &rgbFloat)
		{
			return {
				(unsigned char) (rgbFloat.r * 255),
				(unsigned char) (rgbFloat.g * 255),
				(unsigned char) (rgbFloat.b * 255)
			};
		}


		HSVFloat rgb2Hsv(const RGBFloat &in);
		RGBFloat hsv2Rgb(const HSVFloat &in);
		
		
		// 0 to max
		const size_t random(size_t max);

		// 0 to 1
		const float randomFloat(float min, float max);

		inline bool keyIsAModifier(const string &keyLabel)
		{
			// keyLabel starts with "mod:" instead of "key:"
			return keyLabel.find("mod:") == 0;
		}
	}
}

