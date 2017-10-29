//
//  GlyphSet.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/11/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	This class will be used primarily by the typesetter for presenting chars

#pragma once
#include "Utilities.h"

namespace ac { namespace glyph {
	
	enum GlyphType
	{
		Inconsolata,
		Bello,
		Wilhelm,
		Trixie
		// and more
	};
	
	const char *FontFilePaths[] =
	{
		"fontatlases/inconsolata-recessed.fnt",
		"fontatlases/bello.fnt",
		"fontatlases/wilhelm.fnt",
		"fontatlases/trixie.fnt"
	};
	
	
	inline const char *fontFileForType(GlyphType type)
	{
		LogD4 << "The font file: " << string(FontFilePaths[type]);
		return FontFilePaths[type];
	}	
}}