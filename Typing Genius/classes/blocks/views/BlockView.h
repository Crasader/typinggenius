//
//  BlockView.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Thse are to be directly added to a Batch Sprite Node so should be a sprite with the nec. graphics
//	This is a Sprite Container Sprite.

#pragma once

#include "cocos2d.h"

namespace ac {

	USING_NS_CC;

	class Glyph;
	struct BlockViewImpl;
	class BlockCanvasView;

	class BlockView : public CCSprite, public CCTouchDelegate
	{
	public:

		static BlockView *createWithGlyph(const Glyph &glyph);

		bool initWithGlyph(const Glyph &glyph);

		void setCanvasView(BlockCanvasView *);

		// used to mark a block as incorrect during an animation.
		void showBlock(CCNode* sender, bool on);
		void showBlock();
		void hideBlock();

		const CCSize &blockSize() const;
		
		void finishedPopAnimation(CCObject *obj);
		
		void setGlyph(const Glyph &code);
		const Glyph &getGlyph() const;
		
		inline bool isAnimating() const { return this->animating; }
		inline void setIsAnimating(bool animating) { this->animating = animating; }
		
		void setIndex(int index);
		inline int getIndex() { return this->index; }
		
		void setHintColor(const ccColor3B &);
		const ccColor3B &getHintColor() const;

		void enableHint(bool enable = true);
		bool isHintEnabled() const;

		/** 
		 *	@brief such as a frog. I wonder if BlockView has much of a say on this? Because stuff get added to
		 *	a sprite batch node, not to it. Or should the batch node be passed to BlockView instead?
		 */
		void setHasObstruction(bool);
		bool hasObstruction() const; // may be true but could be hidden, and therefore not be removed.
		void addObstruction();

		void tryScheduleRevealObstruction();

		inline float getObstructionIntroductionDelay() const {
			return this->obstructionIntroductionDelay;
		}

		inline float getObstructionTimeDuration() const {
			return this->obstructionTimeDuration;
		}

		inline void setObstructionTimeDuration(float duration) {
			this->obstructionTimeDuration = duration;
		}

		inline void setObstructionIntroductionDelay(float delay) {
			this->obstructionIntroductionDelay = delay;
		}

		void setEnclosureLevel(size_t level);
		size_t getEnclosureLevel();


		virtual bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
		virtual void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent);
		virtual void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);

		// this called for as long as block is held
		virtual void touchHoldCallback(float deltaTime);

		inline ccColor3B colorFromIndex(int index) {
			switch (index) {
				case -1:
				case 0: return ccc3(255, 94, 58);
				case 1: return ccc3(255, 84, 67);
				case 2: return ccc3(255, 73, 76);
				case 3: return ccc3(255, 65, 86);
				case 4: return ccc3(255, 52, 95);
				default: return ccc3(255, 42, 104);
			}
		}
		
		
		CCSprite *getGlyphSprite() const;

	private:
		BlockView();
		virtual ~BlockView();

		virtual void onEnter();
		virtual void onExit();
		
		int index;
		
		bool animating;


		/**
		 *	after sliding in, how long before the frog shows up
		 */
		float obstructionIntroductionDelay;

		/**
		 *	time after being shown before an obstruction is removed (and unavailable to collect)
		 */
		float obstructionTimeDuration;

		// non-copyable
		BlockView(const BlockView &);
		BlockView &operator=(const BlockView &);

		std::unique_ptr<BlockViewImpl> pImpl;


		inline CCRect myBounds() {
			const CCSize blockSz(this->blockSize());
			return {
				// because the anchor point is in (0.5, 0.5)
				-blockSz.width / 2, -blockSz.height / 2,
				blockSz.width, blockSz.height
			};
		}


		inline bool touchEventInsideBlock(CCTouch *pTouch) {
			return myBounds().containsPoint(this->convertToNodeSpace(CCDirector::sharedDirector()->
																  convertToGL(pTouch->getLocationInView())));
		}
	};

	
	inline std::ostream& operator<<(std::ostream& out, BlockView& bv)
	{
		out << bv.getIndex();
		return out;
	}
}