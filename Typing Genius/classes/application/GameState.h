//
//  GameState.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include <boost/date_time.hpp>
#include <boost/system/error_code.hpp>
#include "Notif.h"

namespace ac {
	
	struct GameStateImpl;
	class CopyText;
	class KeypressTracker;
	class ScoreKeeper;
	class Player;
	class GlyphMap;

//	const int AddTimeCurrencyCost = 5;
//	const int SecondsToAddForFrogs = 10;
	
	using namespace boost::posix_time;


	struct GameStateTimerEventInfo
	{
		float delta; // the operation decides what to do with this.
	};

	
	class GameState : public NotifListener
	{
	public:

		static GameState& getInstance();
		~GameState();
		
		// long values produced are in milliseconds.
		long getTimeRemaining() const;
		bool isGameStarted() const;

		// post game state happens for a brief instant after a level completes, and is
		// set off shortly after when the game resumes
		bool isInPostGameState() const;
		void setPostGameState(bool);

		// game over means the player has to restart from level 1
		bool isGameOver() const;
		void setIsGameOver(bool);

		// you'll be left with this.
		float getTimeRemainingValueForLevel() const;

		float tryStartTimer(float seconds = 0.0);
		// controlling timer during app background event
		void deductTimer(float seconds);
		void addTimer(float seconds);
		void stop(bool finishedStage);
		void resume();

		// this doesn't stop the timer, but gives GameState a chance to check if it has.
		bool checkTimerStopped();
		
		static std::string formattedTimeVal(long millis, bool includeMinute = true);
		
		CopyText &copyText() const;
		KeypressTracker &keypressTracker() const;
		ScoreKeeper &scoreKeeper() const;
		Player &player() const;
		GlyphMap &glyphMap() const;

		// observers will query this.
		bool hasTimerStateUpdatedToStartIt() const;
		void resetGameState();
		void resetPlayer();

		bool isGodMode() const;
		void setGodMode(bool);

		static inline long getTimeNow()
		{
			ptime now(microsec_clock::local_time());
			return now.time_of_day().total_milliseconds();
		}


		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);
		
	private:
		GameState(); // use singleton instead
		std::unique_ptr<GameStateImpl> pImpl;
	};
}