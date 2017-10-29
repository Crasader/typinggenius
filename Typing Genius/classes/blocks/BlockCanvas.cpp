//
//  BlockCanvas.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockCanvas.h"
#include "CopyText.h"
#include "GameState.h"
#include "BlockCanvasModel.h"
#include "BlockCanvasView.h"
#include "ScoreKeeper.h"
#include "StatsHUD.h"
#include "StatsHUDModel.h"

namespace ac {


#pragma mark - Singleton

	BlockCanvas &BlockCanvas::getInstance()
	{
		static BlockCanvas instance;
		return instance;
	}


#pragma mark - Lifetime

	void BlockCanvas::setUp()
	{
		model(); view();
	}


	void BlockCanvas::tearDown()
	{
		this->setModel(nullptr);
		this->view()->release();
		this->setView(nullptr);
		deregisterSignals();
	}

}
