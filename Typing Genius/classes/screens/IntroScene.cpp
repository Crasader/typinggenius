//
//  IntroScene.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 1/8/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#include "IntroScene.h"
#include "MainScene.h"
#include "ScreenResolutionHelper.h"

namespace ac {

	USING_NS_CC;
	
	const char *PlayMenuBtnDown = "buttons/intro-menu-to-main-down.png";
	const char *PlayMenuBtnUp = "buttons/intro-menu-to-main-up.png";
	
	const char *IntroBGImage = "backgrounds/intro-screen-bg.png";


	const char *DefaultIntroFont = "fontatlases/avenir-condensed.fnt"; // for the headline

	enum ElementZOrders
	{
		bg = 2,
		menu = 4
	};

#pragma mark - pImpl

	struct IntroLayerImpl : public CCObject
	{
		IntroLayerImpl(IntroLayer *introLayer) : introLayer(introLayer), playMenuItemLabel(), storeMenuItemLabel(),
		aboutMenuItemLabel(), menu() {}

		IntroLayer *introLayer;
		CCLayerColor *layerColor;
		
		CCSprite *backgroundSprite;
		
		CCMenu *menu;
		CCMenuItemLabel *playMenuItemLabel;
		CCMenuItemLabel *storeMenuItemLabel;
		CCMenuItemLabel *aboutMenuItemLabel;
		
		void configureMenu(CCMenu *menu);
		void addMenuButtons(CCMenu *menu);
		void setUpBackgroundImage(CCLayer *layer);
		
		
		void menuPlayCallback();
	};


#pragma mark - Lifetime


	CCScene *IntroLayer::scene()
	{
		CCScene *scene = CCScene::create();
		IntroLayer *layer = IntroLayer::create();
		scene->addChild(layer);
		return scene;
	}


	// nonstatic version
	CCScene *IntroLayer::sceneWithThisLayer()
	{
		CCScene *scene = CCScene::create(); // an autorelease object
		scene->addChild(this);
		return scene;
	}


	IntroLayer::IntroLayer()
	{
		LogD << "Entered IntroLayer constructor";
		pImpl.reset(new IntroLayerImpl(this));
	}


	IntroLayer::~IntroLayer()
	{
		LogD << "Entered IntroLayer destructor";
	}


	// called by IntroLayer::create() in scene
	// most of the heavy lifting is done in onEnter
	bool IntroLayer::init()
	{
		bool ret = false;
		do {
			CC_BREAK_IF(!CCLayer::init());
			ret = true;
		} while (0);
		return ret;
	}


	void IntroLayer::onEnter()
	{
		LogD << "On entering the intro scene....";
		CCLayer::onEnter();
		
		pImpl->setUpBackgroundImage(this);
		
		pImpl->menu = CCMenu::create();
		this->addChild(pImpl->menu, ElementZOrders::menu);
		
		pImpl->configureMenu(pImpl->menu);
		pImpl->addMenuButtons(pImpl->menu);

	}


	void IntroLayer::onExit()
	{
		LogD << "On exiting the intro scene...";
		CCLayer::onExit();
	}
	
	
#pragma mark - Setup
	
	void IntroLayerImpl::addMenuButtons(CCMenu *menu)
	{
		const CCPoint middleOfMenu(ccp(menu->getContentSize().width / 2, menu->getContentSize().height / 2));
		const float menuItemVGap(utilities::isTabletFormFactor() ? 15 : 5);

		// "Play Game"
		if (!this->playMenuItemLabel) {
			CCLabelBMFont *label = CCLabelBMFont::create("Play Game", DefaultIntroFont);
			this->playMenuItemLabel = CCMenuItemLabel::create(label, this, menu_selector(IntroLayerImpl::menuPlayCallback));

			// set position
			this->playMenuItemLabel->ignoreAnchorPointForPosition(false);
			this->playMenuItemLabel->setAnchorPoint(ccp(0.5, 0.5));
			this->playMenuItemLabel->setPosition(middleOfMenu);

			menu->cocos2d::CCNode::addChild(this->playMenuItemLabel);
		}

		if (!this->storeMenuItemLabel) {
			CCLabelBMFont *label = CCLabelBMFont::create("Store", DefaultIntroFont);
			this->storeMenuItemLabel = CCMenuItemLabel::create(label, this, NULL /* no action yet */);


			CCPoint position(middleOfMenu);
			position.y += -1 * (label->getContentSize().height + menuItemVGap);

			// set position
			this->storeMenuItemLabel->ignoreAnchorPointForPosition(false);
			this->storeMenuItemLabel->setAnchorPoint(ccp(0.5, 0.5));
			this->storeMenuItemLabel->setPosition(position);

			menu->cocos2d::CCNode::addChild(this->storeMenuItemLabel);
		}

		if (!this->aboutMenuItemLabel) {
			CCLabelBMFont *label = CCLabelBMFont::create("About", DefaultIntroFont);
			this->aboutMenuItemLabel = CCMenuItemLabel::create(label, this, NULL /* no action yet */);

			CCPoint position(middleOfMenu);
			position.y += -2 * (label->getContentSize().height + menuItemVGap);

			// set position
			this->aboutMenuItemLabel->ignoreAnchorPointForPosition(false);
			this->aboutMenuItemLabel->setAnchorPoint(ccp(0.5, 0.5));
			this->aboutMenuItemLabel->setPosition(position);

			menu->cocos2d::CCNode::addChild(this->aboutMenuItemLabel);
		}

	}
	
	
	void IntroLayerImpl::configureMenu(CCMenu *menu)
	{
		menu->setPosition(ccp(0, 0));
		menu->setAnchorPoint(CCPoint(0, 0));
		menu->ignoreAnchorPointForPosition(false);
		menu->setContentSize(CCDirector::sharedDirector()->getVisibleSize());
	}
	
	
	void IntroLayerImpl::setUpBackgroundImage(CCLayer *layer)
	{
		this->backgroundSprite = CCSprite::create(IntroBGImage);
		this->backgroundSprite->setAnchorPoint(ccp(0.5, 0.5));
		this->backgroundSprite->ignoreAnchorPointForPosition(false);
		
		const CCSize screenSize(CCDirector::sharedDirector()->getVisibleSize());
		
		this->backgroundSprite->setPosition(ccp(screenSize.width / 2, screenSize.height / 2));
		layer->addChild(this->backgroundSprite, ElementZOrders::bg);

	}

	
#pragma mark - Menu Callbacks
	
	void IntroLayerImpl::menuPlayCallback()
	{
		// sample other transition methods as well.
		CCScene *s = MainLayer::scene();
		CCTransitionScene *transition = CCTransitionFlipX::create(0.7, s, tOrientation::kCCTransitionOrientationLeftOver);
		CCDirector::sharedDirector()->replaceScene(transition);
	}

#pragma mark - NotifListener

	void IntroLayer::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		// .. anything?
	}
}