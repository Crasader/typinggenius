//
//  GameModifierHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 1/13/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#ifndef __Typing_Genius__GameModifierHelper__
#define __Typing_Genius__GameModifierHelper__

#include "Notif.h"
#include "GameState.h"


namespace ac {

	class GameModifierHelper : public NotifListener
	{
	public:
		void notifCallback(const string &, std::shared_ptr<void> data);
	
		inline GameState &gs() const { return GameState::getInstance(); }
		inline Player &player() const { return gs().player(); }
		void performAddTimeSpecialAbility(int cost = 0);
		
	};

	
	struct AbilityActivationInfo
	{
		string abilityCode;
		int cost;
	};
	
}

#endif /* defined(__Typing_Genius__GameModifierHelper__) */
