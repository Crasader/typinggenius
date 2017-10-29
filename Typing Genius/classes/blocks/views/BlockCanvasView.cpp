//
//  BlockCanvasView.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockCanvasView.h"

#include "BlockCanvas.h"
#include "BlockCanvasModel.h"
#include "BlockChain.h"
#include "BlockModel.h"
// #include "BlockTypesetter.h"
#include "BlockView.h"
#include "CopyText.h"
#include "DebugSettingsHelper.h"
#include "GameState.h"
#include "Glyph.h"
#include "GlyphMap.h"
#include "Keyboard.h"
#include "KeyboardModel.h"
#include "KeypressTracker.h"
#include "MCBCallLambda.h"
#include "ScreenResolutionHelper.h"
#include "SimpleAudioEngine.h"
#include "StatsHUD.h"
#include "StatsHUDView.h"
#include "TextureHelper.h"


namespace ac {

	const char *SFXMistake = "sfx/bad-type.mp3";
	const char *SFXWhoosh = "sfx/whoosh.mp3";

#pragma mark - pImpl

	struct BlockCanvasViewImpl : public CCObject /* only to satisfy the requirement for callbacks */
	{
		CCLayerColor *colorLayer;
		CCSpriteBatchNode *spriteBatchNode; // all blocks should be added here!
		BlockCanvasView *canvasView; // pointer passed from the main class
		CCSprite *bgSprite;

		// holds just the pointers to the blocks. This is so that so you don't need to dig through
		// canvasNode->getChildren with potentially expensive dynamic_cast<>s
		// they don't have to be shown
		std::set<BlockView *> allBlocks;

		// blocks which are marked for reuse (and are hidden)
		std::set<BlockView *> reusableBlocks;

		// keeps track of the total number of simultaneous pop operations. When this goes to zero, the next phase begins
		int numOfOngoingBlockPops;

		// --------- LOCATE BLOCKS ---------
		BlockView *blockViewWithIndex(int index);

		// --------- LAYING OUT BLOCKS (NO ANIMATION) ---------
		// this function takes a block chain, creates a number of blocks based on specs, and then
		// adds the block sprites to the canvas' sprite batch node. It then gives the Canvas Model
		// and View a way to access respective block model and view objects.
		void layoutBlockChain(std::shared_ptr<BlockCanvasModel> canvasModel);
		void recycleAllBlocks();
		void addBlockViewToCanvas(BlockView *blockView); // use this instead of addChild
		void setBlockPosition(BlockView *blockView);
		void setBlockGlyph(BlockView *blockView, const Glyph &glyph);
		
		// --------- OBTAIN NEW BLOCKS OR REUSE THEM ---------
		// should use this instead of blindly creating BlockViews naively. canvasView controls
		// the recycle pool
		BlockView *createOrReuseBlockView(const Glyph &glyph);
		BlockView *getReusableBlock(); // may return NULL, so check for the return value

		// --------- HOW BLOCKS GET ADJUSTED WHILE TYPING (ANIMATION) ----------
		void advanceBlockChain(size_t advanceUnits, bool spaceWasUsed = false);
		void blinkBlockChain(size_t units); // used when there is a mistake
		void highlightBlockChain(size_t preadvanceUnits);
		void dimBlockChain();
		void marchOffBlockChain();
		void slideInBlocks(size_t offscreenBlockCount);

		// call this in preparation for running an action on a block
		void stopBlockAnimations(BlockView *);

		CCFiniteTimeAction *popAndRecycleBlock(BlockView *blockView, bool spaceWasUsed = false);
		CCFiniteTimeAction *slideBlock(BlockView *blockView, bool relaxed);
		CCFiniteTimeAction *blinkBlock(BlockView *blockView);
		CCFiniteTimeAction *highlightBlock(); // run on getGlyphSprite
		CCFiniteTimeAction *fadeInBlock();

		// callbacks
		void donePoppingAllBlocksCallback(); // callbacks after finished eliminating blocks
		void doneSlidingAllBlocksCallback();
		void doneSlidingABlock(CCObject *obj);
		void finishedPopAnimation(); // note there's also another version found in BlockView, also used
		void removeFloatingScoreLabel(CCNode *node);
		
		// block score animation
		void floatStatsAboveBlock(const BlockCanvasModelUpdateInfo &info, BlockView *blockView);


		void setBlockViewProperties(BlockView *blockView, const BlockModel &blockModel);
		

