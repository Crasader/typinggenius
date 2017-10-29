//
//  BlockCanvasView.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include "MVC.h"
#include "cocos2d.h"

namespace ac {

	USING_NS_CC;
	struct BlockCanvasViewImpl;
	class BlockView;
	class BlockCanvasModel;

	struct BlockCanvasViewUpdateInfo
	{
		int blockIndex;
		// ...
	};

	class BlockCanvasView : public View, public CCLayer
	{
	public:

		CREATE_FUNC(BlockCanvasView);
		virtual ~BlockCanvasView();

		virtual void foo() {};

		bool init();

		// this is the view (actually a batch sprite node) which will hold the stuff that is shown
		CCNode *getCanvasNode();

		// we plan to use these
		virtual void onEnter();
		virtual void onExit();

		// called by BlockView when it's done... needed?
		void recycleBlock(BlockView *block);

		// called by a BlockView when it's touched
		void attemptedDowngradeOfBlockEnclosureLevel(BlockView *block);

		/** 
		 *	@brief tell the model, through the view signal, that a (frog) obstruction has been removed.
		 *	@param block BlockView from which information like the block index in the chain can be gleaned.
		 */
		void obstructionRemovedFromBlockView(BlockView *block);

		// this is meant for the block canvas model who needs to know before any actual
		// block is shown
		static CCSize getIndividualBlockSize();
		static size_t blockCountPerRow();
		
		// tests need it so it's made public
		CCPoint blockPositionForBlock(int index, float blockWidth);

		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);
		
	private:
		BlockCanvasView();
		std::unique_ptr<BlockCanvasViewImpl> pImpl;
	};


}