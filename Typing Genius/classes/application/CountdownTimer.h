//
//  CountdownTimer.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/10/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

// #include <boost/timer/timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread.hpp>

namespace ac {


	typedef boost::function<void(const boost::system::error_code& e)> TimerCallback_t;

	class CountdownTimer
	{
	public:

		CountdownTimer();
		~CountdownTimer();

		void reset();

		bool running() const;

		void startCountdown(unsigned int milliseconds, bool justStarted = false);
		
		// assumes countdown is running.
		void deductTimeFromCountdown(unsigned int);
		void addTimeToCountdown(unsigned int);

		// useful only when timer isn't paused; expressed in milliseconds
		long timeRemaining();

		// timer expiry callback
		void on_timeout(const boost::system::error_code& e);
		void setExpiryCallbackFunc(TimerCallback_t);

	private:
		
		boost::asio::io_service io;
		boost::asio::deadline_timer countdownTimer;

		// prevents the io service from returning after run() when there's no work to be done
		boost::asio::io_service::work work;

		// has to be done in background thread
		void startIOService();

		bool isStarted;
		long originalCountdownDuration;

		TimerCallback_t callbackFunc;
	};
}