		BlockCanvasViewImpl(BlockCanvasView *canvasView) : colorLayer(), bgSprite(), spriteBatchNode(), allBlocks(),
		numOfOngoingBlockPops(0), reusableBlocks(), 	canvasView(canvasView) {
			// ...
		}
	};


#pragma mark - Lifetime

	BlockCanvasView::BlockCanvasView()
	{
		pImpl.reset(new BlockCanvasViewImpl(this));
	}


	BlockCanvasView::~BlockCanvasView()
	{
	}


	bool BlockCanvasView::init()
	{
		bool ret = false;
		do {
			// do initialization here!
			const float keyboardHeight = Keyboard::getInstance().model()->getKeyboardSize().height;
			const CCSize screenSz(utilities::visibleSize());

			// this is good for the iPhone retina.
//			CCSize phoneSz(480, 140); // should actually belong to the model (who reads it from a config)
//			CCSize widePhoneSz(568, 140);
//			CCSize tabletSz(1024, 320);

			const float bcvHeight = screenSz.height - keyboardHeight;

			CCSize phoneSz(480, bcvHeight); // should actually belong to the model (who reads it from a config)
			CCSize widePhoneSz(568, bcvHeight);
			CCSize tabletSz(1024, bcvHeight);

			CCSize sz;
			if (utilities::isPhoneFormFactor()) {
				sz = utilities::isWidePhoneFormFactor() ? widePhoneSz : phoneSz;
			} else {
				sz = tabletSz;
			}

			this->setContentSize(sz);
			this->setAnchorPoint(CCPointMake(0, 1)); // upper left corner
			this->ignoreAnchorPointForPosition(false);
			this->setPosition(CCPoint(0, utilities::visibleSize().height));

			pImpl->colorLayer = CCLayerColor::create(ccc4(12, 3, 3, 255));
			CC_BREAK_IF(!pImpl->colorLayer);

			pImpl->colorLayer->setContentSize(sz);
			pImpl->colorLayer->setPosition(0, 0);
			pImpl->colorLayer->setAnchorPoint(CCPointZero);
			pImpl->colorLayer->ignoreAnchorPointForPosition(false);
			this->addChild(pImpl->colorLayer);


			pImpl->bgSprite = CCSprite::create("backgrounds/noveau.png");
			pImpl->bgSprite->setPosition(ccp(0, 0));
			pImpl->bgSprite->setAnchorPoint(CCPointZero);
			pImpl->bgSprite->ignoreAnchorPointForPosition(false);
			this->addChild(pImpl->bgSprite);

			CCParticleSystemQuad *particleBG = CCParticleSystemQuad::create("particles/jugurta-noir.plist");
			this->addChild(particleBG);

			CC_BREAK_IF(!particleBG);

			LogD << ">>> Creating sprite batch node for block canvas";
			pImpl->spriteBatchNode = CCSpriteBatchNode::create(utilities::textureForBlockCanvas(), 64);

			CC_BREAK_IF(!pImpl->spriteBatchNode);

			pImpl->spriteBatchNode->setPosition(CCPointZero);
			pImpl->spriteBatchNode->ignoreAnchorPointForPosition(false);
			pImpl->spriteBatchNode->setContentSize(sz);
			this->addChild(pImpl->spriteBatchNode);
			
			ret = true;
		} while (false);
		return ret;
	}


#pragma mark - CCNode Overrides

	// we plan to use these
	void BlockCanvasView::onEnter()
	{
		// add some stuff to the batch node.
		CCLayer::onEnter(); // do this last.
	}


	void BlockCanvasView::onExit()
	{
		CCLayer::onExit();  // do this last.
	}


#pragma mark - NotifListener Callback

	void BlockCanvasView::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		const BlockCanvasModel &model(*BlockCanvas::getInstance().model());
		
		if ("BlockCanvasModel_Load" == code) {
			pImpl->recycleAllBlocks();
			pImpl->layoutBlockChain(BlockCanvas::getInstance().model());
		} else if ("BlockCanvasModel_Advance" == code) {
			pImpl->advanceBlockChain(model.unitsToAdvance(), false);
		} else if ("BlockCanvasModel_AdvanceWithSpace" == code) {
			pImpl->advanceBlockChain(model.unitsToAdvance(), true);
		} else if ("BlockCanvasModel_Mistake" == code) {
			pImpl->blinkBlockChain(model.unitsWithMistake());
		} else if ("BlockCanvasModel_Highlight" == code) {
			pImpl->highlightBlockChain(model.unitsToAdvance());
		} else if ("BlockCanvasModel_EndTimer" == code) {
			pImpl->dimBlockChain();
			pImpl->marchOffBlockChain();
		} else if ("BlockCanvasModel_PerBlockScoreDelta" == code) {
			BlockView *blockView = pImpl->blockViewWithIndex(0);
			if (blockView) {
				std::shared_ptr<BlockCanvasModelUpdateInfo> pInfo = std::static_pointer_cast<BlockCanvasModelUpdateInfo>(data);
				pImpl->floatStatsAboveBlock(*pInfo, blockView);
			}
		} // i think there should be an event that responds to increase / decrease in block chain length.
	}


