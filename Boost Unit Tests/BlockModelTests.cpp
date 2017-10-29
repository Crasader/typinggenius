//
//  BlockModelTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "BlockChain.h"
#include "BlockModel.h"
#include "BlockCanvas.h"
#include "BlockCanvasModel.h"
#include "GameState.h"
#include "CopyText.h"
#include "Keyboard.h"
#include "KeyboardModel.h"
#include "TextureHelper.h"
#include "Glyph.h"
#include "StatsHUD.h"
#include "StatsHUDModel.h"
#include "StatsHUDView.h"
#include "Notif.h"

namespace ac {
	
	struct BlockModelTestFixture
	{
		BlockModelTestFixture() :
		bc(BlockCanvas::getInstance()), sh(StatsHUD::getInstance()), kb(Keyboard::getInstance()) {
			utilities::loadTextures();
			kb.setUp();
			bc.setUp();
			sh.setUp();
			ct().reset();
			
			sh.view()->onEnter();
		}
		
		~BlockModelTestFixture() {
			
			sh.view()->onExit();
			
			bc.tearDown();
			sh.tearDown();
			kb.tearDown();
			utilities::unloadTextures();
			
			Notif::unsubscribeAll();
		}
		
		BlockCanvas &bc;
		StatsHUD &sh;
		Keyboard &kb;
		
		inline CopyText &ct() { return GameState::getInstance().copyText(); }

		/** 
		 @brief does a compound press-and-release event sequence on the specified key
		 @param nakedLabel the key label without the "key:" prefix, e.g. "a" instead of "key:a"
		 */
		inline void doKeyUpAndDown(std::shared_ptr<KeyboardModel> &kbModel,
								   const string &nakedLabel) {
			string label("key:" + nakedLabel);
			CCTouch *touch = new CCTouch();
			kbModel->keyTouchEvent(label, touch, TouchType::TouchBegan);
			kbModel->keyTouchEvent(label, touch, TouchType::TouchEnded);
			CC_SAFE_DELETE(touch);
		}


		inline void doKeyUp(CCTouch *touch, const string &nakedLabel) {
			Keyboard::getInstance().model()->keyTouchEvent("key:" + nakedLabel, touch, TouchType::TouchEnded);
		}

		inline void doKeyDown(CCTouch *touch, const string &nakedLabel) {
			Keyboard::getInstance().model()->keyTouchEvent("key:" + nakedLabel, touch, TouchType::TouchBegan);
		}
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(BlockModelTests, BlockModelTestFixture)
	
	
	BOOST_AUTO_TEST_CASE(BlockChainSetStringProducesCorrectModels)
	{
		GlyphString glyphString;
		// string glyphString("ABCDEF");
		glyphString.append(Glyph(1));
		glyphString.append(Glyph(2));
		glyphString.append(Glyph(3));
		glyphString.append(Glyph(4));
		glyphString.append(Glyph(5));


		BlockChain bc;
		bc.setString(glyphString);
		
		BOOST_REQUIRE_EQUAL(bc.size(), glyphString.size());
		
		for (size_t i = 0; i < glyphString.size(); i++) {
			BOOST_REQUIRE_EQUAL(bc.itemAt(i).getGlyph().getCode(), glyphString[i].getCode());
		}
	}
	
	
	BOOST_AUTO_TEST_CASE(BlockChainAddItemIncreasesSizeByOne)
	{
		BlockChain bc;
		BlockModel blockModel(Glyph(123));
		bc.addItem(blockModel);
		
		BOOST_REQUIRE_EQUAL(bc.size(), 1);
	}
	
	
	BOOST_AUTO_TEST_CASE(BlockCanvasControllerSetupCreatesBlockChainFromString)
	{
		// string glyphString("ABCDEF");
		GlyphString gs;
		gs.append(Glyph(1));
		gs.append(Glyph(2));
		gs.append(Glyph(3));
		gs.append(Glyph(4));

		bc.model()->updateBlockChain(gs);
		BlockChain chain(bc.model()->getBlockChain());
		
		BOOST_REQUIRE_EQUAL(chain.size(), gs.size());
	}
	
	
	BOOST_AUTO_TEST_CASE(BlockCanvasModelTracksCopyTextReset)
	{
		// the fixture's constructor starts off with a reset
		BOOST_REQUIRE_GT(bc.model()->getBlockChain().size(), 0);
	}
	
	
	BOOST_AUTO_TEST_CASE(BlockCanvasModelTracksCopyTextStateChanges)
	{
		// That is, copy text state changes while blocks are being keyed in (and eliminated) are
		// being reflected in the model
		// string glyphString("ABCDEF");

		GlyphString gs;
		gs.append(Glyph(5));
		gs.append(Glyph(6));
		gs.append(Glyph(7));
		gs.append(Glyph(8));

		BlockChain &chain(bc.model()->getBlockChain());
		ct().reset(); // the result is copy string is loaded.
		ct().setCopyString(gs);
		BOOST_REQUIRE_GT(chain.size(), 0);
	}
	
	
	BOOST_AUTO_TEST_CASE(KeyboardInputUpdatesBlockCanvasModel)
	{
		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());
		//string glyphString("mary had a little lamb whose fleece is white as snow");

