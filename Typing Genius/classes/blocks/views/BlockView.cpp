//
//  BlockView.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockView.h"
#include "BlockTypesetter.h"
#include "BlockCanvasView.h"
#include "Utilities.h"
#include "ScreenResolutionHelper.h"
#include "MCBCallLambda.h"
#include "SimpleAudioEngine.h"
#include "Glyph.h"
#include "DebugSettingsHelper.h"


namespace ac {

	// sound effects
	const char *SFXRemoveFrog = "sfx/clear-frog.mp3";

	// frame names are part of the blocks spritesheet.
	const char *BlockSpriteFrameName = "empty.png";
	const char *BackgroundFrameName = "block-bg.png";
	const char *SplatterFrameName = "block-bg-splatter.png";
	const char *OverlayFrameName = "frog-overlay.png";

	const char *HardIceFrameName = "hard-ice-overlay.png";
	const char *SoftIceFrameName = "soft-ice-overlay.png";

	// this amount will be used along with touch hold time to determine how far along the ice should be "melting"
	const float TotalSecIceMeltTime = 0.5f;

	const ccColor3B LightColorTint { 120, 120, 120 };
	const ccColor3B DarkColorTint { 12, 12, 12 };

#pragma - pImpl Definition

	struct BlockViewImpl
	{
		CCSprite *theBlock; // the actual block sprite
		CCSprite *theGlyph;
		CCSprite *theSplatter;
		CCSprite *theOverlay;

		CCSprite *theOuterEnclosure;
		CCSprite *theInnerEnclosure;

		BlockCanvasView *bcView;
		BlockView *blockView;

		Glyph glyph;

		bool hintIsEnabled;
		bool hasObstruction;

		// 'frog' has already been shown and has disappeared after an amount of time
		bool obstructionHasBeenHidden;

		size_t enclosureLevel;

		float touchHoldTimeCounter;
		bool lastTouchInsideBlock; // used when tracking ccTouchesMoved;
		
		BlockViewImpl(BlockView *blockView): blockView(blockView), lastTouchInsideBlock(false),
		glyph(), theOuterEnclosure(), theInnerEnclosure(), theGlyph(), theBlock(), theSplatter(),
		hintIsEnabled(false), hasObstruction(false), theOverlay(), enclosureLevel(0), bcView(), touchHoldTimeCounter(0),
		obstructionHasBeenHidden(false)
		{
			// ...
		}


		void tryRemoveObstruction();
		void setIceEncasementMeltRatio(float ratio); // 1 is fully melted.
	};


#pragma mark - Lifetime

	BlockView::BlockView() : animating(false)
	{
		// LogI << "Inside BlockView constructor";
		pImpl.reset(new BlockViewImpl(this));
	}


	BlockView::~BlockView()
	{
		// LogI << "Inside BlockView destructor";
	}


	BlockView *BlockView::createWithGlyph(const Glyph &glyph)
	{
		BlockView *ret = new BlockView();
		if (ret->initWithGlyph(glyph)) {
			ret->autorelease();
			return ret;
		}

		CC_SAFE_DELETE(ret); // in case of bad alloc
		return nullptr;
	}


	bool BlockView::initWithGlyph(const Glyph &glyph)
	{
		bool ret = false;
		do {
			CC_BREAK_IF(!CCSprite::initWithSpriteFrameName(BlockSpriteFrameName));

			pImpl->theBlock = CCSprite::createWithSpriteFrameName(BackgroundFrameName);
			CC_BREAK_IF(!pImpl->theBlock);
			this->addChild(pImpl->theBlock);

			pImpl->theBlock->setColor(DarkColorTint);
			
			// the splatter
			pImpl->theSplatter = CCSprite::createWithSpriteFrameName(SplatterFrameName);
			CC_BREAK_IF(!pImpl->theSplatter);
			this->addChild(pImpl->theSplatter);
			
			pImpl->theSplatter->setOpacity(255);

			string framename(utilities::spriteFrameNameForGlyphCode(glyph.getCode()));
			pImpl->theGlyph = CCSprite::createWithSpriteFrameName(framename.c_str());

			CC_BREAK_IF(!pImpl->theGlyph);
			this->addChild(pImpl->theGlyph);

			pImpl->glyph = glyph;

			ret = true;
		} while (false);
		return ret;
	}