#pragma mark - Canvas operations

	void BlockCanvasViewImpl::addBlockViewToCanvas(BlockView *blockView)
	{
		if (blockView->getParent() != spriteBatchNode) {
			spriteBatchNode->addChild(blockView);
			blockView->setCanvasView(canvasView);
			this->allBlocks.insert(blockView);
		}
	}

	
	CCNode *BlockCanvasView::getCanvasNode()
	{
		return pImpl->spriteBatchNode;
	}


#pragma mark - Create, Reuse, and Access Blocks

	// not the fastest due to its use of dynamic_cast. Perhaps use static_cast?
	BlockView *BlockCanvasViewImpl::blockViewWithIndex(int index)
	{
		CCNode *canvas = canvasView->getCanvasNode();

		CCArray *canvasChildren = canvas->getChildren();
		CCObject *obj;
		CCARRAY_FOREACH(canvasChildren, obj) {
			BlockView *blockV = dynamic_cast<BlockView *>(obj);
			if (blockV) {
				if (blockV->getIndex() == index) {
					return blockV;
				}
			}
		}
		return NULL;
	}


	BlockView *BlockCanvasViewImpl::createOrReuseBlockView(const Glyph &glyph)
	{
		BlockView *aBlockView = getReusableBlock();
		if (!aBlockView) {
			// LogI << "creating new block";
			aBlockView = BlockView::createWithGlyph(glyph);
		} else {
			// LogI << "reusing old block";
			setBlockGlyph(aBlockView, glyph);
		}

		return aBlockView;
	}

	// blocks that go offscreen will be collected here. Before creating a block, will check the array if there still are
	// available blocks in the pool.
	void BlockCanvasView::recycleBlock(BlockView *block)
	{
		block->setIndex(-1);
		block->setVisible(false);
		pImpl->reusableBlocks.insert(block);
		LogI << "recycling block.";
	}


	// may return NULL, so check for the return value
	BlockView *BlockCanvasViewImpl::getReusableBlock()
	{
		BlockView *ret = NULL;
		auto it = reusableBlocks.begin();
		if (it != reusableBlocks.end()) {
			ret = *it;

//			if (ret->getParent() != NULL) {
//				// it's not yet ready
//				return NULL;
//			}
			ret->setVisible(true);
			ret->setScaleX(1);
			ret->setScaleY(1);

			reusableBlocks.erase(it);
			// ret->release();

		}
		return ret;
	}


	void BlockCanvasViewImpl::recycleAllBlocks()
	{
		for (BlockView *block: this->allBlocks) {
			canvasView->recycleBlock(block);
		}
	}


#pragma mark - Handle Block Touch Events

	void BlockCanvasView::attemptedDowngradeOfBlockEnclosureLevel(BlockView *block)
	{
		std::shared_ptr<BlockCanvasViewUpdateInfo> pInfo(new BlockCanvasViewUpdateInfo);
		pInfo->blockIndex = block->getIndex();
		Notif::send("BlockCanvasView_EncasementDowngraded", pInfo);
		block->setEnclosureLevel(0);
	}


	void BlockCanvasView::obstructionRemovedFromBlockView(BlockView *block)
	{
		// tell the model. What should happen is that the frog count will eventually be updated for the player
		std::shared_ptr<BlockCanvasViewUpdateInfo> pInfo(new BlockCanvasViewUpdateInfo);
		pInfo->blockIndex = block->getIndex();
		Notif::send("BlockCanvasView_ObstructionRemoved", pInfo);
	}


