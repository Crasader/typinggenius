//
//  StatsHUDModel.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "MVC.h"

namespace ac {
	
	using std::unique_ptr;
	
	class GameState;
	class ScoreKeeper;
	struct StatsHUDModelImpl;
	class BlockCanvasModel;

	struct StatsHUDModelUpdateInfo
	{
		int scoreDelta;
		float levelProgressDelta;
		int curStreakLevel;
		int timerDelta;
	};
	

	class StatsHUDModel : public Model
	{
	public:
		
		StatsHUDModel();
		~StatsHUDModel();

		size_t getNumberOfMistakes() const;
		
		void setNumberOfMistakes(size_t num);
		
		size_t getNumberOfCharsTyped() const;
		
		void setNumberOfCharsTyped(size_t num);
		
		// actual value could come from copytext or something.
		float getTimerLength() const;
		
		void updateProgressMeter();

		// only uses the ScoreKeeper for these
		size_t getScore() const;
		float getAccuracy() const;
		size_t getPlayerLevel() const;
		size_t getCurrencyAmount() const;

		size_t totalBlocksDisplayable() const;
		size_t blocksConsumedSoFar() const;

		void notifCallback(const string &code, std::shared_ptr<void> data);
		
	private:
		unique_ptr<StatsHUDModelImpl> pImpl;
	};
}