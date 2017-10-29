/*
 * PlayerLevel
 * Typing Genius
 *
 * Created by Aldrich Co on 11/21/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 *	@brief Not the user but merely a level associated with the player.
 *	provides level-scaled values for different circumstances.
 *	GlyphMap (?) may use current user level to determine difficulty through various parameters.
 */


#pragma once
#include <cmath>


namespace ac
{
	class PlayerLevel
	{
	public:

		PlayerLevel(size_t playerLevel = 1) : level(playerLevel) {}

		inline size_t getLevel() const {
			return this->level;
		}
		
		inline void setLevel(size_t level) { this->level = level; }

		inline void incrementLevel() { this->level++; }
		inline void reset() { this->level = 1; }

		/**
		* @brief this provides some hints to the glyph string random generator on how to
		* make the string "less random". The figures in the quintet represent chances of
		* repeating the set of blocks generated immediately before them. The first number
		* indicates the chances of having a single block repeat, the second number the
		* chance in which the sequence of the last two blocks are repeated, and so on. Note
		* the random generator would not be able to do a single repeat other than through the
		* repeat chance given.
		*/
		inline static std::vector<float> glyphRepeatChances(size_t playerLevel)
		{
			switch (playerLevel) {
				case 1: return { 0.3, 0.3, 0.2, 0.2, 0.2 };
				case 2: return { 0.3, 0.2, 0.2, 0.2, 0.2 };
				case 3: return { 0.25, 0.25, 0.25, 0.2, 0.2 };
				case 4: return { 0.25, 0.25, 0.2, 0.2, 0.2 };
				case 5: return { 0.25, 0.2, 0.2, 0.2, 0.2 };
				case 6: return { 0.2, 0.2, 0.2, 0.2, 0.2 };
				case 7: return { 0.2, 0.15, 0.2, 0.2, 0.2 };
				case 8: return { 0.15, 0.15, 0.15, 0.15, 0.15 };
				case 9: return { 0.15, 0.15, 0.1, 0.1, 0.1 };
				case 10: return { 0.15, 0.1, 0.1, 0.1, 0.1 };
				default: return { 0.1, 0.1, 0.1, 0.1, 0.1 };

				// a chance of zero means repeats are impossible!
			}
		}
		

		/** 
		 * @brief The return value is multiplied to block length to give the timer its initial value in a level.
		 */
		inline static float secondsPerBlock(size_t playerLevel)
		{
//#warning DEBUG
//			return 100;

			// 1.2s per block seemed TOO long, and not challenging enough...
			// should only decrease spb once all the possible symbols are in play...
			if (playerLevel < 30) {
				return 1.0f;
			} else if (playerLevel < 50) {
				return 0.8f;
			} else {
				return 0.7f;
			}
		}
		
		inline static float secsToDeductPerMistake(size_t playerLevel)
		{
			return secondsPerBlock(playerLevel) * 1.5;
		}


		/** this is the level progression value of 1 block clear (excluding combos) */
		inline static float levelProgressPerBlock(size_t playerLevel)
		{
//#warning DEBUG
//			return 0.25;

			if (playerLevel < 1) {
				return 0.01f;
			}
			return 1.0f / (20 + 10 * (1 + ((playerLevel - 1) / 3)));
		}


		inline static int pointsPerBlock(size_t playerLevel)
		{
			// 11 at lvl1-3, 22 at lvl4-6, etc
			return floorf(17 * (1 + (playerLevel - 1) / 3));
		}

		
		/** 
		 *	@brief this result is multiplied to levelProgressPerBlock for each block cleared.
		 */
		inline static float progressMultiplierForStreakLevel(size_t curStreak, size_t playerLevel)
		{
			// AC 2013.12.9: for now the current player level doesn't come into play
			switch (curStreak) {
				case 0: return 1.0;
				case 1: return 1.05;
				case 2: return 1.075;
				case 3: return 1.15;
				case 4: return 1.2;
				case 5: return 1.3;
				case 6: return 1.5;
				case 7: return 1.65;
				case 8: return 1.7;
				case 9: default: return 1.75;
			}
		}
		
		
		/**
		 *	@brief the extra bonus percentage points player get above the normal 1x
		 */
		inline static float cumulativeProgressBonusForStreakLevel(size_t curStreak, size_t playerLevel)
		{
			float ret = 0;
			for (size_t i = 0; i < curStreak; i++) {
				float prog = progressMultiplierForStreakLevel(i, playerLevel);
				ret += prog;
			}
			return ret;
		}


		/**
		 *	@brief this controls how easy a combo is.
		 */
		inline static float slideBackTimeForPlayerLevel(size_t playerLevel)
		{
			if (playerLevel < 8) {
				return 0.3;
			} else if (playerLevel < 16) {
				return 0.2;
			} else if (playerLevel < 24) {
				return 0.1;
			} else {
				return 0;
			}
		}


	private:
		size_t level;
	};
}