#pragma mark - Change Block Properties

	// should be able to handle strings which are shorter than the necessary blocks
	void BlockCanvasViewImpl::layoutBlockChain(std::shared_ptr<BlockCanvasModel> canvasModel)
	{
		BlockChain &bc(canvasModel->getBlockChain());

		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

		LogD1 << "layout block chain";
		BlockView *aBlockView = NULL;
		for (size_t i = 0; i < bc.size(); i++) {
			// try to reuse first, if possible.

			const BlockModel &blockModel(bc.itemAt(i));
			const Glyph &g(blockModel.getGlyph());
			
			aBlockView = createOrReuseBlockView(g);
			aBlockView->setIndex(i);
			
			GlyphMap &gm(GameState::getInstance().glyphMap());

			string keyLabel(gm.keyLabelForGlyph(g));
			if (!keyLabel.empty()) {
				// look up KeyboardModel
				if (g.getCode() == 0) { // space
					aBlockView->enableHint(false);
				} else {
					aBlockView->enableHint(true);
					const utilities::RGBByte &rgb(kbModel->getColorForKey(keyLabel));
					aBlockView->setHintColor(utilities::ccc3FromRGB(rgb));
				}
			}

			// hide it first, then fade in the text
			aBlockView->showBlock(aBlockView, false);
			aBlockView->getGlyphSprite()->runAction(fadeInBlock());

			addBlockViewToCanvas(aBlockView);

			// prepare for sliding in
			setBlockPosition(aBlockView);
			aBlockView->setPositionX(canvasView->getContentSize().width + aBlockView->blockSize().width);

			setBlockViewProperties(aBlockView, blockModel);
		}

		this->slideInBlocks(canvasModel->totalBlocksDisplayable());
	}
	

	// uses index assigned to it as the basis for its position
	void BlockCanvasViewImpl::setBlockPosition(BlockView *blockView)
	{
		blockView->setPosition(canvasView->blockPositionForBlock(blockView->getIndex(),
													 blockView->blockSize().width));

		// LogD3 << boost::format("block with index %d, moved %d places, set to x: %.2f") %
		//	blockView->getIndex() % advancePositions % blockView->getPosition().x;
	}


	CCPoint BlockCanvasView::blockPositionForBlock(int index, float blockWidth)
	{
		std::shared_ptr<BlockCanvasModel> model(BlockCanvas::getInstance().model());
		size_t numBlocks(model->totalBlocksDisplayable());
		float myWidth(utilities::visibleSize().width);

		// NOTE: this is artwork-dependent
		const float noveauBlockYOffset = utilities::isTabletFormFactor() ? 48 : (utilities::isWidePhoneFormFactor() ?
																				 28 : 25);

		float xOffset = blockWidth / 2 + (myWidth - numBlocks * blockWidth) / 2;
		float yOffset = blockWidth / 2 + noveauBlockYOffset;

		return ccp(xOffset + blockWidth * index, yOffset);
	}
	
	
	void BlockCanvasViewImpl::setBlockGlyph(BlockView *blockView, const Glyph &glyph)
	{
		blockView->setGlyph(glyph);
	}


	// this is a static method
	CCSize BlockCanvasView::getIndividualBlockSize()
	{
		CCSize ret(CCSizeZero);
		BlockView *aBlockView = BlockView::createWithGlyph(Glyph(5 /*arbitrary*/));
		if (aBlockView) {
			ret = aBlockView->blockSize();
		}
		return ret;
	}
	
	
	size_t BlockCanvasView::blockCountPerRow()
	{
		CCSize blockSize(getIndividualBlockSize());
		CCSize visibleSize(utilities::visibleSize());
		return (size_t) visibleSize.width / (size_t) blockSize.width;
	}


	void BlockCanvasViewImpl::setBlockViewProperties(BlockView *blockView, const BlockModel &blockModel)
	{
		blockView->setHasObstruction(false);
		if (blockModel.hasObstruction()) {
			blockView->setHasObstruction(true);
		}

		int encasementLevel = blockModel.isEncased() ? 2 : 0;
		blockView->setEnclosureLevel(encasementLevel);
		LogD << "assigning encasement level of " << encasementLevel << " to block with index " << blockView->getIndex();

		blockView->setObstructionTimeDuration(blockModel.getObstructionTimeDuration());
		blockView->setObstructionIntroductionDelay(blockModel.getObstructionIntroductionDelay());
	}


