/*
 * Player
 * Typing Genius
 *
 * Created by Aldrich Co on 11/21/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 * @brief A player, owned by GameState
 * 
 */

#pragma once

#include "PlayerLevel.h"

namespace ac
{
	struct PlayerImpl;
	class ScoreKeeper;

	class Player
	{
	public:
		Player(const std::string& name, const size_t level = 1);
		~Player();

		const std::string &getName() const;

		size_t getLevel() const;
		void setLevel(size_t level);
		void incrementPlayerLevel();

		size_t getTotalScore() const;
		void addToTotalScore(size_t scoreToBeAdded);

		size_t getCurrencyAmount() const;
		void addToCurrencyOwned(size_t amount);
		void deductCurrencyOwned(size_t amount);

		void resetPlayer();

		/** 
		 *	@brief saves updated values to corresponding db entry. Can throw!
		 */
		void syncStatsToDB(const ScoreKeeper &);
		
		
	private:
		std::unique_ptr<PlayerImpl> pImpl;
	};


	inline std::ostream& operator<<(std::ostream& out, const Player& player)
	{
		// cast to void to get rid of the "expression unused" warning
		(void) (out << "{ " << player.getName() << ": Level " << player.getLevel()
				<< " score: " << player.getTotalScore() << " }");
		return out;
	}

}