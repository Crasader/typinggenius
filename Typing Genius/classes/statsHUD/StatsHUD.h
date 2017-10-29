//
//  StatsHUD.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "MVC.h"


namespace ac {
	
	class StatsHUDModel;
	class StatsHUDView;
	
	class StatsHUD : public Controller<StatsHUDModel, StatsHUDView, void>
	{
	public:
		
		static StatsHUD& getInstance();
		~StatsHUD();
		
		virtual void setUp();
		virtual void tearDown();
		
	private:
		
		StatsHUD(); //  use the singleton instead
		
		// noncopyable
		StatsHUD(const StatsHUD &);
		StatsHUD &operator=(const StatsHUD &);
		
		// pImpl
		// unique_ptr<StatsHUDImpl> pImpl;
	};
	
}