#pragma mark - Respond to BlockChain changes
	
	// this kicks off the entire animation sequence. Done after a successful check.
	// affects the X blocks in view. `advanceUnits` blocks will be slid out of view.
	// Could be called rapid-fire depending on the user's typing speed!
	void BlockCanvasViewImpl::advanceBlockChain(size_t advanceUnits, bool spaceWasUsed)
	{
		shared_ptr<BlockCanvasModel> model(BlockCanvas::getInstance().model());

		if (numOfOngoingBlockPops >= model->totalBlocksDisplayable()) {
			LogI << "slow down! too many already. number of ongoing pop block operations pending: "
				<< numOfOngoingBlockPops;
		}

		// pop the blocks first. when that finishes, slide the blocks in through a callback
		int rightMostPoppedBlockIndex = -1; // useful for the index adjustment op later

		for (size_t i = 0; i < advanceUnits; i++) {

			// this relies on the next step (for loop) which readjusts the remaining blocks' indices after
			// these are taken off
			BlockView *blockView = blockViewWithIndex(i);

			if (blockView) {

				// halt any animations before the current one
				stopBlockAnimations(blockView);

				const float delay = model->slideBackTime();

				auto donePoppingABlock = MCBPlatformSupport::MCBCallLambda::createWithDelay(delay, [=] () {
					numOfOngoingBlockPops -= 1;
					if (numOfOngoingBlockPops < 1) {
						CopyText &ct(GameState::getInstance().copyText());
						ct.registerStreakFinished();

						if (!GameState::getInstance().isGameOver()) {
							donePoppingAllBlocksCallback();
						}
					}
				});

				blockView->runAction(CCSequence::create(popAndRecycleBlock(blockView, spaceWasUsed), donePoppingABlock, NULL));
				numOfOngoingBlockPops += 1;

				if (blockView->getIndex() > rightMostPoppedBlockIndex) {
					rightMostPoppedBlockIndex = blockView->getIndex();
				}

				blockView->setIndex(-1); // take this block out of consideration for the next step

			} else {
				LogW << "no block found...";
			}
		}

		// drop the indices of the remaining blocks so they reflect proper index ordering.
		for (BlockView *block : this->allBlocks) {
			int blockIndex = block->getIndex();
			if (blockIndex > rightMostPoppedBlockIndex) {
				int newIndex = blockIndex - 1 - rightMostPoppedBlockIndex;
				block->setIndex(newIndex);
			}
		}
	}


	void BlockCanvasViewImpl::blinkBlockChain(size_t units) // used when there is a mistake
	{
#ifndef BOOST_TEST_TARGET
		if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXMistake);
		}
#endif

		for (size_t i = 0; i < units; i++) {
			BlockView *blockView = blockViewWithIndex(i);
			if (blockView) {
				// halt any animations before the current one
				stopBlockAnimations(blockView);
				blockView->runAction(blinkBlock(blockView));
			} else {
				LogI << "Note: less than " << units << " blocks found to work on!";
			}
		}
	}
	
	
	void BlockCanvasViewImpl::highlightBlockChain(size_t preadvanceUnits)
	{
		for (size_t i = 0; i < preadvanceUnits; i++) {
			BlockView *blockView = blockViewWithIndex(i);
			if (blockView) {
				// halt any animations before the current one
				// stopBlockAnimations(blockView);
				blockView->setZOrder(1000 + i); // make it appear higher up than the others
				blockView->getGlyphSprite()->runAction(highlightBlock());
			} else {
				LogI << "Note: less than " << preadvanceUnits << " blocks found to work on!";
			}
		}
	}


	void BlockCanvasViewImpl::dimBlockChain()
	{
		for (BlockView *blockView : this->allBlocks) {
			blockView->getGlyphSprite()->runAction(CCFadeTo::create(0.3, 40));
		}
	}


	void BlockCanvasViewImpl::marchOffBlockChain()
	{
		for (BlockView *blockView : this->allBlocks) {

			if (blockView->getIndex() != -1) { // target only onscreen blocks

				blockView->stopAllActions();

				float rate = 0.4;
				int index = -2; // blocks will slide into a position denoted by index (whether onscreen or not)
				CCPoint position = canvasView->blockPositionForBlock(index, blockView->blockSize().width);

				CCActionInterval *slide = CCMoveTo::create(rate, position);
				slide = CCEaseIn::create(slide, 1.5);

				float delayTime = 0.075 * (1 + blockView->getIndex());
				CCDelayTime *delay = CCDelayTime::create(delayTime);
				blockView->runAction(CCSequence::create(delay, slide, NULL));
			}
		}
	}
	
	
	/** Now taken over by streak value (above 2) */
	void BlockCanvasViewImpl::floatStatsAboveBlock(const BlockCanvasModelUpdateInfo &info, BlockView *blockView)
	{
		if (info.curStreakLevel < 2) { return; }
		
		// you probably need a pool of these and reuse when the animations are done
		const char *TimerFont = "fontatlases/avenir-timer.fnt";
		static boost::format scoreFmt("+%d");
		static boost::format streakFmt("x%d!");
		// const char *scoreLblValue = ((scoreFmt % info.scoreUpdateDelta).str()).c_str();
		const char *streakLblValue = ((streakFmt % info.curStreakLevel).str()).c_str();
		
//		CCLabelBMFont *scoreLabel = CCLabelBMFont::create(scoreLblValue, TimerFont);
//		canvasView->addChild(scoreLabel); // can't add to the BatchSpriteNode
		
		CCLabelBMFont *streakLabel = CCLabelBMFont::create(streakLblValue, TimerFont);
		canvasView->addChild(streakLabel); // can't add to the BatchSpriteNode
		
		// fix position
		// get the position of the block first
		CCPoint blockGlobalPoint(blockView->getParent()->convertToWorldSpace(blockView->getPosition()));
		
		CCPoint canvasViewPointOfBlockView = canvasView->convertToNodeSpace(blockGlobalPoint);
		float blockHeight = blockView->blockSize().height;
		canvasViewPointOfBlockView.y += blockHeight / 2;
//		scoreLabel->setPosition(canvasViewPointOfBlockView);
		streakLabel->setPosition(canvasViewPointOfBlockView);
		
		// animate up
		static float duration = 0.3;
		float scaleMultiplier = 1.5 + 0.5 * (info.curStreakLevel / 3 - 1);
		
		CCFiniteTimeAction *fadeIn = CCFadeIn::create(duration);
		CCFiniteTimeAction *floatUp = CCEaseOut::create(CCMoveBy::create(duration, ccp(0, 5)), 8);
		CCFiniteTimeAction *floatLeft = CCEaseOut::create(CCMoveBy::create(0.6, ccp(-2, 0)), 4);
		CCFiniteTimeAction *scaleUp = CCEaseOut::create(CCScaleTo::create(duration, scaleMultiplier), 8);
		CCDelayTime *delayBeforeRemove = CCDelayTime::create(0.15);
		
		CCCallFuncN *removeLabel = CCCallFuncN::create(this, callfuncN_selector(BlockCanvasViewImpl::removeFloatingScoreLabel));
		CCSequence *sequence = CCSequence::create(CCSpawn::create(fadeIn, floatUp, scaleUp, floatLeft, NULL),
												  delayBeforeRemove, removeLabel, NULL);
		
		// scoreLabel->runAction(sequence);
		
		
		const RGBByte streakStartColor = { 255, 104, 96 };
		
		// assuming you can't go higher than 12 in a row
		RGBByte rgb = utilities::nextRGBByteHueShift(streakStartColor, 270.0f * (info.curStreakLevel / 12.0f));
		streakLabel->setColor(utilities::ccc3FromRGB(rgb));
		streakLabel->runAction(sequence);
	}
	
	
	void BlockCanvasViewImpl::removeFloatingScoreLabel(CCNode *node)
	{
		// is this node the label?
		node->removeFromParent();
	}
	

