//
//  Keyboard.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/15/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "Keyboard.h"
#include "KeyboardView.h"
#include "KeyboardModel.h"
#include "DefaultKeyboardConfiguration.h"
#include "GameState.h"
#include "CopyText.h"
#include "KeypressTracker.h"
#include "ScoreKeeper.h"

namespace ac
{
	USING_NS_CC;
	using boost::bind;

#pragma mark - pImpl Structure
	
	struct KeyboardImpl
	{
		KeyboardImpl(shared_ptr<KeyboardConfiguration> config) : pConfig(config) {}
		// destructor won't be called, so I had to call a deleter...
		shared_ptr<KeyboardConfiguration> pConfig;
		
		~KeyboardImpl() {
			LogD << "Destructor";
		}
		
	};
	
	// Deleter: seems like a regular destructor won't work
	void deletePimpl(KeyboardImpl *impl)
	{
		impl->pConfig = nullptr;
	}
	
	
#pragma mark - Singleton

	Keyboard& Keyboard::getInstance()
	{
		static Keyboard instance;
		return instance;
	}

	
#pragma mark - Lifetime

	Keyboard::Keyboard()
	{
		LogD << "Constructor for Keyboard";
		pImpl.reset(new KeyboardImpl(KeyboardConfiguration::createKeyboardConfiguration()));
		pImpl->pConfig->initialize(*this);
	}


	Keyboard::~Keyboard()
	{
		LogD << "Destructor for Keyboard";
	}


	void Keyboard::setUp()
	{
		LogD << "now at setUp in Keyboard with layer ";
		if (!pImpl->pConfig) {
			pImpl->pConfig = KeyboardConfiguration::createKeyboardConfiguration();
			pImpl->pConfig->initialize(*this);
		}
		
		// if they're nil, have them force created
		model();
		view();
	}


	void Keyboard::tearDown()
	{
		LogD << "In tear down method of Keyboard";
		
		deregisterSignals();
		
		this->setModel(nullptr);
		
		this->view()->release();
		this->setView(nullptr);
		
		pImpl->pConfig = nullptr;
	}


#pragma mark - KeyboardConfiguration
	
	shared_ptr<KeyboardConfiguration> KeyboardConfiguration::createKeyboardConfiguration(const string &config)
	{
		if (0 == config.compare("default")) {
			LogD << "Default keyboard config chosen.";
			shared_ptr<KeyboardConfiguration> ret(new DefaultKeyboardConfiguration());
			return ret;
		}
		
		LogE << "No suitable carriage configuration found!";
		return nullptr;
	}

}