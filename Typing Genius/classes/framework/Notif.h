//
//  Notif.h
//  Typing Genius
//
//  Created by Aldrich Co on 1/5/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#ifndef __Typing_Genius__Notif__
#define __Typing_Genius__Notif__

#include <iostream>
#include <boost/signals2.hpp>
#include <boost/function.hpp>

namespace ac {

	using std::string;
	using boost::signals2::connection;
	using boost::signals2::signal;
	
	typedef connection sign_conn_t;
	
	typedef signal<void(const string &, std::shared_ptr<void> data)> signal_t;
	
	class Notif
	{
	public:

		static void send(const string &code, std::shared_ptr<void> data = nullptr);
		
		// subscribers keep the return values and use it to unsubscribe (calling conn.disconnect() on it)
		// Children of NotifListener, which include Model and View classes, already call this.
		static sign_conn_t subscribe(const signal_t::slot_type &func);
		
		static void unsubscribeAll();
	};
	
	
	class NotifListener
	{
	public:
		
		NotifListener();
		virtual ~NotifListener() = 0; // pure virtual
		
		void handleNotif(const string &, std::shared_ptr<void> data);
		
		virtual void notifCallback(const string &, std::shared_ptr<void> data);
	
	private:
		sign_conn_t notifConn;
		
	};

}

#endif /* defined(__Typing_Genius__Notif__) */