#pragma mark - Configure CCAction Animations for Blocks

	void BlockCanvasViewImpl::stopBlockAnimations(BlockView *blockView)
	{
		blockView->stopAllActions();
		blockView->setIsAnimating(false);
		blockView->showBlock(blockView, true);
	}

	
	// will shrink the block and then when finished, return it to the 'pool.
	CCFiniteTimeAction *BlockCanvasViewImpl::popAndRecycleBlock(BlockView *blockView, bool spaceWasUsed)
	{
		// note: fade off only affects the parent not the children!
		// http://stackoverflow.com/questions/12409697/ccsprite-fadeout-with-children
		
		CCActionInterval *pop = CCScaleTo::create(0.1, 1, 0.01f); // after which the block is hidden
		pop = CCEaseIn::create(pop, 2);

		StatsHUDView *shView(StatsHUD::getInstance().view());
		
		CCSize bs(blockView->blockSize());
		CCSize progressBlockSize(shView->progressBlockSize());
		float newScale = progressBlockSize.width / bs.width;
		
		// note: while translating coordinates, take anchorpoints of involved nodes under consideration.
		
		CopyText &ct(GameState::getInstance().copyText());
		
		// ct.curOffset gives you the position after it advances
		CCPoint globalPoint(shView->convertToWorldSpace(shView->progressPositionAtIndex(ct.curOffset() - 1)));

		// account for differences in anchor points between the two sets of blocks
		globalPoint.x += progressBlockSize.width / 2;
		globalPoint.y += progressBlockSize.height / 2;
		
		CCPoint pointInBC(canvasView->convertToNodeSpace(globalPoint));
		
		CCActionInterval *flyUp = CCMoveTo::create(0.3, pointInBC);
		flyUp = CCEaseIn::create(flyUp, 2);

		CCActionInterval *flyLeft = CCMoveTo::create(0.2, ccp(0 - 2 * bs.width, blockView->getPosition().y));
		flyLeft = CCEaseIn::create(flyLeft, 2);
		
		CCActionInterval *scaleDown = CCScaleTo::create(0.3, newScale);
		scaleDown = CCEaseOut::create(scaleDown, 2);
		
		CCSpawn *spawn;

		if (spaceWasUsed) {
			if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXWhoosh);
			}
			spawn = CCSpawn::create(flyLeft, NULL);
		} else {
			spawn = CCSpawn::create(flyUp, scaleDown, NULL);
		}

		// the call back reclaims the block
		CCCallFunc *recycleAction = CCCallFuncO::create(blockView,
														callfuncO_selector(BlockView::finishedPopAnimation),
														canvasView);
		
		CCCallFunc *finishedAnimationAction = CCCallFunc::create(this, callfunc_selector(BlockCanvasViewImpl::finishedPopAnimation));
		
		// once the fly up animation is done you can tell StatsHud so it can update the view.
		// but shview is currently mainly governed by CopyText.
		// here's the plan:
		// 1. bcv uses the shview for dimensions
		// 2. bcv uses the ct for the up to date info (where to put the stuff).. this is independent of shmodel's ideas
		// of which blocks are in use.
		// 3. bcv sends a signal to the shmodel to make the shview update, using ct.

		CCSequence *sequence = CCSequence::create(spawn, finishedAnimationAction, recycleAction, NULL);
		return sequence;
	}
	
	
	// will slide the existing blocks by x units to the left
	CCFiniteTimeAction *BlockCanvasViewImpl::slideBlock(BlockView *blockView, bool relaxed)
	{
		float rate = relaxed ? 0.4 : 0.2;
		CCPoint position = canvasView->blockPositionForBlock(blockView->getIndex(), blockView->blockSize().width);
		// LogI << boost::format("position for index %d: %.2f") % index % position.x;
		CCActionInterval *action = CCMoveTo::create(rate, position);
		action = CCEaseIn::create(action, 2);
		return action;
	}


	CCFiniteTimeAction *BlockCanvasViewImpl::blinkBlock(BlockView *blockView)
	{
		CCMoveTo *resetPosition = CCMoveTo::create(0.1, canvasView->blockPositionForBlock(blockView->getIndex(), blockView->blockSize().width));

		CCCallFunc *blinkOff = CCCallFunc::create(blockView, callfunc_selector(BlockView::hideBlock));
		CCCallFunc *blinkOn = CCCallFunc::create(blockView, callfunc_selector(BlockView::showBlock));
		CCDelayTime *delay = CCDelayTime::create(0.05);
		CCSequence *theBlinking = CCSequence::create(blinkOff, delay, blinkOn, delay, blinkOff, delay, blinkOn, NULL);

		return CCSpawn::create(resetPosition, theBlinking, NULL);
	}
	
	
	CCFiniteTimeAction *BlockCanvasViewImpl::highlightBlock()
	{
		CCFiniteTimeAction *goUp = CCEaseIn::create(CCScaleTo::create(0.15, 1.3), 2);
		CCFiniteTimeAction *goDown = CCEaseIn::create(CCScaleTo::create(0.10, 1), 2);
		return CCSequence::create(goUp, goDown, NULL);
	}


	CCFiniteTimeAction *BlockCanvasViewImpl::fadeInBlock()
	{
		return CCFadeTo::create(1.0, 255);
	}


