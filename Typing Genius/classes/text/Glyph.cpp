//
//  Glyph.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/3/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "Glyph.h"
#include "Utilities.h"

namespace ac {
	
	// just a utility that only makes sense in this file
	bool intVecIsAllTheSame(const std::vector<int> &);
	
	GlyphString::GlyphString()
	{
		// constructor
	}
	
	
	GlyphString::GlyphString(const GlyphString &gsToCopy)
	{
		clear();
		for (int i = 0; i < gsToCopy.size(); i++) {
			append(gsToCopy[i]);
		}
	}


	GlyphString::GlyphString(const Glyph &glyph)
	{
		clear();
		append(glyph);
	}
	
	
	GlyphString &GlyphString::operator=(const GlyphString& rhs)
	{
		clear();
		for (int i = 0; i < rhs.size(); i++) {
			append(rhs[i]);
		}
		return *this;
	}
	
	
	const Glyph& GlyphString::operator[](size_t idx) const
	{
		if (idx >= size()) {
			throw std::out_of_range("idx too large");
		}
		return vec[idx];
	}
	
	
	void GlyphString::clear()
	{
		vec.clear();
	}
	
	
	size_t GlyphString::size() const
	{
		return vec.size();
	}
	
	
	void GlyphString::append(const Glyph &glyph)
	{
		vec.push_back(glyph); // uses implicit conversion
	}
	
	
	void GlyphString::append(const GlyphString &gs)
	{
		for (int i = 0; i < gs.size(); i++) {
			vec.push_back(gs[i]);
		}
	}
	
	
	// you have to throw an exception if idx >= size()
	GlyphString GlyphString::substr(size_t idx, size_t len) const
	{
		if (idx >= size()) {
			throw std::out_of_range("idx too large");
		}
		
		GlyphString ret;
		// this creates a new GlyphString
		if (idx + len > vec.size()) {
			// to the end of the string
			len = vec.size() - idx;
		}
		
		for (size_t i = idx; i < idx + len; i++) {
			ret.append(vec[i]);
		}
		
		return ret;
	}


	// also takes care of obstructions
	void GlyphString::generateRandom(size_t requiredSize, std::vector<Glyph>& glyphsUsed,
			const std::vector<float> &repeatChances)
	{
		this->clear();

		if (glyphsUsed.size() < 1) {
			LogW << "No glyphs assigned to keys yet. Try to load mappings first";
			return;
		}

		bool spaceWillBeUsed(false);
		std::vector<Glyph> actualGlyphsUsed; // will exclude the space if found
		for (const Glyph &g : glyphsUsed) {
			if (g.getCode() == 0) {
				spaceWillBeUsed = true;
			} else {
				actualGlyphsUsed.push_back(g);
			}
		}

		// distribution over the set of glyphs (not including the space)
		boost::random::uniform_int_distribution<> rGlyphCode(0, actualGlyphsUsed.size() - 1);
		
		// note: a space (glyphCode=0) can never repeat, and has its own probability.
		const float chanceOfSpace = 0.2f;
		bool spaceLastAdded = false;

		// one rule to note: you can't repeat if the last one is a repeat
		size_t lastRepeatOrdinal = 0;
		while (vec.size() < requiredSize) {


			if (spaceWillBeUsed && !spaceLastAdded && utilities::randomChance(chanceOfSpace)) {
				vec.push_back(Glyph(0));
				spaceLastAdded = true;
			} else {

				// over the actualGlyphsUsed
				int nextGlyphCode = actualGlyphsUsed[rGlyphCode(utilities::rng)].getCode();

				bool hasRepeated = false;
				for (size_t i = 0; i < repeatChances.size(); i++) {

					bool spaceFoundInRepeat = false; // abort repeat if found

					// check if the previous `repeatChances.size()` glyphs in vec is zero.
					if (vec.size() > i) {
						// for (size_t j = 0; j < i + 1; j++) { // 0..i+1
						for (int j = i+1; j >= 0; j--) {// i+1..0
							if (0 ==  vec[vec.size() - (i + 1)].getCode()) {
								spaceFoundInRepeat = true;
								break;
							}
						}
					}


					// vec.size() is the amount that's been added to it by far. It's required for repeats
					if (!spaceFoundInRepeat && vec.size() > i && utilities::randomChance(repeatChances[i]) &&
						i >= lastRepeatOrdinal) {

						// collect the glyphs to be added first
						std::vector<int> glyphCodesToAdd;

						for (size_t j = 0; j < i + 1; j++) {
							nextGlyphCode = vec[vec.size() - (i + 1)].getCode();
							glyphCodesToAdd.push_back(nextGlyphCode);
						}

						// impose a 'loose' uniqueness requirement: for i > 0 if a set is all the same
						// glyphcode don't do it
						if (i == 0 || !intVecIsAllTheSame(glyphCodesToAdd)) {

							for (int glyphCode : glyphCodesToAdd) {
								vec.push_back(Glyph(glyphCode));
								// std::cout << "appending " << glyphCode << std::endl;
							}

							// std::cout << ".. repeated x" << i + 1 << std::endl;
							hasRepeated = true;
							lastRepeatOrdinal = i + 1;
							break;

						} else {
							// std::cout << "No append... set is all the same (no good)" << std::endl;
						}
					}
				}

				// no repeat: use the randomly-generated code
				if (!hasRepeated) {
					// Note: every repeat x 1 is deliberate, and can't happen as a result of randomness!
					// don't push if it contains the same as the last -- force it to be different
					if (vec.size() == 0 || vec[vec.size() - 1].getCode() != nextGlyphCode) {
						vec.push_back(Glyph(nextGlyphCode));
						// std::cout << "appending " << nextGlyphCode << " (no repeat)" << std::endl;
						lastRepeatOrdinal = 0;
					} else {
						// std::cout << "No append... trying to repeat last by accident" << std::endl;
					}
				}

				spaceLastAdded = false;
			}
		}

		// in case repeats cause vec to grow larger than size
		vec.resize(requiredSize);
	}


