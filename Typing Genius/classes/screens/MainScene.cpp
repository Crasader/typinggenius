//
//  MainScene.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "MainScene.h"

#include "AppDelegate.h"
#include "BlockCanvas.h"
#include "BlockCanvasView.h"
#include "CCControlButton.h"
#include "DebugSettingsHelper.h"
#include "GameState.h"
#include "IntroScene.h"
#include "Keyboard.h"
#include "KeyboardModel.h"
#include "KeyboardView.h"
#include "KeypressTracker.h"
#include "ScreenResolutionHelper.h"
#include "StatsHUD.h"
#include "StatsHUDView.h"
#include "TextureHelper.h"
#include "SimpleAudioEngine.h"


namespace ac {
	
	// images for the buttons
	const char *RestartBtnUpFilename = "buttons/restart-up.png";
	const char *RestartBtnDownFilename = "buttons/restart-down.png";

	const char *StartBtnUpFilename = "buttons/start-up.png";
	const char *StartBtnDownFilename = "buttons/start-down.png";

	const char *GodBtnOnFilename = "buttons/god-on.png";
	const char *GodBtnOffFilename = "buttons/god-off.png";

	const char *NewGameOnFilename = "buttons/clean-slate-on.png";
	const char *NewGameOffFilename = "buttons/clean-slate-off.png";
	
	const char *ReturnToIntroScreenButtonFilename = "buttons/x.png";

	// background music
	const char *BGMRelaxing = "bgm/catchy-piano.mp3";

	using std::shared_ptr;
	using std::static_pointer_cast;
	USING_NS_CC;
	using CocosDenshion::SimpleAudioEngine;

#pragma mark - pImpl
	
	struct MainLayerImpl
	{
		MainLayerImpl(MainLayer *mainLayer) : audio((SimpleAudioEngine::sharedEngine())), mainLayer(mainLayer)
		{

		}

		SimpleAudioEngine *audio;
		MainLayer *mainLayer;

		void removePostGameClickShield();
		
		void returnToIntroScreen();
	};
	
	
	// created during onEnter (rather than init)... good idea?
	struct MainSceneElements
	{
		MainSceneElements() : menu() {
			LogI << "MainSceneElements constructor";
			menu = CCMenu::create();
			menu->setPosition(CCPointZero);
			// resync with gamestate's idea of god mode.
			isGodMode = GameState::getInstance().isGodMode();
		}
		
		~MainSceneElements() {
			LogI << "MainSceneElements destructor";
		}
				
		CCMenu *menu;
		CCMenuItem *menuItemForTag(int tag) const;
		bool isGodMode;
	};
	
	
#pragma mark - Lifetime and Setup
	
	CCScene *MainLayer::scene()
	{
		CCScene *scene = CCScene::create(); // an autorelease object
		MainLayer *layer = MainLayer::create(); // autorelease object
		scene->addChild(layer);
		return scene;
	}
	
	
	// nonstatic version
	CCScene *MainLayer::sceneWithThisLayer()
	{
		CCScene *scene = CCScene::create(); // an autorelease object
		scene->addChild(this);
		return scene;
	}
	
	
	MainLayer::MainLayer()
	{
		LogD << "Entered MainLayer constructor";
		pImpl.reset(new MainLayerImpl(this));
	}
	
	
	MainLayer::~MainLayer()
	{
		LogD << "Entered MainLayer destructor";
	}
	
	
	// called by MainLayer::create() in scene
	// most of the heavy lifting is done in onEnter
	bool MainLayer::init()
	{
		bool ret = false;
		do {
			CC_BREAK_IF(!CCLayer::init());
			ret = true;
		} while (0);
		return ret;
	}
	
	
	void MainLayer::addRestartButton()
	{
		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
		
		CCMenuItemImage *pRestartItem = CCMenuItemImage::create(RestartBtnUpFilename, RestartBtnDownFilename,
								this, menu_selector(MainLayer::menuRestartCallback));
		
		const float margin(5);
		pRestartItem->setPosition(ccp(origin.x + visibleSize.width - margin, origin.y + margin));

		pRestartItem->ignoreAnchorPointForPosition(false);
		pRestartItem->setAnchorPoint(CCPoint(1, 0)); // lower right corner
		utilities::scaleNodeToSize(pRestartItem, CCSizeMake(40, 40), CCSizeMake(75, 75));
		
		mainSceneElements->menu->addChild(pRestartItem, 0, RestartButtonMenuTag);
	}
	
	
	void MainLayer::addStartButton()
	{
		const float margin(5);
		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
		
		CCMenuItemImage *pStartItem = CCMenuItemImage::create(StartBtnUpFilename, StartBtnDownFilename,
															  this, menu_selector(MainLayer::menuStartCallback));
		
		const CCSize &supposedSize(utilities::isTabletFormFactor() ? CCSizeMake(75, 75) : CCSizeMake(40, 40));
		
		if (DebugSettingsHelper::sharedHelper().boolValueForProperty("show_restart_button", false)) {
			// assumes another button took its original place
			pStartItem->setPosition(ccp(origin.x + visibleSize.width - margin - supposedSize.width, origin.y + margin));
		} else {
			pStartItem->setPosition(ccp(origin.x + visibleSize.width - margin, origin.y + margin));
		}

		pStartItem->ignoreAnchorPointForPosition(false);
		pStartItem->setAnchorPoint(CCPoint(1, 0)); // lower right corner
		utilities::scaleNodeToSize(pStartItem, supposedSize);
		
		mainSceneElements->menu->addChild(pStartItem, 0, StartButtonMenuTag);
	}
	

