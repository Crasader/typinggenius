//
//  GameState.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "GameState.h"
#include "StatsHUDModel.h"
#include "StatsHUD.h"
#include "DebugSettingsHelper.h"
#include "CountdownTimer.h"
#include "CopyText.h"
#include "KeypressTracker.h"
#include "ScoreKeeper.h"
#include "Player.h"
#include "GameModifierHelper.h"
#include "GlyphMap.h"

namespace ac {
	
#pragma mark - pImpl
	
	struct GameStateImpl
	{
		GameStateImpl() :
		timerJustStarted(false),
		keypressTracker(),
		scoreKeeper(),
		gameModifierHelper(),
		copyText(),
		glyphMap(),
		player("Player", 1),
		isGodMode(), isGameOver(false),
		isInPostGameState(false),
		timer()
		{
			isGodMode = DebugSettingsHelper::sharedHelper().boolValueForProperty("god_mode", false);
			timer.setExpiryCallbackFunc(boost::bind(&GameStateImpl::countdownExpiryCallback, this, _1));
		}
		
		ac::CountdownTimer timer;

		inline GameState &gs() { return GameState::getInstance(); }
		
		CopyText copyText;
		KeypressTracker keypressTracker;
		ScoreKeeper scoreKeeper;
		Player player;
		GameModifierHelper gameModifierHelper;
		GlyphMap glyphMap;

		bool timerJustStarted; // will be set when starting, and unset after notifying
		bool isInPostGameState;
		bool isGodMode;
		bool isGameOver;

		void processTypingMistake();

		void countdownExpiryCallback(const boost::system::error_code& e);
	};
	
	
#pragma mark - Lifetime
	
	GameState::GameState()
	{
		LogD << "Inside GameState constructor";
		pImpl.reset(new GameStateImpl);
	}
	
	
	GameState::~GameState()
	{
		LogD << "Inside GameState destructor";
	}
	

	GameState& GameState::getInstance()
	{
		static GameState instance;
		return instance;
	}


#pragma mark - App Events

	void GameState::stop(bool finishedStage = false)
	{
		pImpl->timer.reset();
		if (finishedStage) {
			this->player().incrementPlayerLevel();
			LogI << "Incrementing player level. Now at " << this->player().getLevel();
			this->player().addToTotalScore(scoreKeeper().getTotalScore());
			LogI << "New score for the player is: " << this->player().getTotalScore();
			// now 2 or so seconds after this, or after some ideal condition, set "post game state" to true.
			// it should be dependent on StatsHUD announcement.
		}
	}


	void GameState::resume()
	{
		// pImpl->timer.resume(); // have I ever called this?
	}
	
	
#pragma mark - Notif Event Handling and Processing

	void GameState::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		if ("ScoreKeeper_Mistake" == code) {
			pImpl->processTypingMistake();
		}
	}
	
	
	void GameStateImpl::processTypingMistake()
	{
		float secsToDeduct = PlayerLevel::secsToDeductPerMistake(gs().player().getLevel());
		gs().deductTimer(secsToDeduct);
	}
	
	
#pragma mark - Main Time Functions
	
	bool GameState::isGameStarted() const
	{
		return pImpl->timer.running();
	}

	
	long GameState::getTimeRemaining() const
	{
		return pImpl->timer.timeRemaining();
	}


#pragma mark - Game State

	CopyText &GameState::copyText() const
	{
		return pImpl->copyText;
	}


	ScoreKeeper &GameState::scoreKeeper() const
	{
		return pImpl->scoreKeeper;
	}
	
	
	KeypressTracker &GameState::keypressTracker() const
	{
		return pImpl->keypressTracker;
	}


	Player &GameState::player() const
	{
		return pImpl->player;
	}
	
	
	GlyphMap &GameState::glyphMap() const
	{
		return pImpl->glyphMap;
	}

	
	void GameState::resetGameState()
	{
		setPostGameState(false);
		setIsGameOver(false);

		pImpl->copyText.reset();
		pImpl->timer.reset();
		pImpl->keypressTracker.reset();
		pImpl->scoreKeeper.reset();
	}


	void GameState::resetPlayer()
	{
		pImpl->player.resetPlayer();
	}


	bool GameState::isGodMode() const
	{
		return pImpl->isGodMode;
	}


	void GameState::setGodMode(bool godMode)
	{
		pImpl->isGodMode = godMode;
	}


	bool GameState::isInPostGameState() const
	{
		return pImpl->isInPostGameState;
	}


	void GameState::setPostGameState(bool postGameState)
	{
		pImpl->isInPostGameState = postGameState;
	}


	bool GameState::isGameOver() const
	{
		return pImpl->isGameOver;
	}


	void GameState::setIsGameOver(bool gameOver)
	{
		pImpl->isGameOver = gameOver;
	}

	
