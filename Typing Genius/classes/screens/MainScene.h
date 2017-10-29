//
//  MainScene.h
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Provides a button to carry out some demo stuff.

#pragma once

#include "cocos2d.h"
#include "ACTypes.h"
#include "Notif.h"

namespace ac {
	
	USING_NS_CC;
	struct MainSceneElements;
	struct MainLayerImpl;
	
	const int RestartButtonMenuTag = 1;
	const int StartButtonMenuTag = 2;
	const int GodButtonMenuTag = 3;
	
	class MainLayer : public CCLayer, public NotifListener
	{
	public:
		virtual bool init();
		
		virtual void onEnter();
		virtual void onExit();

		MainLayer();
		~MainLayer();
		
		static CCScene *scene();
		CCScene *sceneWithThisLayer();
		
		// some callback
		void menuRestartCallback(CCObject *pSender);
		void menuStartCallback(CCObject *pSender);
		void menuGodCallback(CCObject *pSender);
		void cleanSlateCallback(CCObject *pSender);
		void closeButtonCallback(CCObject *pSender);

		// preprocessor macro for "static create()" constructor
		CREATE_FUNC(MainLayer);
		
		void receivedEndGameCallback();
		
		// this does the job without releasing everything.
		void resetState();

		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);

	private:
		// shouldn't these be inside MainSceneElements?
		void addRestartButton(); // for debugging purposes. Can be turned off in debug settings.json
		void addStartButton();
		void addGodButton();
		void addCleanSlateButton();
		void addCloseButton();
		void addOddsAndEnds();

		std::shared_ptr<MainSceneElements> mainSceneElements;
		std::unique_ptr<MainLayerImpl> pImpl;
		void keyboardEventCallback(std::string &keyLabel, KeyPressState &state);
	};
}