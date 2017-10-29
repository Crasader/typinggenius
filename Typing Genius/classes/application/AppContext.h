//
//  AppContext.h
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include <iostream>

namespace ac {
	
	void doSomeLoggingStuff();
	void doSomeJSONStuff();
	void doSomeRegexStuff();

	//! This is AppContext (Hello Doxygen!)
	class AppContext
	{
	public:
		AppContext()
		{
			// this just needs to be set once during the app
			LogI << "Starting app context";
		}
		
		/** do stuff related to boost and other libraries to demonstrate how to use them. */
		void performLibrariesDemo();
		
		/** The class is a singleton and this is the preferred way of obtaining an instance */
		static AppContext& sharedAppContext()
		{
			static AppContext instance;
			return instance;
		}
	
	private:

	};
	
	
}