	void BlockView::setCanvasView(BlockCanvasView *bcView)
	{
		pImpl->bcView = bcView;
	}


	// index refers to the order in the block chain
	void BlockView::setIndex(int index) {
		this->index = index;
	}
	
	
	void BlockView::setHintColor(const ccColor3B &color)
	{
		pImpl->theSplatter->setColor(color);
	}
	
	
	const ccColor3B &BlockView::getHintColor() const
	{
		return pImpl->theSplatter->getColor();
	}


	void BlockView::enableHint(bool enabled)
	{
		pImpl->theSplatter->setOpacity(enabled ? 255 : 0);
		pImpl->hintIsEnabled = enabled;

		pImpl->theBlock->setColor(enabled ? DarkColorTint : LightColorTint);
		pImpl->theBlock->setOpacity(enabled ? 255 : 50);
	}


	bool BlockView::isHintEnabled() const
	{
		return pImpl->hintIsEnabled;
	}


	void BlockView::onEnter()
	{

		CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
		CCSprite::onEnter();
	}


	void BlockView::onExit()
	{
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
		CCSprite::onExit();
	}
	
	
	void BlockView::setGlyph(const Glyph &glyph)
	{
		const char *sfn = utilities::spriteFrameNameForGlyphCode(glyph.getCode()).c_str();
		pImpl->theGlyph->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->
										 spriteFrameByName(sfn));

		pImpl->glyph = glyph;
	}


	const Glyph &BlockView::getGlyph() const
	{
		return pImpl->glyph;
	}


	void BlockView::showBlock(CCNode* sender, bool on)
	{
		pImpl->theGlyph->setOpacity(on ? 255 : 0);
	}
	
	
	void BlockView::showBlock()
	{
		pImpl->theGlyph->setOpacity(255);
	}
	
	
	void BlockView::hideBlock()
	{
		pImpl->theGlyph->setOpacity(0);
	}


	const CCSize &BlockView::blockSize() const
	{
		return pImpl->theBlock->getContentSize();
	}
	
	
	CCSprite *BlockView::getGlyphSprite() const
	{
		return pImpl->theGlyph;
	}


#pragma mark - CCTouchDispatcher


	bool BlockView::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
	{
		pImpl->lastTouchInsideBlock = touchEventInsideBlock(pTouch);

		// still need to know whether this is the block actually targetted
		if (pImpl->lastTouchInsideBlock) {

			// begin measuring touch hold time
			// LogD << "touch began on block with index " << this->getIndex();

			this->schedule(schedule_selector(BlockView::touchHoldCallback));

			if (hasObstruction()) {
				pImpl->tryRemoveObstruction(); // no promises here
			}

			return true;
		}
		return false;
	}


	void BlockView::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
	{
		const bool isTouchInsideBlock = touchEventInsideBlock(pTouch);
		// might have to track whether touch is moving out of block.

		if (isTouchInsideBlock && !pImpl->lastTouchInsideBlock) { // just entered

		} else if (!isTouchInsideBlock && pImpl->lastTouchInsideBlock) {
			this->unschedule(schedule_selector(BlockView::touchHoldCallback));
			// LogD << "touches moved on block with index " << this->getIndex() << ". Unscheduling selector.";
		}

		pImpl->lastTouchInsideBlock = isTouchInsideBlock;
	}


	void BlockView::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
	{
		pImpl->lastTouchInsideBlock = false;
		this->unschedule(schedule_selector(BlockView::touchHoldCallback));
		// LogD << "touches ended on block with index" << this->getIndex() << ". Unscheduling selector.";
	}


	void BlockView::touchHoldCallback(float deltaTime)
	{
		if (pImpl->touchHoldTimeCounter < TotalSecIceMeltTime) {
			pImpl->touchHoldTimeCounter += deltaTime;
			float ratio = pImpl->touchHoldTimeCounter / TotalSecIceMeltTime;
			// LogD << "touch hold timer for block now set to " << pImpl->touchHoldTimeCounter << ", ratio now at " <<
			// ratio;
			pImpl->setIceEncasementMeltRatio(ratio);

		} else {
			// LogD << "touch hold duration exceeds required melt time, unscheduling selector.";
			this->unschedule(schedule_selector(BlockView::touchHoldCallback));
		}
	}


