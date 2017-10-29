//
//  StatsHUDView.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include "MVC.h"
#include "cocos2d.h"

namespace ac {
	
	USING_NS_CC;
	using std::unique_ptr;
	
	class StatsHUDModel;

	enum class TimerResetAnimationType
	{
		NoAnimation,
		Increase,
		Decrease
	};
	
	struct StatsHUDViewImpl;

	class StatsHUDView : public View, public CCLayer
	{
	public:
		
		StatsHUDView();
		~StatsHUDView();
		
		CREATE_FUNC(StatsHUDView);
		bool init();
		
		virtual void onEnter();
		virtual void onExit();

		void setCharsTypedCount(size_t typedCharCount);
		void setAccuracyValue(size_t typedCharCount, size_t mistakesCount);
		
		CCSize progressBlockSize() const;
		CCPoint progressPositionAtIndex(size_t idx) const;
		
		// timer
		// stats model should provide the event to start the timer?
		void startTimerWithCountdown();
		void timerExpiredCallback();
		
		// timer callback
		void timerUpdate(float delta);
		
		void startTimerCountdown(float seconds, float startPercent);
		void resetTimeInCountdown(float seconds, const TimerResetAnimationType &animationType =
								  TimerResetAnimationType::NoAnimation); // live change to active timer.

		void stopTimerCountdown();
		
		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);

	private:
		unique_ptr<StatsHUDViewImpl> pImpl;
	};
	
	
	}