#pragma mark - Post-Animation Callbacks

	void BlockCanvasViewImpl::doneSlidingABlock(CCObject *obj)
	{
		BlockView *blockView = static_cast<BlockView *>(obj);
		if (blockView) {
			blockView->setIsAnimating(false);

			if (blockView->hasObstruction()) {
				// attempt could fail if frog has already gone away.
				blockView->tryScheduleRevealObstruction();
			}
		}
		
		bool inProgress = false;
		for (BlockView *blockView : allBlocks) {
			if (blockView->isAnimating()) {
				inProgress = true;
				break;
			}
		}
		
		if (!inProgress) {
			doneSlidingAllBlocksCallback();
		}
	}
	
	
	void BlockCanvasViewImpl::finishedPopAnimation()
	{
		// this informs the model. Could be improved by specifying which block caused this.
		Notif::send("BlockCanvasView_FinishedPopAnimation");
	}


	void BlockCanvasViewImpl::doneSlidingAllBlocksCallback()
	{
		LogD4 << "done sliding all blocks";

		CopyText &ct(GameState::getInstance().copyText());
		ct.tryProcessingNextBufferedInput();

		KeypressTracker &kpt(GameState::getInstance().keypressTracker());
		kpt.reset();
	}


	// all blocks in a sequence, at least
	void BlockCanvasViewImpl::donePoppingAllBlocksCallback()
	{
		// all of the blocks needed to be shuffled out has been done so.
		const std::shared_ptr <BlockCanvasModel> &model(BlockCanvas::getInstance().model());
		std::shared_ptr<KeyboardModel> kbModel(Keyboard::getInstance().model());

		BlockChain &blockChain = model->getBlockChain();

		// now slide the chain in; add as many blocks that were lost to the end (right)
		//
		// all popped blocks have -1 as their index. I can use that info to determine how many
		// needs to be brought back to the right end.

		int poppedCount = 0;
		const int totalBlocksDisplayable = model->totalBlocksDisplayable();
		const int visibleStringSize = model->visibleStringSize();
		
		for (BlockView *block : this->allBlocks) {
			if (block->getIndex() == -1) {
				poppedCount++;
			}
		}
		
		int restorable = MAX(0, visibleStringSize - totalBlocksDisplayable + poppedCount);
		// the number of new blocks to slide in = poppedCount (with some proviso)
		
		// reuse or create as many blocks; bring in the next glyphs
		restorable = MIN(poppedCount, restorable);

		// the index given to these new blocks are the final ones they'll be getting.
		for (size_t i = 0; i < restorable; i++) {
			int newIndex = i + totalBlocksDisplayable - poppedCount;

			const BlockModel &blockModel(blockChain.itemAt(newIndex));
			const Glyph &glyph(blockModel.getGlyph());

			BlockView *blockView = createOrReuseBlockView(glyph);
			if (blockView) {
				blockView->setIndex(newIndex);
				
				GlyphMap &gm(GameState::getInstance().glyphMap());

				string keyLabel(gm.keyLabelForGlyph(glyph));
				if (!keyLabel.empty()) {
					// look up KeyboardModel
					if (glyph.getCode() == 0) { // space
						blockView->enableHint(false);
					} else {
						blockView->enableHint(true);
						const utilities::RGBByte &rgb(kbModel->getColorForKey(keyLabel));
						blockView->setHintColor(utilities::ccc3FromRGB(rgb));
					}
				}

				addBlockViewToCanvas(blockView);

				// prepare for sliding in
				setBlockPosition(blockView);
				blockView->setPositionX(canvasView->getContentSize().width + blockView->blockSize().width);

				setBlockViewProperties(blockView, blockModel);

			} else {
				LogW << "trouble! can't find a block view to add";
			}
		}

		slideInBlocks(poppedCount);
	}


	/** 
	 *	@brief performs the slide-to-left animation on all blocks, whether onscreen or offscreen. This is done after
	 *	some blocks had been cleared (popped).
	 *	@param offscreenBlockCount the number of blocks being slid in that will undergo a 'relaxed' animation. This is
	 *	typically done on blocks that were made offscreen after being popped.
	 */
	void BlockCanvasViewImpl::slideInBlocks(size_t offscreenBlockCount)
	{
		const std::shared_ptr <BlockCanvasModel> &model(BlockCanvas::getInstance().model());
		BlockChain &blockChain = model->getBlockChain();
		const int totalBlocksDisplayable = model->totalBlocksDisplayable();

		for (BlockView *blockV : this->allBlocks) {
			// prepare to slide
			stopBlockAnimations(blockV);

			int index(blockV->getIndex());
			if (index > -1) {

				setBlockGlyph(blockV, blockChain.itemAt(index).getGlyph());

				// set z-order
				blockV->setZOrder(totalBlocksDisplayable - index);

				// do the slide: if its one of the newer blocks take a little longer to slide in.
				bool relaxed = index >= totalBlocksDisplayable - offscreenBlockCount;

				CCCallFuncO *func = CCCallFuncO::create(this,
														callfuncO_selector(BlockCanvasViewImpl::doneSlidingABlock),
														blockV);

				if (relaxed) {
					// staggered entry: the left blocks from outside the screen will make their entrance a bit sooner
					const float relaxDelay = 0.075 * (1 + index - totalBlocksDisplayable + offscreenBlockCount);
					blockV->runAction(CCSequence::create(CCDelayTime::create(relaxDelay), slideBlock(blockV, relaxed),
														 func, NULL));
				} else {
					blockV->runAction(CCSequence::create(slideBlock(blockV, relaxed), func, NULL));
				}

				// actually should wait till all the sliding blocks are done.
				blockV->setIsAnimating(true);
			}
		}
	}

}