#pragma mark - Obstructions (Le Froge)

	void BlockView::setHasObstruction(bool hasObstruction)
	{
		if (hasObstruction && !pImpl->hasObstruction) {
			addObstruction();
		} else if (!hasObstruction && pImpl->hasObstruction) {
			pImpl->theOverlay->removeFromParentAndCleanup(true);
			pImpl->theOverlay = NULL;
		}

		pImpl->hasObstruction = hasObstruction;
	}


	void BlockView::addObstruction()
	{
		if (!pImpl->theOverlay) {
			pImpl->theOverlay = CCSprite::createWithSpriteFrameName(OverlayFrameName);
			this->addChild(pImpl->theOverlay);
			pImpl->theOverlay->setVisible(false); // will be invisible until a moment after sliding in
		}

		pImpl->hasObstruction = true;
	}


	bool BlockView::hasObstruction() const
	{
		return pImpl->hasObstruction;
	}


	// this also schedules when the frog is going to disappear at some later point
	void BlockView::tryScheduleRevealObstruction()
	{
		if (!pImpl->obstructionHasBeenHidden) {

			// 3/4s of the way will start blinking (going between on and off)
			const size_t OnOffPairCount = 3;
			const float BlinkStartRatio = 0.5;
			const float BlinkStart(this->obstructionTimeDuration * BlinkStartRatio);
			const float BlinkInterval((this->obstructionTimeDuration * (1 - BlinkStartRatio)) / (2 * OnOffPairCount));

			auto show = MCBPlatformSupport::MCBCallLambda::create([=] {
				if (pImpl->theOverlay) { pImpl->theOverlay->setVisible(true); pImpl->theOverlay->setOpacity(255); }
			});

			auto hide = MCBPlatformSupport::MCBCallLambda::create([=] {
				if (pImpl->theOverlay) { pImpl->theOverlay->setOpacity(255 * 0.25); }
			});

			CCArray *sequence = CCArray::create();

			sequence->addObject(CCDelayTime::create(this->obstructionIntroductionDelay));
			sequence->addObject(show); // =============== being shown now

			sequence->addObject(CCDelayTime::create(BlinkStart));

			CCDelayTime *delayBlink = CCDelayTime::create(BlinkInterval);

			for (size_t i = 0; i < OnOffPairCount; i++) {
				sequence->addObject(hide);
				sequence->addObject(delayBlink);
				sequence->addObject(show);
				sequence->addObject(delayBlink);
			}
			sequence->addObject(hide);

			// final hide
			sequence->addObject(MCBPlatformSupport::MCBCallLambda::create([=] {
				if (pImpl->theOverlay) {
					pImpl->theOverlay->setVisible(false);
				}
				pImpl->obstructionHasBeenHidden = true;
			}));

			this->runAction(CCSequence::create(sequence));
		}
	}


	// make le froge jump to the upper left, disappear, and remove itself
	void BlockViewImpl::tryRemoveObstruction()
	{
		if (this->hasObstruction) {
			if (this->theOverlay->isVisible()) {

				blockView->stopAllActions();
				this->theOverlay->setOpacity(255);

				if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXRemoveFrog);
				}

				// will check state first (whether player is allowed to remove the obstruction)
				// theOverlay->setOpacity(255);

				const CCSize blockSz(theBlock->getContentSize());
				const CCPoint moveOffset(-0.75 * blockSz.width, 0.75 * blockSz.height);

				// animation group 1
				CCMoveBy *moveDiagUL1 = CCMoveBy::create(0.15, moveOffset);
				CCFiniteTimeAction *doubleScale = CCEaseIn::create(CCScaleTo::create(0.15, 1.5), 5);
				CCSpawn *group1 = CCSpawn::create(moveDiagUL1, doubleScale, NULL);

				// animation group 2
				CCMoveBy *moveDiagUL2 = CCMoveBy::create(0.15, moveOffset);
				CCFiniteTimeAction *normalScale = CCEaseIn::create(CCScaleTo::create(0.15, 1), 5);
				CCFadeTo *fadeTo20Pct = CCFadeTo::create(0.15, 255 * 20.0 / 100.0);
				CCSpawn *group2 = CCSpawn::create(moveDiagUL2, normalScale, fadeTo20Pct, NULL);

				auto remove = MCBPlatformSupport::MCBCallLambda::create([=] () {
					theOverlay->removeFromParentAndCleanup(true);
					this->theOverlay = NULL;

					// inform the BCV
					bcView->obstructionRemovedFromBlockView(blockView);
				});

				this->theOverlay->runAction(CCSequence::create(group1, group2, remove, NULL));
				this->hasObstruction = false;

			} else {
				LogI << "Obstruction not yet / no longer visible, can't remove";
			}
		}
	}


	void BlockViewImpl::setIceEncasementMeltRatio(float ratio)
	{
		// outer holds for meltratio 100-50, inner starts 50 goes down to 0
		float meltRatio = 1.0 - ratio; // going from 1.0 to 0.0 as you hold it

		const float IMVT = 0.25;
		// or "innerMeltVisibilityThreshold" - visibility will hit bottom at 10% before abruptly disappearing

		if (this->enclosureLevel == 2) {
			if (meltRatio >= 0.5) {
				this->theOuterEnclosure->setOpacity(255 * (meltRatio - 0.5) / 0.5);
				this->theInnerEnclosure->setOpacity(255);

			} else if (meltRatio > 0) {
				this->theOuterEnclosure->setOpacity(0);
				const float opacityFactor(IMVT + (meltRatio * 2) * (1 - IMVT)); // from 100% to 0% again
				this->theInnerEnclosure->setOpacity(255 * (opacityFactor));

			} else {

				bcView->attemptedDowngradeOfBlockEnclosureLevel(blockView);
				if (this->theOuterEnclosure) {
					this->theOuterEnclosure->removeFromParentAndCleanup(true);
					this->theOuterEnclosure = NULL;
				}
				if (this->theInnerEnclosure) {
					this->theInnerEnclosure->removeFromParentAndCleanup(true);
					this->theInnerEnclosure = NULL;
				}
			}
		}
	}


