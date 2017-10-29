//
//  BlockViewTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "BlockCanvas.h"
#include "BlockCanvasView.h"
#include "BlockCanvasModel.h"
#include "TextureHelper.h"
#include "BlockView.h"
#include "Keyboard.h"
#include "KeyboardModel.h"
#include "CopyText.h"
#include "GameState.h"
#include "BlockModel.h"
#include "Glyph.h"
#include "BlockChain.h"

namespace ac {

	struct BlockViewTestFixture
	{
		BlockViewTestFixture() : bc(BlockCanvas::getInstance())
		{
			utilities::loadTextures();
			bc.setUp();
			Keyboard::getInstance().setUp();
			GameState::getInstance().resetGameState();
			bc.view()->onEnter();
		}

		~BlockViewTestFixture()
		{
			bc.view()->onExit();
			utilities::unloadTextures();
			bc.tearDown();
			Keyboard::getInstance().tearDown();
		}
		BlockCanvas &bc;

		inline CopyText &ct() { return GameState::getInstance().copyText(); }

		inline void doKeyUpAndDown(std::shared_ptr<KeyboardModel> &kbModel,
								   const string &nakedLabel) {
			string label("key:" + nakedLabel);
			CCTouch *touch = new CCTouch();
			kbModel->keyTouchEvent(label, touch, TouchType::TouchBegan);
			kbModel->keyTouchEvent(label, touch, TouchType::TouchEnded);
			CC_SAFE_DELETE(touch);
		}

		std::map<size_t, BlockView *> blocksInCanvas();
	};


	std::map<size_t, BlockView *> BlockViewTestFixture::blocksInCanvas()
	{
		std::map<size_t, BlockView *> ret;

		CCNode *canvas = bc.view()->getCanvasNode();
		CCArray *canvasChildren = canvas->getChildren();
		CCObject *obj;
		CCARRAY_FOREACH(canvasChildren, obj) {
			BlockView *blockView = dynamic_cast<BlockView *>(obj);
			if (blockView) {
				ret[blockView->getIndex()] = blockView;
			}
		}
		return ret;
	}


	BOOST_FIXTURE_TEST_SUITE(BlockViewTests, BlockViewTestFixture)


	BOOST_AUTO_TEST_CASE(AsManyBlockViewsAsThereAreBlocksInModelChain)
	{
		// there are as many blocks as there glyphs in the visible string.

		GlyphString gs;
		gs.append(Glyph(5));
		gs.append(Glyph(6));
		gs.append(Glyph(7));

		bc.model()->updateBlockChain(gs);
		BOOST_REQUIRE_EQUAL(bc.model()->getBlockChain().size(), 3);

		gs.clear();
		bc.model()->updateBlockChain(gs);
		BOOST_REQUIRE_EQUAL(bc.model()->getBlockChain().size(), 0);
	}


	BOOST_AUTO_TEST_CASE(BlockViewRespondsToModel)
	{
		// have model update, itself, advancing 1 unit
		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

//		const string glyphString("mary had a little"); // length: 22
//		ct().setCopyString(glyphString);

		GlyphString gs;
		gs.append(Glyph(12));
		gs.append(Glyph(13));
		gs.append(Glyph(14));
		ct().setCopyString(gs);

		this->doKeyUpAndDown(kbModel, "012"); // after this, blocks should start with "a"

		std::map<size_t, BlockView *> blockMap(blocksInCanvas());

		BlockView *firstBlock = blockMap[0];
		BOOST_REQUIRE_GT(blockMap.size(), 0);
		BOOST_REQUIRE(firstBlock);

		BOOST_REQUIRE_EQUAL(firstBlock->getGlyph().getCode(), 13);
	}


	BOOST_AUTO_TEST_CASE(BlockGlyphContentsReflectModelState)
	{
		std::map<size_t, BlockView *> blockMap(blocksInCanvas());
		BlockChain &bchain(bc.model()->getBlockChain());
		for (size_t i = 0; i < blockMap.size(); i++) {
			int shownOnBlockView = blockMap[i]->getGlyph().getCode();
			BOOST_REQUIRE_EQUAL(shownOnBlockView, bchain.itemAt(i).getGlyph().getCode());
		}
	}


	BOOST_AUTO_TEST_CASE(BlockPositionsReflectsIndexOrder)
	{

		// blockPositionForBlock(blockView->getIndex(), blockView->blockSize().width);

		std::map<size_t, BlockView *> blockMap(blocksInCanvas());
		BOOST_REQUIRE_GT(blockMap.size(), 0);
		float marker = 0;
		for (size_t i = 0; i < blockMap.size(); i++) {

			// this won't work: the blocks will have to be slid in via animation.
			// float x = blockMap[i]->getPosition().x;

			BlockView *blockView = blockMap[i];
			float x = bc.view()->blockPositionForBlock(blockView->getIndex(), blockView->blockSize().width).x;

			BOOST_REQUIRE_GT(x, marker); // increasing X
			marker = x;
		}
	}

	BOOST_AUTO_TEST_SUITE_END()
}