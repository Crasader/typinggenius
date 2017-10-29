//
//  KeyboardModel.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "KeyboardModel.h"
#include "DebugSettingsHelper.h"
#include "Keyboard.h"
#include "CopyText.h"
#include "KeypressTracker.h"
#include "GlyphMap.h"
#include "KeyModel.h"
#include "GameState.h"
#include "Utilities.h"
#include "ScoreKeeper.h"
#include "KeyboardView.h"
#include "Player.h"

namespace ac {
	
	using namespace std;

	
#pragma mark - pImpl Structure

	struct KeyboardModelImpl
	{
		KeyboardModelImpl(KeyboardModel &kbModel) :
		label(), description(), keysize(), keyboardSize(), printablesMap(), keyPositionsMap(),
		keyAltGlyphCodes(), 	customKeySizes(), touchEventsConnection(), keyGlyphCodes(), kbModel(kbModel),
		keyColores(), altMode(false)
		{
			// LogD << "Inside KeyboardImpl constructor";
		}
		
		~KeyboardModelImpl() {
			// LogD << "Inside KeyboardImpl destructor";
		}

		KeyboardModel& kbModel;

		/** unique short identifier of the configuration  */
		string label;
		
		/** a longer description */
		string description;
		
		/** base size of the keys. Some could be larger than this */
		KeySize keysize;
		
		KeyboardSize keyboardSize;
		
		/** what can be printed when this key is struck. */
		map<string, vector<string>> printablesMap;
		
		map<string, KeyboardPoint> keyPositionsMap;
		
		map<string, KeySize> customKeySizes;

		map<string, KeySpriteType> customKeySpriteTypes;
		
		/** K=>key code, V=>Key Model */
		map<string, KeyModel> keyMap;
		
		map<string, string> keyDisplayLabelsMap;
		
		vector<string> keyLabels;

		map<string, Glyph> keyGlyphCodes;
		map<string, Glyph> keyAltGlyphCodes;

		map<string, utilities::RGBByte> keyColores;
		
		sign_conn_t touchEventsConnection; // from KeyboardView

		bool altMode;

		// have the model call this up when all the rest have been read in
		void setUpKeysFromGlyphMap() {
			
			GlyphMap &glyphMap(GameState::getInstance().glyphMap());
			
			for (const auto &label : keyLabels) {
				if (glyphMap.hasMapping(label)) {
					const Glyph &g(glyphMap.glyphForKeyLabel(label));
					kbModel.setGlyphForKeyLabel(g, label);
				}

				// alt glyphs
				if (glyphMap.hasAltMapping(label)) {
					const SpecialAbility &specialAbility(glyphMap.specialAbilityForKey(label));
					kbModel.setAltGlyphForKeyLabel(specialAbility.glyph, label);
				}
			}
		}
	};
	
	
#pragma mark - Ctors and Dtors


	KeyboardModel::KeyboardModel()
	{
		pImpl.reset(new KeyboardModelImpl(*this));
		LogD << "KeyboardModel constructor";
	}
	
	
	KeyboardModel::~KeyboardModel()
	{
		LogD << "KeyboardModel destructor";
	}


	void KeyboardModel::setupKeyMappings()
	{
		pImpl->setUpKeysFromGlyphMap();
	}


#pragma mark - Keyboard Metadata

	const string& KeyboardModel::getLabel() const
	{
		return pImpl->label;
	}
	
	
	void KeyboardModel::setLabel(const string& label)
	{
		pImpl->label = label;
	}
	
	
	const string& KeyboardModel::getDescription() const
	{
		return pImpl->description;
	}
	
	
	void KeyboardModel::setDescription(const string& description)
	{
		pImpl->description = description;
	}


	void KeyboardModel::enableAltMode(bool inAltMode)
	{
		pImpl->altMode = inAltMode;
		// primarily intended for the KBView
		Notif::send("KeyboardModel_AltModeToggled", std::make_shared<bool>(pImpl->altMode));
	}


	bool KeyboardModel::isInAltMode() const
	{
		return pImpl->altMode;
	}
	
	
#pragma mark - Keyboard and Key Dimensions
	
	const KeySize KeyboardModel::getKeySize() const
	{
		return pImpl->keysize;
	}
	
	
	void KeyboardModel::setKeySize(const KeySize &size)
	{
		pImpl->keysize = size;
	}
	
	
	const KeyboardSize KeyboardModel::getKeyboardSize() const
	{
		return pImpl->keyboardSize;
	}
	
	
	void KeyboardModel::setKeyboardSize(const KeyboardSize &size)
	{
		pImpl->keyboardSize = size;
	}
	
	
	const KeyboardPoint& KeyboardModel::getKeyPosition(const string& label) const
	{
		return pImpl->keyPositionsMap[label];
	}
	
	
	void KeyboardModel::setKeyPosition(const string &label, KeyboardPoint &point)
	{
		pImpl->keyPositionsMap[label] = point;
	}
	
	
	const KeySize KeyboardModel::getKeySize(const string& label) const
	{
		try {
			KeySize sz = pImpl->customKeySizes.at(label);
			if (sz.width == 0.0f && sz.height == 0.0f) { //not sure why at isn't throwing exceptions
				return getKeySize();
			}
			return sz;
		} catch (std::out_of_range exception) {
			return getKeySize();
		}
	}
	
	
	void KeyboardModel::setKeySize(const string &label, KeySize &size)
	{
		pImpl->customKeySizes[label] = size;
	}
	
	
#pragma mark - Key Informations (Labels & Printables)