#pragma mark - Enclosures

	void BlockView::setEnclosureLevel(size_t level)
	{
		pImpl->enclosureLevel = level;

		switch (level) {
			case 0: // remove the sprites from the enclosures
			{
				if (pImpl->theOuterEnclosure) {
					pImpl->theOuterEnclosure->removeFromParentAndCleanup(true);
					pImpl->theOuterEnclosure = NULL;
				}

				if (pImpl->theInnerEnclosure) {
					pImpl->theInnerEnclosure->removeFromParentAndCleanup(true);
					pImpl->theInnerEnclosure = NULL;
				}

			}
			break;

			case 1: case 2: default:
			{
				// level 2 is two layers, outer and inner, where as level 1 is just inner

				if (!pImpl->theInnerEnclosure) {
					pImpl->theInnerEnclosure = CCSprite::createWithSpriteFrameName(SoftIceFrameName);
					this->addChild(pImpl->theInnerEnclosure);
				}

				if (!pImpl->theOuterEnclosure) {
					pImpl->theOuterEnclosure = CCSprite::createWithSpriteFrameName(HardIceFrameName);
					this->addChild(pImpl->theOuterEnclosure);
				}

//				if (level == 1) {
//					pImpl->theOuterEnclosure->setOpacity(0);
//				}

				// if reused, make sure the ff are reset
				this->unschedule(schedule_selector(BlockView::touchHoldCallback));
				pImpl->touchHoldTimeCounter = 0;
			}
			break;
		}
	}


	size_t BlockView::getEnclosureLevel()
	{
		return pImpl->enclosureLevel;
	}

	
#pragma mark - After some animation
	
	void BlockView::finishedPopAnimation(CCObject *obj)
	{
		// that obj should be the canvas view
		BlockCanvasView *canvasView = dynamic_cast<BlockCanvasView *>(obj);
		if (canvasView) {
			canvasView->recycleBlock(this);
		}
		// make it disappear
		this->setVisible(false);
	}

}