	void MainLayer::addGodButton()
	{
		const float margin(5);
		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();


		const char *defaultFile = mainSceneElements->isGodMode ? GodBtnOnFilename : GodBtnOffFilename;
		const char *pressFile = mainSceneElements->isGodMode ? GodBtnOffFilename : GodBtnOnFilename;

		CCMenuItemImage *pGodItem = CCMenuItemImage::create(defaultFile, pressFile,
															  this, menu_selector(MainLayer::menuGodCallback));

		const CCSize &supposedSize(utilities::isTabletFormFactor() ? CCSizeMake(75, 75) : CCSizeMake(40, 40));

		pGodItem->setPosition(ccp(origin.x + visibleSize.width - margin, origin.y + margin));

		pGodItem->ignoreAnchorPointForPosition(false);
		pGodItem->setAnchorPoint(CCPoint(1, 0)); // lower right corner
		utilities::scaleNodeToSize(pGodItem, supposedSize);

		mainSceneElements->menu->addChild(pGodItem, 0, GodButtonMenuTag);
	}


	void MainLayer::addCleanSlateButton()
	{
		const float margin(5);
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

		CCMenuItemImage *pCleanSlateItem = CCMenuItemImage::create(RestartBtnUpFilename,
				RestartBtnDownFilename, this, menu_selector(MainLayer::/*menuRestartCallback*/cleanSlateCallback));

		const CCSize &supposedSize(utilities::isTabletFormFactor() ?
				CCSizeMake(75, 75) : CCSizeMake(40, 40));

		pCleanSlateItem->setPosition(ccp(origin.x + margin, origin.y + margin));
		pCleanSlateItem->ignoreAnchorPointForPosition(false);
		pCleanSlateItem->setAnchorPoint(CCPoint(0, 0)); // lower left corner

		utilities::scaleNodeToSize(pCleanSlateItem, supposedSize);

		mainSceneElements->menu->addChild(pCleanSlateItem, 0, GodButtonMenuTag);
	}
	
	
	void MainLayer::addCloseButton()
	{
		const float margin(5);
		const CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
		const CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		
		CCMenuItemImage *closeButtonItem = CCMenuItemImage::create(ReturnToIntroScreenButtonFilename,
																   ReturnToIntroScreenButtonFilename, this,
																   menu_selector(MainLayer::closeButtonCallback));
		
		closeButtonItem->setPosition(ccp(margin, origin.y + visibleSize.height - margin));
		closeButtonItem->ignoreAnchorPointForPosition(false);
		closeButtonItem->setAnchorPoint(CCPoint(0, 1)); // upper left corner
		mainSceneElements->menu->addChild(closeButtonItem);
	}
	
	
	void MainLayer::addOddsAndEnds()
	{
		// menu and buttons
		this->addChild(mainSceneElements->menu);
		this->addGodButton();
		this->addCleanSlateButton();
		this->addCloseButton();
	}
	
	
	void MainLayer::onEnter()
	{
		LogD << "On entering the main scene....";
		CCLayer::onEnter();

		utilities::loadTextures();

		Keyboard::getInstance().setUp();
		BlockCanvas::getInstance().setUp();
		StatsHUD::getInstance().setUp();

		GameState::getInstance().resetGameState(); // can only be reset once BC, SH, and KB are properly initialized

		this->addChild(BlockCanvas::getInstance().view());
		this->addChild(StatsHUD::getInstance().view());
		this->addChild(Keyboard::getInstance().view());

		// have the stats panel subscribe to the Keyboard black box
		mainSceneElements.reset(new MainSceneElements);

		bool showDebugButtons = DebugSettingsHelper::sharedHelper().boolValueForProperty("show_debug_buttons", false);
		if (showDebugButtons) {
			this->addOddsAndEnds();
		}

		// play the music
		bool shouldDisableBGM = DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_bgm", false);
		if (!shouldDisableBGM && !pImpl->audio->isBackgroundMusicPlaying()) {
			pImpl->audio->setBackgroundMusicVolume(0.2);
			pImpl->audio->playBackgroundMusic(BGMRelaxing, true);
		}
	}


	void MainLayer::onExit()
	{
		LogD << "On exiting the main scene...";
		CCLayer::onExit();

		Keyboard::getInstance().tearDown();
		BlockCanvas::getInstance().tearDown();
		StatsHUD::getInstance().tearDown();


		pImpl->audio->stopBackgroundMusic(true);

	}