		BOOST_REQUIRE_EQUAL(ct().curOffset(), 0);

		GlyphString gs;
		gs.append(Glyph(7)); gs.append(Glyph(8)); gs.append(Glyph(9)); gs.append(Glyph(10));
		gs.append(Glyph(11)); gs.append(Glyph(12)); gs.append(Glyph(13)); gs.append(Glyph(14));
		gs.append(Glyph(15)); gs.append(Glyph(16)); gs.append(Glyph(17)); gs.append(Glyph(18));
		gs.append(Glyph(19)); gs.append(Glyph(20)); gs.append(Glyph(21)); gs.append(Glyph(22));

		const int stringSize(gs.size());
		const int charLimit(ct().getBlocksPerLine());

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);

		string label("key:007"); // we know this corresponds to glyph code-7
		CCTouch *touch = new CCTouch();
		kbModel->keyTouchEvent(label, touch, TouchType::TouchBegan);

		BOOST_REQUIRE_EQUAL(bc.model()->unitsToAdvance(), 1);

		kbModel->keyTouchEvent(label, touch, TouchType::TouchEnded);
		CC_SAFE_DELETE(touch);

		const bool beforeAdvance = false;
		// the -1 represents the 1 char removed from the pool of remaining chars not yet shown
		// (past the right edge of the visible area)
		BOOST_REQUIRE_EQUAL(ct().remainingCharCount(beforeAdvance), stringSize - charLimit - 1);

