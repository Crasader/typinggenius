//
//  StatsHUD.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "StatsHUD.h"
#include "GameState.h"
#include "CopyText.h"
#include "BlockCanvasModel.h"
#include "BlockCanvas.h"
#include "ScoreKeeper.h"
#include "StatsHUDModel.h"
#include "StatsHUDView.h"

namespace ac {
	
#pragma mark - pImpl
	

#pragma mark - Singleton
	
	StatsHUD& StatsHUD::getInstance()
	{
		static StatsHUD instance;
		return instance;
	}
	

#pragma mark - Lifetime
	
	StatsHUD::StatsHUD()
	{
		LogI << boost::format("Inside StatsHUD constructor");
	}
	
	
	StatsHUD::~StatsHUD()
	{
		LogI << boost::format("Inside StatsHUD destructor");
	}

	
	void StatsHUD::setUp()
	{
		LogI << "Inside StatsHUD setup";
		model(); view();
	}
	
	
	void StatsHUD::tearDown()
	{
		LogI << "Inside StatsHUD teardown";
		
		deregisterSignals();
		
		setModel(nullptr);
		view()->release();
		setView(nullptr);
				
	}
}