	void MainLayerImpl::removePostGameClickShield()
	{
		if (GameState::getInstance().isInPostGameState()) {
			LogI << "Keypress in post game state happened!";
			GameState::getInstance().setPostGameState(false);
			mainLayer->resetState();
		}
	}


	// duplicated in onEnter... should refactor.
	void MainLayer::resetState()
	{
		GameState &gs(GameState::getInstance());
		if (gs.isGameOver()) {
			gs.resetPlayer();
		}

		StatsHUD &hud(StatsHUD::getInstance());
		BlockCanvas &bc(BlockCanvas::getInstance());
		Keyboard &kb(Keyboard::getInstance());

		hud.view()->removeFromParent();
		hud.tearDown();

		bc.view()->removeFromParent();
		bc.tearDown();

		kb.view()->removeFromParent();
		kb.tearDown();

		kb.setUp();
		bc.setUp();
		hud.setUp();

		GameState::getInstance().resetGameState();

		this->addChild(bc.view());
		this->addChild(hud.view());
		this->addChild(kb.view());

		bool showDebugButtons = DebugSettingsHelper::sharedHelper().boolValueForProperty("show_debug_buttons", false);
		if (showDebugButtons) {
			// the menu would have been buried beneath by the readding of views
			mainSceneElements->menu->setZOrder(kb.view()->getZOrder() + 1);
		}
	}
	
	
	void MainLayerImpl::returnToIntroScreen()
	{
		CCScene *s = IntroLayer::scene();
		CCTransitionScene *transition = CCTransitionFlipX::create(0.7, s, tOrientation::kCCTransitionOrientationLeftOver);
		CCDirector::sharedDirector()->replaceScene(transition);
	}


#pragma mark - NotifListener


	void MainLayer::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		if ("AppDelegate_EnteringForeground" == code) {
			GameState::getInstance().stop(false);
			this->resetState(); // it should also show a "resuming" dialog while the app brings itself back

		} else if ("AppDelegate_EnteredBackground" == code) {
			LogI << "App entered background!";

		} else if ("AppDelegate_FinishLaunch" == code) {
			LogI << "App finished launching!";

		} else if ("KeypressTracker_RequiresUIRefresh" == code) {
			// flip game state from post game=true to false. This allows "Press any key to continue"
			shared_ptr<KeypressTrackerUpdateInfo> pInfo = static_pointer_cast<KeypressTrackerUpdateInfo>(data);
			if (pInfo->touchType == TouchType::TouchBegan && pInfo->label != "") {
				pImpl->removePostGameClickShield();
			}
		}
	}


#pragma mark - Button Press Callbacks

	void MainLayer::menuStartCallback(CCObject *pSender)
	{
		// playbackControlSignal(); doesn't exist no more

		// at this point the button to start must be hidden. (It's unhidden
		// after a reset.)
		CCMenuItem *menuItem = mainSceneElements->menuItemForTag(StartButtonMenuTag);
		if (menuItem) {
			menuItem->setVisible(false);
		}
	}


	void MainLayer::menuRestartCallback(CCObject *pSender)
	{
		// do some keyboard related stuff
		LogI << "Pressed restart button";

		// contrast with cleanSlateCallback
		this->resetState();
	}
	

	void MainLayer::cleanSlateCallback(CCObject *pSender)
	{
		LogI << "Clean slate!";
		GameState::getInstance().resetPlayer();

		AppDelegate *appDelegate = (AppDelegate *)CCApplication::sharedApplication();
		if (appDelegate) {
			appDelegate->relaunchApp();
		}
	}
	
	
	void MainLayer::closeButtonCallback(CCObject *pSender)
	{
		pImpl->returnToIntroScreen();
	}


	void MainLayer::menuGodCallback(CCObject *pSender)
	{
		mainSceneElements->isGodMode = !mainSceneElements->isGodMode;
		GameState::getInstance().setGodMode(mainSceneElements->isGodMode);

		const char *defaultFile = mainSceneElements->isGodMode ? GodBtnOnFilename : GodBtnOffFilename;
		const char *pressFile = mainSceneElements->isGodMode ? GodBtnOffFilename : GodBtnOnFilename;

		CCMenuItemSprite *sprite = static_cast<CCMenuItemSprite *>(pSender);
		if (sprite) {
			sprite->setNormalImage(CCSprite::create(defaultFile));
			sprite->setSelectedImage(CCSprite::create(pressFile));
		}
	}


#pragma mark - Main Scene Elements
	
	CCMenuItem *MainSceneElements::menuItemForTag(int tag) const
	{
		CCNode *node = menu->getChildByTag(tag);
		if (node) {
			CCMenuItem *menuItem = dynamic_cast<CCMenuItem *>(node);
			if (menuItem) {
				return menuItem;
			}
		}
		return nullptr;
	}
}
