//
//  KeypressTracker.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	A helper class of KeyboardView

#pragma once

#include "cocos2d.h"
#include "ACTypes.h"
#include <deque>

namespace ac {

	USING_NS_CC;

	using std::string;
	using std::map;
	using std::vector;
	using std::set;
	using std::deque;

	enum class TouchType;
	class KeyboardView;


	struct KeypressTrackerUpdateInfo
	{
		size_t oldKeysSize;
		size_t newKeysSize;
		string label; // of last update (could be "")
		TouchType touchType;
	};


	class KeypressTracker
	{
	public:
		// key labels with modifier "mod" have special status
		// e.g. "key:mod_shift"
		KeypressTracker();

		void reset();

		// the return value is to indicate whether any substantial addition has been made to the buffer
		void trackTouchEvent(CCTouch *touch, const string &label, TouchType type, bool isMapped);

		// use to get what is currently being pressed at any time.
		set<string> keysInDownState() const;

		const KeyEvent removeNextKeyEventFromBuffer();

		inline bool hasElementsInBuffer() const { return !keyPressBuffer.empty(); }

	private:
		map<CCTouch *, string> tracker;
		
		string lastKeyDown; // ie pressed and registered as new

		// this will keep track of key presses in the order they are encountered
		// only TouchTypes "Began" and "Ended" will be added to the buffer... the other types are compressed to these
		deque<KeyEvent> keyPressBuffer;
	};


	
	
	
	
}