#pragma mark - Timer Handling
	
	float GameState::tryStartTimer(float seconds)
	{
		pImpl->timerJustStarted = true; // observers will query this
		float timeRemaining = seconds == 0.0 ? this->getTimeRemainingValueForLevel() : seconds;
		pImpl->timer.startCountdown(timeRemaining * 1000, true);

		std::shared_ptr<GameStateTimerEventInfo> pInfo(new GameStateTimerEventInfo);
		pInfo->delta = timeRemaining;
		Notif::send("GameState_Timer_StartTimer", pInfo);

		pImpl->timerJustStarted = false; // observers already notified
		return timeRemaining;
	}
	
	
	bool GameState::checkTimerStopped()
	{
		long timeRemaining = getTimeRemaining();
		if (timeRemaining <= 0) {
			// timer has indeed stopped, send a notification to the observers
			Notif::send("GameState_Timer_StopTimer");
			return true;
		}
		return false;
	}


	void GameStateImpl::countdownExpiryCallback(const boost::system::error_code& e)
	{
		if (e == boost::asio::error::operation_aborted) {
			LogD << ">>> Timer reset!";

		} else {
			LogI << ">>> Timer finished!";
			// should now trigger a signal that SHM should listen for...
			Notif::send("GameState_Timer_StopTimer");
			gs().stop(false);
			gs().setIsGameOver(true); // AC 2013.12.10: based on our rules
			gs().copyText().clearCopyString();
		}
	}
	
	
	bool GameState::hasTimerStateUpdatedToStartIt() const
	{
		return pImpl->timerJustStarted;
	}
	
	
	void GameState::deductTimer(float seconds)
	{
		pImpl->timer.deductTimeFromCountdown(seconds * 1000);

		std::shared_ptr<GameStateTimerEventInfo> pInfo(new GameStateTimerEventInfo);
		pInfo->delta = seconds;
		Notif::send("GameState_Timer_DeductTime", pInfo);
	}
	
	
	void GameState::addTimer(float seconds)
	{
		pImpl->timer.addTimeToCountdown(seconds * 1000);
		std::shared_ptr<GameStateTimerEventInfo> pInfo(new GameStateTimerEventInfo);
		pInfo->delta = seconds;
		Notif::send("GameState_Timer_AddTime", pInfo);
	}
	
	
#pragma mark - pImpl
	
	float GameState::getTimeRemainingValueForLevel() const
	{
		size_t playerLevel = player().getLevel();
		return PlayerLevel::secondsPerBlock(playerLevel) / // copyText().copyString().size();
			PlayerLevel::levelProgressPerBlock(playerLevel);
	}


#pragma mark - Misc

	std::string GameState::formattedTimeVal(long millis, bool includeMinute)
	{
		static boost::format fmtNoMinute("%|1$|.%|2$1d| sec");
		static boost::format fmt("%|1$02|:%|2$02|.%|3$1d|");

		int secondsRaw = millis / 1000;
		int secs = secondsRaw % 60;
		int mins = secondsRaw / 60;

		int frac = (millis % 1000) / 100; // tenths of second
		
		if (mins < 1) {
			includeMinute = false;
		}
		
		if (includeMinute) {
			fmt % mins % secs % frac;
			return fmt.str();
		} else {
			secs += mins * 60;
			fmtNoMinute % secs % frac;
			return fmtNoMinute.str();
		}
	}
}