	const KeySpriteType &KeyboardModel::getKeySpriteType(const string &label) const
	{
		return pImpl->customKeySpriteTypes[label];
	}


	void KeyboardModel::setKeySpriteType(const string &label, const KeySpriteType &type)
	{
		pImpl->customKeySpriteTypes[label] = type;
	}

	
	size_t KeyboardModel::numberOfKeys() const
	{
		return pImpl->keyLabels.size();
	}
	
	const vector<string> &KeyboardModel::getKeyLabels() const
	{
		return pImpl->keyLabels;
	}
	
	
	void KeyboardModel::setKeyLabels(const vector<string> &labels)
	{
		pImpl->keyLabels = labels;
	}
	
	
	bool KeyboardModel::hasLabel(const string &label)
	{
		return std::find(pImpl->keyLabels.begin(), pImpl->keyLabels.end(), label) != pImpl->keyLabels.end();
	}
	
	// how to get the printable from the label
	const string &KeyboardModel::getPrintableFromLabel(const string &label)
	{
		static string Empty("");
		vector<string> &printables(getPrintable(label));
		if (printables.size() == 0) {
			return Empty;
		}
		return printables[0];
	}
	
	
	void KeyboardModel::setPrintables(const map<string, vector<string>> &vecmap)
	{
		pImpl->printablesMap = vecmap;
	}
	
	
	// a 'printable' is something that can be displayed. Any key can
	// have 0-n printables
	vector<string> &KeyboardModel::getPrintable(const string& keyLabel)
	{
		return pImpl->printablesMap[keyLabel];
	}
	
	
	void KeyboardModel::addPrintable(const string& keyLabel, const vector<string>& printables)
	{
		pImpl->printablesMap[keyLabel] = printables;
	}
	
	
	const string KeyboardModel::getKeyDisplayLabel(const string& label) const
	{
		return pImpl->keyDisplayLabelsMap[label];
	}
	
	
	void KeyboardModel::setKeyDisplayLabel(const string &label, const string &displayLabel)
	{
		pImpl->keyDisplayLabelsMap[label] = displayLabel;
	}


	bool KeyboardModel::hasGlyphForKeyLabel(const string &label) const
	{
		return pImpl->keyGlyphCodes.find(label) != pImpl->keyGlyphCodes.end();
	}


	const Glyph &KeyboardModel::getGlyphForKeyLabel(const string &label) const
	{
		return pImpl->keyGlyphCodes.at(label);
	}


	void KeyboardModel::setGlyphForKeyLabel(const Glyph &glyph, const string &label)
	{
		pImpl->keyGlyphCodes[label] = glyph;
	}


	bool KeyboardModel::hasAltGlyphForKeyLabel(const string& label) const
	{
		return pImpl->keyAltGlyphCodes.find(label) != pImpl->keyAltGlyphCodes.end();
	}


	const Glyph &KeyboardModel::getAltGlyphForKeyLabel(const string &label) const
	{
		return pImpl->keyAltGlyphCodes.at(label);
	}


	void KeyboardModel::setAltGlyphForKeyLabel(const Glyph &glyph, const string &label)
	{
		pImpl->keyAltGlyphCodes[label] = glyph;
	}


	const RGBByte &KeyboardModel::getColorForKey(const string &label)
	{
		// use a try catch here
		RGBByte &ret(pImpl->keyColores.at(label));
		return ret;
	}


	void KeyboardModel::setColorForKey(const RGBByte &color, const string &keyLabel)
	{
		pImpl->keyColores[keyLabel] = color;
	}
	
	
#pragma mark - Event Handling
	
	void KeyboardModel::keyTouchEvent(string keyLabel, CCTouch *touch, TouchType type)
	{
		int requiredGlyphLevel = pImpl->keyGlyphCodes[keyLabel].getLevel();
		bool hasMapping = GameState::getInstance().player().getLevel() >= requiredGlyphLevel;
		GameState::getInstance().keypressTracker().trackTouchEvent(touch, keyLabel, type, hasMapping);
	}


	void KeyboardModel::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		if ("KeyboardView_KeyPress" == code) {
			std::shared_ptr<KeyboardViewTouchInfo> pInfo = std::static_pointer_cast<KeyboardViewTouchInfo>(data);
			keyTouchEvent(pInfo->label, pInfo->touch, pInfo->type);
		}

		else if ("ScoreKeeper_NewLevelUpdate" == code) {
			pImpl->setUpKeysFromGlyphMap();

			// trigger an event for the KBV
			Notif::send("KeyboardModel_NewLevel");
		}

		else if ("KeypressTracker_ModKeyPressed" == code) {
			this->enableAltMode(true);

		} else if ("KeypressTracker_ModKeyReleased" == code) {
			// this->enableAltMode(false);

		} else if ("CopyText_TriggerAltKey" == code) {
			this->enableAltMode(false);
		}
	}
	
}