	void GlyphString::generateObstructions()
	{
		// 0 to startOffset -1, if that exists
		indicesWithObstructions.clear();
		// group them into 50s.
		const size_t groupSize = 50;

		// each element represents a chance happening over a group of 50. Last element covers all glyphs beyond that
		// const std::vector<float> chancesOfObstruction = { 0.25 };
		const std::vector<float> chancesOfObstruction = { 0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3 };

		for (size_t i = 0; i < vec.size(); i++) {
			if (vec[i].getCode() > 0) { // just need to be a nonspace glyph
				size_t groupIndex = MIN(i / groupSize, chancesOfObstruction.size() - 1);
				if (utilities::randomChance(chancesOfObstruction[groupIndex])) {
					this->indicesWithObstructions.insert(i);
					// LogD << "obstruction added at index: " << i;
				}
			}
		}
	}


	/** 
	 *	@brief this is called after generate obstructions, but no encasings will be added on blocks with obstructions
	 */
	void GlyphString::generateEncasements(size_t requiredSize, size_t startOffset)
	{
		// keep all values: copy to temp up to [startOffset - 1]

//		// AC 2013.12.21: for debugging purposes
//		for (size_t i = 0; i < requiredSize; i++) {
//			this->indicesWithEncasements[i] = 0;
//		}
//		this->indicesWithEncasements[0] = 2;
//		this->indicesWithEncasements[2] = 2;
//		return;

		// const std::vector<float> chancesOfEncasements = { 1 };
		const std::vector<float> chancesOfEncasements = { 0, 0, 0.15, 0.18, 0.2, 0.25, 0.33 };

		std::map<size_t, size_t> copyOfEncasements;

		// can we expect that this traversal happens in an expected order?
		for (const auto &pair: indicesWithEncasements) {
			if (pair.first < startOffset) {
				copyOfEncasements[pair.first] = pair.second;
			}
		}

		// group them into 50s. Each element represents a chance happening over a group of 50.
		// Last element covers all glyphs beyond that
		const size_t groupSize = 50;
		boost::random::uniform_int_distribution<> rZeroTo99(0, 99);



		for (size_t i = startOffset; i < requiredSize; i++) {
			copyOfEncasements[i] = 0;
			if (vec[i].getCode() > 0 && !hasObstructionAtIndex(i)) { // just need to be a nonspace glyph
				size_t groupIndex = MIN(i / groupSize, chancesOfEncasements.size() - 1);
				if (utilities::randomChance(chancesOfEncasements[groupIndex])) {
					// determine level
					const size_t level = 2;
					copyOfEncasements[i] = level;
					if (i < 200) {
						LogD << "encasement added at index: " << i << " with level: " << level;
					}
				}
			}
		}

		this->indicesWithEncasements = copyOfEncasements;
	}


	// note: if requiredSize is larger than current, will fill it with zeros.
	void GlyphString::resize(size_t requiredSize)
	{
		vec.resize(requiredSize);
	}


#pragma - Miscellaneous

	bool intVecIsAllTheSame(const std::vector<int> &vec)
	{
		std::set<int> uniqueSet;
		for (int i : vec) {
			uniqueSet.insert(i);
		}
		return uniqueSet.size() == 1;
	}
}

