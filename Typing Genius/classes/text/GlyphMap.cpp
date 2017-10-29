//
//  GlyphMap.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/6/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/foreach.hpp>
#include "GlyphMap.h"
#include "GameState.h"
#include "BoostPTreeHelper.h"
#include "Player.h"
#include "PlayerLevel.h"
#include "Glyph.h"

namespace ac {

	const std::string GlyphMappingConfigurationFileName = "glyphmap/DefaultGlyphMapConfig.json";

	void GlyphMap::reset()
	{
		clear();
#if BOOST_TEST_TARGET
		setUpBoostTestKeyMappings();
#else
		loadGlyphToKeyMappings(); // from JSON
#endif
		regenerateHintColors();
	}


	vector<Glyph> GlyphMap::glyphsUsed(size_t level) const
	{
		vector<Glyph> ret;
		for (const auto &kv : labelToGlyphMap) {
			if (kv.second.getLevel() <= level) {
				ret.push_back(kv.second);
			}
		}
		return ret;
	}


	void GlyphMap::clear()
	{
		labelToGlyphMap.clear();
		specialAbilitiesMap.clear();
	}


	void GlyphMap::setGlyphToKeyLabel(const Glyph &glyph, const string &label)
	{
		labelToGlyphMap[label] = glyph;
	}


	const Glyph &GlyphMap::glyphForKeyLabel(const string &label) const
	{
		return labelToGlyphMap.at(label);
	}


	const SpecialAbility &GlyphMap::specialAbilityForKey(const string &keyLabel) const
	{
		return specialAbilitiesMap.at(keyLabel);
	}


	// reverse lookup
	const string &GlyphMap::keyLabelForGlyph(const Glyph &glyph) const
	{
		static string Empty("");
		// traverse the map
		for (const auto &pair : labelToGlyphMap) {
			if (glyph.getCode() == pair.second.getCode()) {
				return pair.first;
			}
		}

		return Empty;
	}



	bool GlyphMap::hasMapping(const string &keyLabel) const
	{
		return labelToGlyphMap.count(keyLabel) > 0;
	}


	bool GlyphMap::hasAltMapping(const string &keyLabel) const
	{
		return specialAbilitiesMap.count(keyLabel) > 0;
	}
	
	
	void GlyphMap::regenerateHintColors()
	{
		glyphHintColorMap.clear();
		utilities::RGBByte randomRGB { 0,0,0 };

		// per Glyph found, assign it a random color
		for (auto &kv : labelToGlyphMap) {

			if (randomRGB.r == 0 && randomRGB.g == 0 && randomRGB.b == 0) {
				// randomRGB = utilities::randomRGBByte();
				randomRGB.r = 52, randomRGB.g = 60, randomRGB.b = 125;
			} else {
				randomRGB = utilities::nextRGBByteHueShift(randomRGB, 360 / labelToGlyphMap.size());
			}
			
			int code = kv.second.getCode();
			glyphHintColorMap[code] = randomRGB;
			LogI << boost::format("code: %d gets assigned rgb(%d, %d, %d)") % kv.second.getCode() %
				(int)randomRGB.r % (int)randomRGB.g % (int)randomRGB.b;
		}
	}
	
	
	const utilities::RGBByte &GlyphMap::hintColorForGlyph(const Glyph &glyph) const
	{
		static utilities::RGBByte dummyRGB { 255, 255, 255 };
		try {
			const utilities::RGBByte &ret(glyphHintColorMap.at(glyph.getCode()));
			return ret;
		} catch (std::out_of_range) {
			return dummyRGB;
		}
	}


	void GlyphMap::setUpBoostTestKeyMappings()
	{
		boost::format fmt("key:%03d");
		for (size_t i = 1; i <= 30; i++) {
			fmt % i;
			string keyLabel = fmt.str();
			setGlyphToKeyLabel(Glyph(i), keyLabel);
		}
		// space bar
		setGlyphToKeyLabel(Glyph(31), "key:000");
	}


	/**
	 * @brief this method pulls the glyph to key mappings from a json file (see
	 * GlyphMappingConfigurationFileName)
	 */
	void GlyphMap::loadGlyphToKeyMappings(size_t playerLevel)
	{
		using utilities::PropTree;
		using utilities::PropTreeValType;

		labelToGlyphMap.clear();

		// read from GlyphMappingConfigurationFileName
		LogI << "Initializing GlyphMap configuration, with file to be loaded: " <<
					GlyphMappingConfigurationFileName;
		PropTree pt = utilities::getPropertyTreeFromJSONFileBundle(GlyphMappingConfigurationFileName);

		// in the case of a unit test
		if (pt.empty()) {
			LogE << "PropTree is empty. Adding sample glyphs";
			setGlyphToKeyLabel(Glyph(86), "key:012");
			setGlyphToKeyLabel(Glyph(98), "key:013");
		}

		// mappings->row_X->key:XXX->{glyphCode/startLevel}
		PropTree &mappingsRoot = pt.get_child("mappings");
		BOOST_FOREACH(PropTreeValType &v1, mappingsRoot) {
			PropTree &perRowRoot = pt.get_child("mappings." + v1.first);
			BOOST_FOREACH(PropTreeValType &v2, perRowRoot) {

				// glyphCode = 0 is reserved for the space bar (Empty)
				int glyphCode = pt.get<int>("mappings." + v1.first + "." + v2.first + ".glyphcode", 0);
				int startLevel = pt.get<int>("mappings." + v1.first + "." + v2.first + ".startlevel", 1);

				if (glyphCode >= 0 /*&& playerLevel >= startLevel*/) {
					LogD2 << boost::format("Key: %s gets Glyph Code %d at level %d") % v2.first % glyphCode % startLevel;
					Glyph g(glyphCode, startLevel);
					setGlyphToKeyLabel(g, v2.first);
				}
			}
		}

		specialAbilitiesMap.clear();

		// grab any special power mappings (alt mode)
		PropTree &perSpecialAbilitiesRoot = pt.get_child("special_abilities");
		for (const auto &kv : perSpecialAbilitiesRoot) {
			const string path("special_abilities." + kv.first + ".");

			SpecialAbility ability;

			ability.startLevel = pt.get<int>(path + "startlevel");
			ability.glyph = Glyph(pt.get<int>(path + "glyphcode"), ability.startLevel);
			ability.cost = pt.get<int>(path + "cost");
			ability.glyphDescription = pt.get<string>(path + "glyphdescription");
			ability.abilityCode = pt.get<string>(path + "abilitycode");
			ability.abilityDescription = pt.get<string>(path + "abilitydescription");

			// set to a map (key => SA)
			specialAbilitiesMap[kv.first] = ability;
		}
	}
}
