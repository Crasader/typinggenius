//
//  BlockTypesetter.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/14/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Looking to me more like an extension to the BlockCanvasView class.

//#include "BlockTypesetter.h"
#include "BlockModel.h"
#include "BlockCanvasModel.h"
#include "BlockCanvasView.h"
#include "cocos2d.h"
#include "BlockView.h"
#include <boost/algorithm/string.hpp>
#include <cctype>

namespace ac { namespace utilities {

	USING_NS_CC;
	
	string spriteFrameNameForGlyphCode(int glyphCode)
	{
		if (glyphCode == 0) return "empty.png";
		string ret = (boost::format("helvetiblox-%03d.png") % glyphCode).str();
		return ret;
	}

	
	string keyLabelToSpriteFrameName(const string &keyLabel)
	{
		// key:a --> A.png
		// special cases first

		if (keyLabel == "key:space") {
			return ""; // nothing (yet)
		}

		if (keyLabel == "key:,") {
			return "keylabel-comma.png";
		}

		if (keyLabel == "key:cr") {
			return "keylabel-carriage-return.png";
		}

		if (keyLabel == "key:period") {
			return "keylabel-period.png";
		}

		if (keyLabel == "key:backspace") {
			return "keylabel-backspace.png";
		}

		// rightOfColon: a, b, space, etc
		string rightOfColon = keyLabel.substr(keyLabel.find_last_of(":") + 1);
		if (rightOfColon.size() == 1) {
			return "keylabel-" + boost::to_upper_copy(rightOfColon)  + ".png";
		}

		return ""; // callers should check for this output
	}
	
}}