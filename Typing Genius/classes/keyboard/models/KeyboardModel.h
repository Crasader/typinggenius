//
//  KeyboardModel.h
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	A virtual black box that other components can send commands to (highlight this key), or listen to (react when some
//	key is pressed e.g., print, move the carraige position, etc)
//
//	It knows how to talk to the keys, their states, and their relative positions, too.
//

#pragma once

#include <map>
#include "MVC.h"
#include "Utilities.h"

namespace ac {

	using std::string;
	using std::unique_ptr;
	using std::map;
	using std::vector;
	using utilities::RGBByte;

	// forward declares
	class KeyboardModelImpl;
	enum class KeySpriteType;

	class Glyph;
	class GlyphMap;

	class ScoreKeeper;

	class KeyboardModel : public Model
	{
	public:
		
		/** ctors and dtors */
		KeyboardModel();
		~KeyboardModel();

		// toggle between normal and alt mode (using the Shift keys)
		void enableAltMode(bool inAltMode);
		bool isInAltMode() const;

		// have this be called once the others are read in
		void setupKeyMappings();
		
		// initiate a keypress event
		void keyTouchEvent(string, CCTouch *, TouchType);

		/** Keys Information: labels */
		size_t numberOfKeys() const;
		const vector<string> &getKeyLabels() const;
		void setKeyLabels(const vector<string> &labels);
		bool hasLabel(const string &label);
		
		/** Keyboard metadata */
		const string& getLabel() const;
		void setLabel(const string& label);
		const string& getDescription() const;
		void setDescription(const string& description);
		
		/** Keyboard and Key dimensions */
		// this is the default key size. it's possible to individually set the sizes of each key
		const KeySize getKeySize() const;
		void setKeySize(const KeySize &size);
		
		const KeyboardSize getKeyboardSize() const;
		void setKeyboardSize(const KeyboardSize &size);
		
		const KeyboardPoint& getKeyPosition(const string& label) const;
		void setKeyPosition(const string &label, KeyboardPoint &point);
		
		const KeySize getKeySize(const string& label) const; // if not found return 0,0 (you'd then have to use default)
		void setKeySize(const string &label, KeySize &size);

		// not used: what a key is supposed to represent
		const string getKeyDisplayLabel(const string& label) const;
		void setKeyDisplayLabel(const string &label, const string &displayLabel);

		// not used: a 'printable' is something that can be displayed. Any key can have 0-n printables
		const string &getPrintableFromLabel(const string &keyLabel);
		vector<string> &getPrintable(const string& keyLabel);
		void addPrintable(const string& keyLabel, const vector<string>& printables);
		void setPrintables(const map<string, vector<string>> &vecmap);

		const KeySpriteType &getKeySpriteType(const string &label) const;
		void setKeySpriteType(const string &label, const KeySpriteType &type);

		bool hasGlyphForKeyLabel(const string & label) const;
		const Glyph &getGlyphForKeyLabel(const string &label) const;
		void setGlyphForKeyLabel(const Glyph &code, const string &label);

		bool hasAltGlyphForKeyLabel(const string & label) const;
		const Glyph &getAltGlyphForKeyLabel(const string &label) const;
		void setAltGlyphForKeyLabel(const Glyph &code, const string &label);

		void setColorForKey(const RGBByte &, const string &);
		const RGBByte &getColorForKey(const string &label);

		void notifCallback(const string &code, std::shared_ptr<void> data);

	private:
		unique_ptr<KeyboardModelImpl> pImpl;
	};
}