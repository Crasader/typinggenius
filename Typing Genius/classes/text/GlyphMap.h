//
//  GlyphMap.h
//  Typing Genius
//
//  Created by Aldrich Co on 11/6/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include <map>
#include <vector>
#include <set>
#include "Utilities.h"
#include "Glyph.h"

namespace ac {

	using std::map;
	using std::string;
	using std::vector;
	using std::set;


	struct SpecialAbility
	{
		int startLevel;
		int cost;

		Glyph glyph;
		string glyphDescription;

		string abilityCode;
		string abilityDescription;
	};

	class GlyphMap
	{
	public:

		GlyphMap() {
			// don't call this until GameState is fully set up
			reset();
		}

		void reset();

		vector<Glyph> glyphsUsed(size_t level) const;

		void clear();

		void setGlyphToKeyLabel(const Glyph &, const string &);

		const Glyph &glyphForKeyLabel(const string &) const;

		const string &keyLabelForGlyph(const Glyph &) const;

		bool hasMapping(const string &) const;
		bool hasAltMapping(const string &keyLabel) const;

		void loadGlyphToKeyMappings(size_t playerLevel = 1);
		void regenerateHintColors();
		
		const utilities::RGBByte &hintColorForGlyph(const Glyph &) const;

		// can throw, uses std::map.at()
		const SpecialAbility &specialAbilityForKey(const string &keyLabel) const;

	private:
		map<string, Glyph> labelToGlyphMap; // the main map
		
		map<int, utilities::RGBByte> glyphHintColorMap;

		void setUpBoostTestKeyMappings();

		set<Glyph> newGlyphsThisLevel;

		map<string, SpecialAbility> specialAbilitiesMap;
	};
}