//
//  MainLayerTest.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/30/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//


#include <boost/test/unit_test.hpp>
#include "MainScene.h"

namespace ac {
	
	USING_NS_CC;
	
	struct MainLayerTestFixture
	{
		MainLayerTestFixture() {
			mainLayer = MainLayer::create();
			initializeScreen();
		}
		
		~MainLayerTestFixture() {
			mainLayer->onExit();
		}
		
		MainLayer *mainLayer;
		
		void initializeScreen()
		{
			// initialize director
			CCDirector *pDirector = CCDirector::sharedDirector();
			pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());
			
			// create a scene. it's an autorelease object
			CCScene *pScene = mainLayer->sceneWithThisLayer();
			
			// run
			pDirector->runWithScene(pScene);
			
			mainLayer->onEnter();
		}
		
		CCMenu *getMenu() const
		{
			CCArray *mainLayerChildren = mainLayer->getChildren();
			CCObject *obj;
			CCARRAY_FOREACH(mainLayerChildren, obj) {
				CCMenu *menu = dynamic_cast<CCMenu *>(obj);
				if (menu) {
					return menu;
				}
			}
			return nullptr;
		}
		
		CCMenuItem *menuItemForTag(int tag) const
		{
			CCMenu *menu = getMenu();
			if (!menu) { return nullptr; }
			
			CCNode *node = menu->getChildByTag(tag);
			if (node) {
				CCMenuItem *menuItem = dynamic_cast<CCMenuItem *>(node);
				if (menuItem) {
					return menuItem;
				}
			}
			return nullptr;
		}
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(MainLayerTests, MainLayerTestFixture)

//	Note: the start button has been removed as of 2013-10-31
//	BOOST_AUTO_TEST_CASE(AfterPressingStartButtonWillDisappear)
//	{
//		CCMenuItem *startMenuItem = menuItemForTag(StartButtonMenuTag);
//		BOOST_REQUIRE(startMenuItem);
//		BOOST_REQUIRE_EQUAL(startMenuItem->isVisible(), true);
//		mainLayer->menuStartCallback(startMenuItem);
//		BOOST_REQUIRE_EQUAL(startMenuItem->isVisible(), false);
//	}

	
	BOOST_AUTO_TEST_SUITE_END()
}