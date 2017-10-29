//
//  TextureHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "cocos2d.h"

namespace ac { namespace utilities {

	USING_NS_CC;

	// safe to call multiple times, it seems
	void loadTextures();
	void unloadTextures();

	const char *textureForKeyboard();
	const char *textureForBlockCanvas();
	const char *textureForKeyLabels();
}}