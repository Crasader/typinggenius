//
//  StatsHUDModel.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "StatsHUDModel.h"
#include "GameState.h"
#include "CopyText.h"
#include "ScoreKeeper.h"
#include "Player.h"
#include "BlockCanvasModel.h"


namespace ac {
	
#pragma mark - pImpl Definition
	
	struct StatsHUDModelImpl
	{
		StatsHUDModelImpl() :
		numberOfMistakes(0), numberOfCharsTyped(0)
		{
		}
		
		size_t numberOfMistakes;
		size_t numberOfCharsTyped;

		float timeRemaining;
	};
	
#pragma mark - Lifetime
	
	StatsHUDModel::StatsHUDModel()
	{
		LogI << "Inside StatsHUDModel constructor";
		pImpl.reset(new StatsHUDModelImpl);
	}
	
	
	StatsHUDModel::~StatsHUDModel()
	{
		LogI << "Inside StatsHUDModel destructor";
	}

#pragma mark - Getters & Setters
	
	size_t StatsHUDModel::getNumberOfMistakes() const
	{
		return pImpl->numberOfMistakes;
	}
	
	
	void StatsHUDModel::setNumberOfMistakes(size_t num)
	{
		pImpl->numberOfMistakes = num;
		Notif::send("StatsHUDModel_SetNumberOfMistakes"); // unused
	}
	
	
	size_t StatsHUDModel::getNumberOfCharsTyped() const
	{
		return pImpl->numberOfCharsTyped;
	}
	
	
	void StatsHUDModel::setNumberOfCharsTyped(size_t num)
	{
		pImpl->numberOfCharsTyped = num;
		Notif::send("StatsHUDModel_SetNumberOfCharsTyped"); // unused
	}


	size_t StatsHUDModel::getScore() const
	{
		// score for this round
		size_t scoreForThisStage = GameState::getInstance().scoreKeeper().getTotalScore();
		size_t totalSavedScore = GameState::getInstance().player().getTotalScore();
		return totalSavedScore + scoreForThisStage;
	}


	float StatsHUDModel::getAccuracy() const
	{
		return GameState::getInstance().scoreKeeper().getAccuracy();
	}


	size_t StatsHUDModel::getPlayerLevel() const
	{
		return GameState::getInstance().player().getLevel();
	}


	size_t StatsHUDModel::getCurrencyAmount() const
	{
		return GameState::getInstance().player().getCurrencyAmount();
	}

	
#pragma mark - Progress Bar and Timer

	float StatsHUDModel::getTimerLength() const
	{
		return GameState::getInstance().getTimeRemainingValueForLevel();
	}


	void StatsHUDModel::updateProgressMeter()
	{
		// meant for the SHView.
		Notif::send("StatsHUDModel_UpdateProgress");
	}
	
	
	size_t StatsHUDModel::totalBlocksDisplayable() const
	{
		return GameState::getInstance().copyText().copyString().size();
	}
	
	
	size_t StatsHUDModel::blocksConsumedSoFar() const
	{
		return GameState::getInstance().copyText().curOffset();
	}


#pragma mark - NotifListener callback
	
	void StatsHUDModel::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		if ("StatsHUDView_TimerExpired" == code) {
			LogI << "SHM: timer expired! (SHV)";
			// most of the work is done already by GS.
		} else if ("StatsHUDView_PostGameSubheadlineShown" == code) {
			GameState::getInstance().setPostGameState(true);
		}

		// CopyText
		else if ("CopyText_FirstPress" == code) { // still need this? maybe, later.
			// start the timer.
			GameState::getInstance().tryStartTimer(getTimerLength());
			Notif::send("StatsHUDModel_StartTimer");

		} else if ("CopyText_AllCleared" == code) {
			Notif::send("StatsHUDModel_AllCleared");

		} else if ("CopyText_Blocked" == code) {
			Notif::send("StatsHUDModel_Blocked");
		}

		// BlockCanvasModel
		else if ("BlockCanvasModel_DoneAnimation" == code) {
			updateProgressMeter();
		} else if ("BlockCanvasModel_CurrencyCollected" == code) {
			Notif::send("StatsHUDModel_CurrencyCollected"); // pass this on to the view.
		}


		else if ("MainLayer_AddTime" == code) {
			Notif::send("StatsHUDModel_CurrencyConsumed");
		}

		// ScoreKeeper
		else if ("ScoreKeeper_LevelProgressUpdate" == code) {
			std::shared_ptr<ScoreKeeperUpdateInfo> updateInfo = std::static_pointer_cast<ScoreKeeperUpdateInfo>(data);
			std::shared_ptr<StatsHUDModelUpdateInfo> pInfo(new StatsHUDModelUpdateInfo);
			pInfo->levelProgressDelta = updateInfo->levelProgressDelta;
			Notif::send("StatsHUDModel_LevelProgressUpdate", pInfo);

		} else if ("ScoreKeeper_NewLevelUpdate" == code) {
			GameState::getInstance().stop(false);
			GameState::getInstance().tryStartTimer(getTimerLength());
			Notif::send("StatsHUDModel_NewLevel");

		} else if ("ScoreKeeper_Mistake" == code) {
			Notif::send("StatsHUDModel_Mistake");

		} else if ("ScoreKeeper_Score" == code) {
			std::shared_ptr<ScoreKeeperUpdateInfo> updateInfo = std::static_pointer_cast<ScoreKeeperUpdateInfo>(data);
			std::shared_ptr<StatsHUDModelUpdateInfo> pInfo(new StatsHUDModelUpdateInfo);
			pInfo->scoreDelta = updateInfo->scoreDelta;
			pInfo->curStreakLevel = updateInfo->curStreakLevel;
			Notif::send("StatsHUDModel_ScoreOrAccuracyUpdate", pInfo);

		} else if ("ScoreKeeper_StreakFinished" == code) {
			std::shared_ptr<ScoreKeeperUpdateInfo> updateInfo = std::static_pointer_cast<ScoreKeeperUpdateInfo>(data);
			std::shared_ptr<StatsHUDModelUpdateInfo> pInfo(new StatsHUDModelUpdateInfo);
			pInfo->curStreakLevel = updateInfo->curStreakLevel;
			Notif::send("StatsHUDModel_StreakFinished", pInfo);

		}

		// GameState (Timer)
		else if ("GameState_Timer_StartTimer" == code) {
			// AC 2014.1.3: not sure what this condition means any more.
			if (GameState::getInstance().hasTimerStateUpdatedToStartIt()) {
				// should trigger its own signal that the stats view should be
				// a slot to the view will query the GameState directly for the values.
				Notif::send("StatsHUDModel_StartTimer");
			}

		} else if ("GameState_Timer_StopTimer" == code) {
			LogI << "SHM: timer expired!!! (GS)";
			Notif::send("StatsHUDModel_EndTimer");

		} else if ("GameState_Timer_AddTime" == code) {
			LogI << "now: add the time.";
			std::shared_ptr<GameStateTimerEventInfo> info = std::static_pointer_cast<GameStateTimerEventInfo>(data);
			std::shared_ptr<StatsHUDModelUpdateInfo> pInfo(new StatsHUDModelUpdateInfo);
			pInfo->timerDelta = info->delta;
			Notif::send("StatsHUDModel_AddTime", pInfo);

		}

	}
}

