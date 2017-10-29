//
//  CountdownTimer.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/10/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "CountdownTimer.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "GameState.h"


namespace ac {

	const long long Million = 1000000LL;

	CountdownTimer::CountdownTimer()
	: countdownTimer(io), originalCountdownDuration(), work(io)
	{
		isStarted = false;

		// 'work' prevents this io service from returning when there's no work to be done
		boost::thread t(&CountdownTimer::startIOService, this);
	}


	CountdownTimer::~CountdownTimer()
	{
		reset();
	}


	void CountdownTimer::reset()
	{
		originalCountdownDuration = 0;
		isStarted = false;
	}


	void CountdownTimer::setExpiryCallbackFunc(TimerCallback_t callback)
	{
		this->callbackFunc = callback;
	}


	bool CountdownTimer::running() const
	{
		return isStarted;
	}


	long CountdownTimer::timeRemaining()
	{
		if (!isStarted) { return 0; }
		boost::posix_time::time_duration duration = countdownTimer.expires_from_now();
		long ms = duration.total_milliseconds();
		return ms;
	}


	void CountdownTimer::startCountdown(unsigned int milliseconds, bool justStarted)
	{
		if (!this->callbackFunc) {
			string errorMessage("no callback specified in CountdownTimer!");
			LogE << errorMessage;
			throw errorMessage;
		}

		if (justStarted) {
			originalCountdownDuration = milliseconds;
			isStarted = true;
			countdownTimer.cancel(); // is this a good idea?
		}

		// i have no idea as of yet why it crashes here.
		countdownTimer.expires_from_now(boost::posix_time::milliseconds(milliseconds));
		countdownTimer.async_wait(this->callbackFunc);

	}


	void CountdownTimer::startIOService()
	{
		io.run();
	}

	
	void CountdownTimer::addTimeToCountdown(unsigned int millis)
	{
		if (!isStarted) { return; }
		long timeRem = timeRemaining();
		timeRem += millis;
		if (timeRem > originalCountdownDuration) {
			timeRem = originalCountdownDuration;
		}
		startCountdown(timeRem);
	}
	
	
	void CountdownTimer::deductTimeFromCountdown(unsigned int millis)
	{
		if (!isStarted) { return; }
		long newTime = timeRemaining() - millis;
		startCountdown(newTime >= 0 ? newTime : 0);
	}

}
