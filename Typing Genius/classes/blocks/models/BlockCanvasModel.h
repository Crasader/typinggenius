//
//  BlockCanvasModel.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Should be thought of as a string of Glyphs.

#pragma once

#include "MVC.h"

namespace ac {

	class CopyText;
	class GlyphString;
	class ScoreKeeper;
	class BlockChain;
	
	struct BlockCanvasModelUpdateInfo
	{
		int scoreUpdateDelta;
		int curStreakLevel;
	};

	struct BlockCanvasModelImpl;

	// random obstruction delay between 0.3 to 0.8 seconds
	const float minObstructionDelay = 0.3;
	const float maxObstructionDelay = 0.8;
	const float obstructionDuration = 1.8;


	class BlockCanvasModel : public Model
	{
	public:
		BlockCanvasModel();
		~BlockCanvasModel();

		BlockChain &getBlockChain();

		// this sets the block chain contents with what's
		// in str.
		void updateBlockChain(const GlyphString &str);

		// convenience methods in relation to CopyText and checking.
		size_t unitsToAdvance() const; // after checking, how many blocks should go?
		size_t unitsWithMistake() const;
		size_t totalBlocksDisplayable() const; // the most number of blocks that can be displayed regardless of copy string
		size_t visibleStringSize() const; // depends on the remaining string size
		
	
		// returns in seconds
		float slideBackTime() const;

		/**
		 *	@brief tell GameState that a "frog has been collected".
		 */
		void reportObstructionRemoved();

		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);

	private:
		std::unique_ptr<BlockCanvasModelImpl> pImpl;
	};
	
}