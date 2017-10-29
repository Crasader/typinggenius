//
//  Keyboard.h
//  Typing Genius
//
//  Created by Aldrich Co on 8/15/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	@brief The keyboard controller

#pragma once

#include "MVC.h"

namespace ac {
	
	using std::unique_ptr;
	
	// forward declares
	class KeyboardView;
	class KeyboardModel;
	class KeyboardConfiguration;

	struct KeyboardImpl;

	//! This is Keyboard (Hello Doxygen!)
	class Keyboard : public Controller<KeyboardModel, KeyboardView, KeyboardConfiguration>
	{
	public:
		
		/** The class is a singleton and this is the preferred way of obtaining an instance */
		static Keyboard& getInstance();
		~Keyboard();
		
		/** set up and tear down */
		virtual void setUp();
		virtual void tearDown();
		
	private:
		Keyboard();
		Keyboard(const Keyboard &);
		Keyboard &operator=(const Keyboard &);
		unique_ptr<KeyboardImpl> pImpl;
	};
	
	
	class KeyboardConfiguration : public Configuration
	{
	public:
		static shared_ptr<KeyboardConfiguration> createKeyboardConfiguration(const string &config = "default");
		virtual bool initialize(Keyboard& keyboard) = 0;
	};
}
