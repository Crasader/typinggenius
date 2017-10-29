//
//  Scorekeeper.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "ScoreKeeper.h"
#include "CopyText.h"
#include "GameState.h"
#include "PlayerLevel.h"
#include "Player.h"


namespace ac {
	
	ScoreKeeper::ScoreKeeper() : curStreak(0), correctCount(0), mistakeCount(0), levelProgress(0)
	{
	}
	
	
	ScoreKeeper::~ScoreKeeper() {}
	
	
	void ScoreKeeper::reset()
	{
		this->sessionScore.score = 0;
		this->sessionScore.bonusForAccuracy = 0;
		this->sessionScore.bonusForTimeRemaining = 0;
		this->levelProgress = 0;
		
		this->mistakeCount = 0;
		this->correctCount = 0;
		this->curStreak = 0;
	}
	

#pragma mark - RecordXXX functions
	
	void ScoreKeeper::recordBlockClear(size_t units = 1)
	{
		int addedPoints = bonusForSuccessfulBlockClear(units) +
			bonusForActiveStreak(this->curStreak);

		LogI << "Adding " << addedPoints << " points";

		this->sessionScore.score += addedPoints;
		this->correctCount += units;
		
		const size_t playerLevel(GameState::getInstance().player().getLevel());
		
		float levelProgressPerBlock = units *
			PlayerLevel::levelProgressPerBlock(playerLevel) *
			PlayerLevel::progressMultiplierForStreakLevel(this->curStreak, playerLevel);
		
		LogD << "levelProgressPerBlock added: " << levelProgressPerBlock;
		
		this->addToLevelProgress(levelProgressPerBlock); //  for now

		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->scoreDelta = addedPoints;
		pInfo->curStreakLevel = this->curStreak;
		Notif::send("ScoreKeeper_Score", pInfo);
	}
	
	
	void ScoreKeeper::recordMistakenAttempt(size_t units = 1)
	{
		this->mistakeCount += units;
		Notif::send("ScoreKeeper_Mistake");
	}
	
	
	void ScoreKeeper::recordPostSessionTimeRemaining(size_t seconds)
	{
		int bonus = bonusForTimeRemaining(seconds);
		this->sessionScore.bonusForTimeRemaining += bonus;

		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->scoreDelta = bonus;
		Notif::send("ScoreKeeper_PostGameTimeRemainingBonus", pInfo);
	}


	void ScoreKeeper::recordPostSessionAccuracy()
	{
		int bonus = bonusForAccuracy(getAccuracy(), this->correctCount);
		this->sessionScore.bonusForAccuracy += bonus;

		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->scoreDelta = bonus;
		Notif::send("ScoreKeeper_PostGameAccuracyBonus", pInfo);
	}
	
	
	void ScoreKeeper::resetCurrentStreak()
	{
		// last chance to inform followers. If leveling up, curStreak will reset to zero without notifying
		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->curStreakLevel = this->curStreak;
		Notif::send("ScoreKeeper_StreakFinished", pInfo);

		this->curStreak = 0;
	}


#pragma mark - Level Progression

	void ScoreKeeper::addToLevelProgress(float progress) {
		progress = MAX(0, MIN(progress, 1));
		this->levelProgress += progress;
		
		// leveled up.
		if (this->levelProgress >= 1.0f) {
			this->levelProgress = 0.0f;
			GameState::getInstance().player().incrementPlayerLevel();
			this->curStreak = 0;
			GameState::getInstance().copyText().reComposeCopyText(GameState::getInstance().player().getLevel());

			std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
			pInfo->levelProgressDelta = progress;
			Notif::send("ScoreKeeper_NewLevelUpdate", pInfo);

			GameState::getInstance().player().syncStatsToDB(*this);
			LogI << "Level progress at 100%";
		}

		// LevelProgressUpdate
		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->levelProgressDelta = progress;
		Notif::send("ScoreKeeper_LevelProgressUpdate", pInfo);
	}


	void ScoreKeeper::setLevelProgress(float progress) {
		progress = MAX(0, MIN(progress, 1));
		this->levelProgress = progress;
		LogI << "Setting level progress to " << progress;
		if (this->levelProgress >= 1.0f) {
			LogI << "Level progress at 100%";
		}

		std::shared_ptr<ScoreKeeperUpdateInfo> pInfo(new ScoreKeeperUpdateInfo);
		pInfo->levelProgressDelta = progress;
		Notif::send("ScoreKeeper_LevelProgressUpdate", pInfo);
	}


#pragma mark -

	int ScoreKeeper::bonusForSuccessfulBlockClear(size_t units)
	{
		size_t level = GameState::getInstance().player().getLevel();
		return units * PlayerLevel::pointsPerBlock(level);
	}


	float ScoreKeeper::getAccuracy() const
	{
		if (this->correctCount + this->mistakeCount < 1) {
			// avoid a division by zero
			return 0.0f;
		}
		// AC 2013.11.20: I'm not really sure if this is the best calculation
		return (float) this->correctCount / (this->correctCount + this->mistakeCount);
	}


	float ScoreKeeper::getProgress() const
	{
		return GameState::getInstance().copyText().getProgress();
	}

}
