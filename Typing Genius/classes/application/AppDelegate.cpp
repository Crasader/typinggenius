//
//  Typing_GeniusAppDelegate.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/12/13.
//  Copyright Aldrich Co 2013. All rights reserved.
//

#include "AppDelegate.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "MainScene.h"
#include "IntroScene.h"
#include "AppContext.h"
#include "DebugSettingsHelper.h"
#include "ScreenResolutionHelper.h"
#include "GameState.h"
#include "Notif.h"


USING_NS_CC;
using namespace CocosDenshion;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
}


bool AppDelegate::applicationDidFinishLaunching()
{
	using std::string;
	using namespace ac;

	// initialize director
	CCDirector *pDirector = CCDirector::sharedDirector();
	pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());
	
	// turn on display FPS
	bool shouldShowFPS = DebugSettingsHelper::sharedHelper().boolValueForProperty("show_fps_stats", false);
	pDirector->setDisplayStats(shouldShowFPS);
	
	// set FPS. the default value is 1.0/60 if you don't call this
	pDirector->setAnimationInterval(1.0 / 60);
	
	// create a scene. it's an autorelease object
	CCScene *pScene;
	if (ac::DebugSettingsHelper::sharedHelper().boolValueForProperty("start_in_main_screen")) {
		pScene = ac::MainLayer::scene();
	} else {
		pScene = ac::IntroLayer::scene();
	}
	
	// run
	pDirector->runWithScene(pScene);
	
	// -- figure out ideal resolution and point asset lookup to right path
	utilities::initializeScreenSizeParameters();
	utilities::initializeSearchPathsAndResolutionOrder();

	ac::Notif::send("AppDelegate_FinishLaunch");

	return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
	CCDirector::sharedDirector()->stopAnimation();
	SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
	SimpleAudioEngine::sharedEngine()->pauseAllEffects();

	ac::Notif::send("AppDelegate_EnteredBackground");
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
	CCDirector::sharedDirector()->startAnimation();
	SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
	SimpleAudioEngine::sharedEngine()->resumeAllEffects();

	ac::Notif::send("AppDelegate_EnteringForeground");
}


void AppDelegate::relaunchApp()
{
	LogI << "Restarting app";

	// needed?
	// CCPoolManager::sharedPoolManager()->purgePoolManager();
	// CCPoolManager::sharedPoolManager()->finalize();

	// initialize director
	CCDirector *pDirector = CCDirector::sharedDirector();
	pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());

	// remove all touch handlers. This may also clear some autorelease objects..
	pDirector->getTouchDispatcher()->removeAllDelegates();

	// set FPS. the default value is 1.0/60 if you don't call this
	pDirector->setAnimationInterval(1.0 / 60);

	// create a separate scene. it's an autorelease object
	CCScene *pScene = ac::MainLayer::scene();

	// run
	pDirector->replaceScene(pScene);
	// destructor for the replaced Scene only happens at this point.
}