		BOOST_REQUIRE_EQUAL(ct().curOffset(), 1);
	}


	BOOST_AUTO_TEST_CASE(BlockCanvasModelRespondsToTypingMistakes)
	{
		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());
		// string glyphString("mary had"); // length: 8

		GlyphString gs; // length: 4
		gs.append(Glyph(11));
		gs.append(Glyph(12));
		gs.append(Glyph(13));
		gs.append(Glyph(14));

		size_t blocksPerLine = 2; // arbitrary

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);
		ct().setBlocksPerLine(blocksPerLine);

		BOOST_REQUIRE_EQUAL(ct().curOffset(), 0);

		// first five chars, no mistakes
		this->doKeyUpAndDown(kbModel, "011"); // correct
		this->doKeyUpAndDown(kbModel, "020"); // incorrect (should be '12')

		// after this, "ary had" [12] remains, but since blocks_per_line is 2,
		// visible string is "ar"

		GlyphString gs2;
		gs2.append(Glyph(12)); gs2.append(Glyph(13));

		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs2);
		BOOST_REQUIRE_EQUAL(ct().curOffset(), 1); // point of first mistake
	}


	BOOST_AUTO_TEST_CASE(BlockCanvasModelRespondsToRunningOutOfGlyphs)
	{
		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());
		// string glyphString("mary had a little"); // length: 22

		size_t blocksPerLine = 5; // arbitrary

		GlyphString gs;
		gs.append(Glyph(12));gs.append(Glyph(13));gs.append(Glyph(14));gs.append(Glyph(15));
		gs.append(Glyph(16));gs.append(Glyph(17));gs.append(Glyph(18));gs.append(Glyph(19));
		gs.append(Glyph(20));gs.append(Glyph(21));gs.append(Glyph(22));gs.append(Glyph(23));
		gs.append(Glyph(24));

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);
		ct().setBlocksPerLine(blocksPerLine);

		BOOST_REQUIRE_EQUAL(ct().curOffset(), 0);

		// first five chars, no mistakes
		this->doKeyUpAndDown(kbModel, "012");
		this->doKeyUpAndDown(kbModel, "013");
		this->doKeyUpAndDown(kbModel, "014");
		this->doKeyUpAndDown(kbModel, "015");
		this->doKeyUpAndDown(kbModel, "016");

		// at this point, "had a little" [12] remains, but since blocks_per_line is 10,
		// visible string is "had a litt"
		
		// use this string for comparison with what's in ct's visible string
		gs.clear();
		gs.append(Glyph(17));gs.append(Glyph(18));gs.append(Glyph(19));gs.append(Glyph(20));gs.append(Glyph(21));
		// ct().setCopyString(gs);
		
		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs);

		// and that leaves "le" still not within the visible area
		const bool beforeAdvance = false;
		BOOST_REQUIRE_EQUAL(ct().remainingCharCount(beforeAdvance), 3);

		// now enter "had"
		this->doKeyUpAndDown(kbModel, "017");
		this->doKeyUpAndDown(kbModel, "018");

		BOOST_REQUIRE_EQUAL(ct().remainingCharCount(beforeAdvance), 1);
	}



	BOOST_AUTO_TEST_CASE(MultiClearTest1)
	{
		// hold a (correct) key down, and then keep holding (correct) keys down without releasing them.
		// Upon release, the model advances such that they are eliminated by the view.

		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

		// string glyphString("mary had a little"); // length: 22
		size_t blocksPerLine = 10; // arbitrary

		GlyphString gs;
        gs.append(Glyph(12)); gs.append(Glyph(13)); gs.append(Glyph(14)); gs.append(Glyph(15));
        gs.append(Glyph(16)); gs.append(Glyph(17)); gs.append(Glyph(18)); gs.append(Glyph(19));
        gs.append(Glyph(20)); gs.append(Glyph(21)); gs.append(Glyph(22)); gs.append(Glyph(23));

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);
		ct().setBlocksPerLine(blocksPerLine);

		CCTouch touch[3];
		doKeyDown(&touch[0], "012");
		doKeyDown(&touch[1], "013");
		doKeyDown(&touch[2], "014");

		gs.clear();
		gs.append(Glyph(12)); gs.append(Glyph(13)); gs.append(Glyph(14)); gs.append(Glyph(15));
        gs.append(Glyph(16)); gs.append(Glyph(17)); gs.append(Glyph(18)); gs.append(Glyph(19));
        gs.append(Glyph(20)); gs.append(Glyph(21)); // 10 of them

		// without having released, nothing changes (yet).
		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs);

		doKeyUp(&touch[0], "012");
		// one key is released. this eliminates 12, 13, and 14.

		GlyphString gs3;
		gs3.append(Glyph(15)); gs3.append(Glyph(16)); gs3.append(Glyph(17)); gs3.append(Glyph(18)); gs3.append(Glyph(19));
        gs3.append(Glyph(20)); gs3.append(Glyph(21)); gs3.append(Glyph(22)); gs3.append(Glyph(23));

		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs3);
	}


	BOOST_AUTO_TEST_CASE(MultiClearTest2)
	{
		// When a mistake is encountered somewhere along the way, only the right keys are eliminated.

		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

		// string glyphString("mary had a little"); // length: 22
		size_t blocksPerLine = 10; // arbitrary

		GlyphString gs;
		gs.append(Glyph(12)); gs.append(Glyph(13)); gs.append(Glyph(14)); gs.append(Glyph(15));
        gs.append(Glyph(16)); gs.append(Glyph(17)); gs.append(Glyph(18)); gs.append(Glyph(19));
        gs.append(Glyph(20)); gs.append(Glyph(21)); gs.append(Glyph(22)); gs.append(Glyph(23));

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);
		ct().setBlocksPerLine(blocksPerLine);

		CCTouch touch[3];
		doKeyDown(&touch[0], "012");
		doKeyDown(&touch[1], "013");
		doKeyDown(&touch[2], "014"); // this is the first mistake

		doKeyUp(&touch[0], "012");
		// one key is released. this eliminates "m", "a" and also "r"

		GlyphString gs2;
		gs2.append(Glyph(15));
        gs2.append(Glyph(16)); gs2.append(Glyph(17)); gs2.append(Glyph(18)); gs2.append(Glyph(19));
        gs2.append(Glyph(20)); gs2.append(Glyph(21)); gs2.append(Glyph(22)); gs2.append(Glyph(23));

		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs2);
	}


	BOOST_AUTO_TEST_CASE(MultiClearTest3)
	{
		// Even if all the right keys are entered in a multi-press, they have to be in the right order.

		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

		size_t blocksPerLine = 10; // arbitrary

		GlyphString gs;
        gs.append(Glyph(12)); gs.append(Glyph(13)); gs.append(Glyph(14)); gs.append(Glyph(15));
        gs.append(Glyph(16)); gs.append(Glyph(17)); gs.append(Glyph(18)); gs.append(Glyph(19));
        gs.append(Glyph(20)); gs.append(Glyph(21)); gs.append(Glyph(22)); gs.append(Glyph(23));

		ct().setCopyString(gs);
		GameState::getInstance().setGodMode(false);
		ct().setBlocksPerLine(blocksPerLine);

		CCTouch touch[2];
		doKeyDown(&touch[0], "013"); // mistake
		doKeyDown(&touch[1], "012"); // mistake

		// however, the two glyphs are part of the first two in gs

		doKeyUp(&touch[0], "013"); // only eliminates 12

		GlyphString gs2;
		gs2.append(Glyph(13)); gs2.append(Glyph(14)); gs2.append(Glyph(15));
        gs2.append(Glyph(16)); gs2.append(Glyph(17)); gs2.append(Glyph(18));
        gs2.append(Glyph(19)); gs2.append(Glyph(20)); gs2.append(Glyph(21));
        gs2.append(Glyph(22));

		// only '12' is eliminated
		BOOST_REQUIRE_EQUAL(ct().getVisibleString(), gs2);
	}

	BOOST_AUTO_TEST_SUITE_END()
}
