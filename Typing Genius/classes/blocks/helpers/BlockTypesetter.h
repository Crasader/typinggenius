//
//  BlockTypesetter.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/14/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Works mostly as a helper / surrogate of BlockCanvasView, but is not part of it.

#pragma once

#include "cocos2d.h"

namespace ac {
	
	class BlockChain;
	class BlockCanvasModel;
	class BlockCanvasView;
	class BlockView;
	
	namespace utilities {

		// note: none of these methods, with the exception of spriteFrameNameForGlyphCode, is a good candidate
		// to be in the typesetter. Move them back to BlockCanvasView.
		
		USING_NS_CC;
		
		using std::string;
		
		// this will map a standard key value "a,b,c..." into the necessary spriteframename
		// not the best lookup system in place as of the moment
		string spriteFrameNameForGlyphCode(int glyphCode);


		string keyLabelToSpriteFrameName(const string &keyLabel);


		inline string keyGlyphCodeToSpriteFrameName(int code)
		{
			if (code < 0) {
				return "";
			}
			string ret = (boost::format("key-helvetiblox-%03d.png") % code).str();
			return ret;
		}
	}
}