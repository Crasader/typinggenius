//
//  keyboardview.h
//  Typing Genius
//
//  Created by Aldrich Co on 8/15/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Root view of the Keyboard View Hierarchy
//	Responsible for setting up keys as well

#pragma once

#include "cocos2d.h"
#include "MVC.h"
#include "ACTypes.h"

namespace ac
{
	USING_NS_CC;
	using std::string;
	using std::unique_ptr;

	// forward declare
	class KeyView;
	class GameState;
	class KeypressTracker;
	struct KeyboardViewImpl;

	struct KeyboardViewTouchInfo
	{
		string label;
		CCTouch *touch;
		TouchType type;
	};


	class KeyboardView : public View, public CCLayer
	{
	public:
		KeyboardView();
		virtual ~KeyboardView();
		
		// cooks up static create function with autorelease!
		CREATE_FUNC(KeyboardView);
		bool init();
		
		virtual void onEnter();
		virtual void onExit();

		virtual void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
		virtual void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);
		virtual void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent);
		virtual void ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent);
		
		KeyView *getKeyFromLabel(const std::string &label) const;

		const string &keyLabelIntersectingPoint(const CCPoint &point);

		KeypressTracker &keypressTracker();

		void showNewKeySymbols();

		void notifCallback(const string &code, std::shared_ptr<void> data);

	private:
		unique_ptr<KeyboardViewImpl> pImpl;
	};
}
