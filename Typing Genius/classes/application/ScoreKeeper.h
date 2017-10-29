//
//  Scorekeeper.h
//  Typing Genius
//
//  Created by Aldrich Co on 11/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	This is a dependent class of GameState. Also handles streaks, accuracies, and progress.
//	Client typically calls one of the record*** functions during the course of the game.
//	It would trigger signals that StatsHUD might hook up into...

#pragma once
#include <cmath>

namespace ac {

	struct SessionScore
	{
		int score; // by end of a session by end of a round / level
		int bonusForAccuracy;
		int bonusForTimeRemaining;

		inline int totalScore() const { return score + bonusForAccuracy + bonusForTimeRemaining; }
	};
	
	// this comes with every scorekeeper signal update
	struct ScoreKeeperUpdateInfo
	{
		int scoreDelta; // the Update Type "Score" has it, along with a few others.
		float levelProgressDelta;
		int curStreakLevel;
	};
	

	class ScoreKeeper
	{
	public:
		
		ScoreKeeper();
		~ScoreKeeper();
		
		void reset(); // resets all.
		
		// - - - - - - - - - - - - - - - - - - - - -
		// The Statics
		// note: prefer prime numbers
		
		static int bonusForSuccessfulBlockClear(size_t units);

		inline static int bonusForMistakenAttemptedBlockClear(size_t units) { return units * -7; }
		inline static int bonusForTimeRemaining(float secsLeft) { return (int)secsLeft * 19; }
		inline static int bonusForActiveStreak(int curStreak) { return (int) powf(fmax(0, curStreak - 1), 1.5) * 5; }
		inline static int bonusForAccuracy(float accuracy, size_t total) {
			if (accuracy > 0.95) // v. high accuracy;
				return (int) (total * 10);
			else if (accuracy > 0.9) // high accuracy;
				return (int) (total * 8);
			else if (accuracy > 0.8) // somewhat high accuracy
				return (int) (total * 6);
			else if (accuracy > 0.7)
				return (int) (total * 4);
			else if (accuracy > 0.6)
				return (int) (total * 2);
			else {
				return 0;
			}
		}

		// Important: internal streak counter should be updated in a separate call
		void recordBlockClear(size_t units);
		void recordMistakenAttempt(size_t units);
		
		// score
		inline size_t getTotalScore() const {
			return (size_t)
				this->sessionScore.score +
				this->sessionScore.bonusForAccuracy +
				this->sessionScore.bonusForTimeRemaining	;
		}

		inline const SessionScore &getSessionScore() const { return this->sessionScore; }

		// (normally you don't use these two, not yet anyway)
		// inline void addScore(int score) { this->score += score; }
		// inline void setScore(size_t score) { this->score = score; }
		
		// streaks
		inline void recordStreakIncrement(size_t streak = 1) { incrementCurrentStreak(streak); }
		inline void recordStreakReset() { resetCurrentStreak(); }
		inline size_t getCurrentStreak() const { return (size_t) this->curStreak; }
		inline size_t getLongestStreak() const { return (size_t) this->longestStreak; }
		void resetCurrentStreak();
		inline void incrementCurrentStreak(size_t streak = 1) {
			this->curStreak += streak;
			this->longestStreak = fmax(this->longestStreak, this->curStreak);
		}
		
		// time remaining (be sure to call this before getting final score)
		void recordPostSessionTimeRemaining(size_t seconds);
		void recordPostSessionAccuracy();

		// accuracy: 0-1 value
		// note: there should be an accuracy bonus
		// I can expose the mistakeCount / correctCount variables if necessary.
		float getAccuracy() const;

		// progress (could query from CopyText, which should know everything about it)
		// this refers to the progress of copy text. Progress to leveling up is levelProgress.
		float getProgress() const;

		// level progression
		inline float getLevelProgress() const { return this->levelProgress; }
		void addToLevelProgress(float progress);
		void setLevelProgress(float progress);

		// stats
		inline size_t getMistakeCount() const { return this->mistakeCount; }
		inline size_t getCorrectCount() const { return this->correctCount; }
		

	private:

		int curStreak;
		size_t mistakeCount; // used for accuracy
		size_t correctCount;

		size_t longestStreak;

		float levelProgress; // 0 - 1
		SessionScore sessionScore;
	};
}