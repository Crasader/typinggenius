//
//  BlockCanvas.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include "MVC.h"

namespace ac {

	class BlockCanvasView;
	class BlockCanvasModel;

	class BlockCanvas : public Controller<BlockCanvasModel, BlockCanvasView, void>
	{
	public:
		static BlockCanvas &getInstance();

		virtual void setUp();
		virtual void tearDown();

	private:

		/** ctors and dtors */
		BlockCanvas() {}

		// noncopyable
		BlockCanvas(const BlockCanvas &);
		BlockCanvas &operator=(const BlockCanvas &);
	};
	
}