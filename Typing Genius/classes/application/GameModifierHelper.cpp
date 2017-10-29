//
//  GameModifierHelper.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 1/13/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#include "GameModifierHelper.h"
#include "ACTypes.h"
#include "GlyphMap.h"
#include "Player.h"

namespace ac {
	
	using std::shared_ptr;
	using std::static_pointer_cast;
	
	const int SecondsToAddForFrogs = 10;
	
	void GameModifierHelper::notifCallback(const string &code, shared_ptr<void> data)
	{
		
		if ("CopyText_TriggerAltKey" == code) {
			
			shared_ptr<KeyEvent> kev = static_pointer_cast<KeyEvent>(data);
		
			// use GS to get what the key stands for...
			GlyphMap &gm(GameState::getInstance().glyphMap());
			
			if (gm.hasAltMapping(kev->key)) {
				const SpecialAbility &ability(gm.specialAbilityForKey(kev->key));
				
				// decide what to do with the ability
				if ("add_time" == ability.abilityCode) {
					performAddTimeSpecialAbility(ability.cost);
				}
				// handle the other abilities here
			}
			
		}
	}
	
	
	void GameModifierHelper::performAddTimeSpecialAbility(int cost)
	{
		if (gs().isGameStarted()) {
			if (player().getCurrencyAmount() >= cost) {
				player().deductCurrencyOwned(cost);
				gs().addTimer(SecondsToAddForFrogs);
			}
		}
	}
}