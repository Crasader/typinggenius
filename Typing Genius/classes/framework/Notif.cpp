//
//  Notif.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 1/5/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#include "Notif.h"
#include <boost/bind.hpp>

namespace ac {
	
#pragma mark - Notif
	
	static signal_t NotificationSignal;
	
	void Notif::send(const string &notifCode, std::shared_ptr<void> data)
	{
		NotificationSignal(notifCode, data);
	}
	
	
	sign_conn_t Notif::subscribe(const signal_t::slot_type &func)
	{
		return NotificationSignal.connect(func);
	}
	
	
	void Notif::unsubscribeAll()
	{
		NotificationSignal.disconnect_all_slots();
	}
	
	
#pragma mark - NotifListener
	
	NotifListener::NotifListener() : notifConn()
	{
		this->notifConn = Notif::subscribe(boost::bind(&NotifListener::handleNotif, this, _1, _2));
	}
	
	
	NotifListener::~NotifListener()
	{
		this->notifConn.disconnect();
	}
	
	
	void NotifListener::handleNotif(const string &code, std::shared_ptr<void> data)
	{
		this->notifCallback(code, data);
	}
	
	
	void NotifListener::notifCallback(const string &, std::shared_ptr<void> data)
	{
		// std::cout << "[NL] notification handled!" << std::endl;
	}
}