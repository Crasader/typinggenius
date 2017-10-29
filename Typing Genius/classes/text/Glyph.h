//
//  Glyph.h
//  Typing Genius
//
//  Created by Aldrich Co on 11/3/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Also includes the GlyphString class

#pragma once

#include <set>
#include <map>


namespace ac {
	
	class Glyph
	{
	public:
		explicit Glyph() : Glyph(-1) {}
		explicit Glyph(int code) : Glyph(code, 1) {}
		explicit Glyph(int code, int level) : code(code), level(level) {	}
		explicit Glyph(const Glyph &other) : code(other.getCode()), level(other.getLevel()) {}
		inline int getCode() const { return code; }
		inline void setCode(int code) { this->code = code; }

		inline int getLevel() const { return level; }
		inline void setLevel(int level) { this->level = level; }

	private:
		int code;
		int level; // level when this glyph becomes available.
	};
	
	
	inline bool operator==(const Glyph &g1, const Glyph &g2)
	{
		return (g1.getCode() == g2.getCode() && g1.getLevel() == g2.getLevel());
	}

	inline bool operator!=(const Glyph &g1, const Glyph &g2)
	{
		return !(g1 == g2);
	}

	
	// maybe I should just inline all of these
	class GlyphString
	{
	public:
		GlyphString();
		
		GlyphString(const GlyphString &gsToCopy);
		GlyphString(const Glyph &glyph);
		
		const Glyph &operator[](size_t idx) const;
		GlyphString &operator=(const GlyphString& rhs);
		
		void clear();
		
		size_t size() const;

		void resize(size_t);

		GlyphString substr(size_t idx, size_t len) const;

		inline bool hasObstructionAtIndex(size_t index) const {
			return indicesWithObstructions.find(index) != indicesWithObstructions.end();
		}


		/** 
		 *	@brief answers the question as to whether a glyph has something "covering it up"... and to what extent.
		 *	@return 0 is nonexistent any other number represents the number of times the block has to be 'tapped'
		 */
		inline size_t encasementLevelAtIndex(size_t index) const { return indicesWithEncasements.at(index); }

		inline void reduceEncasementLevelAtIndex(size_t index, int amount) {
			size_t newEncasementLevel = 0;
			if ((int) indicesWithEncasements[index] > amount) {
				newEncasementLevel = indicesWithEncasements[index] - amount;
			}
			indicesWithEncasements[index] = newEncasementLevel;
		}
		
		void append(const Glyph &glyph);

		void append(const GlyphString &gs);

		// assistanceLevel at zero means "truly random" (not necessarily the hardest).
		// The larger the amount (positive), the more likely it is that a recognizable pattern can be generated.
		// The smaller the amount (negative), the less likely that randomly-generated patterns are generated
		void generateRandom(size_t requiredSize, std::vector<Glyph>& glyphsUsed,
							const std::vector<float> &repeatChances);

		// has to be regenerated on a level up
		void generateObstructions(); // should be called after generateRandom

		void generateEncasements(size_t requiredSize, size_t startOffset);

	private:
		std::vector<Glyph> vec;
		std::set<size_t> indicesWithObstructions;
		std::map<size_t, size_t> indicesWithEncasements; // index => encasement level

	};


	inline std::ostream& operator<<(std::ostream& out, const GlyphString& gs)
	{
		out << "[ ";
		for (int i = 0; i < gs.size(); ++i) {
			out << gs[i].getCode() << " ";
		}
		out << "] (length " << gs.size() << ")";
		return out;
	}

	inline bool operator==(const GlyphString &gs1, const GlyphString &gs2)
	{
		if (gs1.size() != gs2.size()) return false;

		for (int i = 0; i < gs1.size(); i++) {
			if (gs1[i].getCode() != gs2[i].getCode()) return false;
		}

		return true;
	}
	
	
	inline bool operator<(const Glyph &lhs, const Glyph &rhs)
	{
		return lhs.getCode() < rhs.getCode();
	}


	inline bool operator!=(const GlyphString &gs1, const GlyphString &gs2)
	{
		return !(gs1 == gs2);
	}
}
