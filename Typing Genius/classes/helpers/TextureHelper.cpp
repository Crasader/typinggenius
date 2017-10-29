//
//  TextureLoader.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/22/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "TextureHelper.h"


namespace ac { namespace utilities {

	const char *BlockSpriteSheetPlist = "spriteatlases/helvetiblox-blocks.plist";
	const char *BlockSpriteSheetTexture = "spriteatlases/helvetiblox-blocks.pvr.ccz";

	const char *KeySpriteSheetPlist = "spriteatlases/default-keyboard.plist";
	const char *KeySpriteSheetTexture = "spriteatlases/default-keyboard.pvr.ccz";

//	const char *KeySpriteSheetPlist = "spriteatlases/helvetiblox-blocks.plist";
//	const char *KeySpriteSheetTexture = "spriteatlases/helvetiblox-blocks.pvr.ccz";

	// unused
	const char *KeyLabelAvenirPlist = "spriteatlases/keylabels-avenir.plist";
	const char *KeyLabelAvenirTexture = "spriteatlases/keylabels-avenir.pvr.ccz";

	const char *KeyLabelHelvetibloxPlist = "spriteatlases/keylabels-helvetiblox.plist";
	const char *KeyLabelHelvetibloxTexture = "spriteatlases/keylabels-helvetiblox.pvr.ccz";


	void loadKeyboardTextures();
	void loadBlockCanvasTextures();
	void loadKeyLabelsTextures();
	

	void loadTextures()
	{
		loadKeyboardTextures();
		loadBlockCanvasTextures();
		loadKeyLabelsTextures();
	}


	void unloadTextures()
	{
		CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(BlockSpriteSheetPlist);
		CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(KeySpriteSheetPlist);
		CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFrameByName(KeyLabelAvenirPlist);
	}


	const char *textureForKeyboard()
	{
		return KeySpriteSheetTexture;
	}


	const char *textureForBlockCanvas()
	{
		return BlockSpriteSheetTexture;
	}


	const char *textureForKeyLabels()
	{
		return KeyLabelHelvetibloxTexture;
	}


#pragma mark - Private

	void loadKeyboardTextures()
	{
		CCSpriteFrameCache::sharedSpriteFrameCache()->
			addSpriteFramesWithFile(KeySpriteSheetPlist, KeySpriteSheetTexture);
	}


	void loadBlockCanvasTextures()
	{
		CCSpriteFrameCache::sharedSpriteFrameCache()->
			addSpriteFramesWithFile(BlockSpriteSheetPlist, BlockSpriteSheetTexture);
	}


	void loadKeyLabelsTextures()
	{
		CCSpriteFrameCache::sharedSpriteFrameCache()->
		addSpriteFramesWithFile(KeyLabelHelvetibloxPlist, KeyLabelHelvetibloxTexture);
	}
}}