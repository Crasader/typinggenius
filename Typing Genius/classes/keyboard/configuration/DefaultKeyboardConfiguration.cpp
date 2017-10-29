/*
 * DefaultKeyboardConfiguration.cpp
 * Typing Genius
 *
 * Created by Aldrich Co on 8/16/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 * @brief a special kind of keyboard config manager that presents sensible defaults.
 * Has methods to parse the associated JSON file and initialize the Keyboard.
 */


#include "DefaultKeyboardConfiguration.h"
#include "KeyboardModel.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include "Keyboard.h"
#include "KeyModel.h"
#include "Utilities.h"

namespace ac
{
	using std::pair;
	using std::shared_ptr;
	using std::map;
	using std::vector;
	using utilities::PropTree;
	using utilities::PropTreeValType;

	struct RowInfo
	{
		float xOffset;
		float yOffset;
		float hGap; // horizontal gap between successive buttons
		string rowLabel;
	};

	// definitions below.
	void initializeKeyboardMetadata(Keyboard &keyboard, PropTree &pt);
	
	float initializeKeyPositionAndSize(Keyboard &keyboard, const string &label,
									   const RowInfo &rowInfo, float xOffset, PropTree &pt);
	void initializeKeyPrintables(Keyboard &keyboard, const string &label, const RowInfo &rowInfo,
								 PropTree &pt);
	void initializeKeyDisplayLabels(Keyboard &keyboard, const string &label, const RowInfo &rowInfo,
									PropTree &pt);

	RowMap readRowInfo(PropTree &pt);
	

	bool DefaultKeyboardConfiguration::initialize(Keyboard &keyboard)
	{
		LogD << "Inside DefaultKeyboardConfiguration::initialize";
		
		// maybe these should be called after the json string loading....
		// should have keyboard initialized through this.
		
		// start parsing here
		const string &jsonFileToLoad = configFileName();
		LogI << "Initializing Keyboard configuration, with file to be loaded: " << jsonFileToLoad;
		
		PropTree pt = utilities::getPropertyTreeFromJSONFileBundle(jsonFileToLoad);
		
		if (pt.empty()) { LogE << "PropTree is empty."; return false; }
				
		LogI << "Initializing keyboard metadata...";
		initializeKeyboardMetadata(keyboard, pt);

		LogI << "Initializing row data...";
		RowMap rowInfo(readRowInfo(pt));

		LogI << "Initializing keys...";
		this->initializeKeys(keyboard, rowInfo, pt);
		this->initializeOtherKeys(keyboard, pt);

		LogI << "Loaded completely. Initializing keyboard configuration...";
		// const shared_ptr<KeyboardModel>& model = 	keyboard.model();
				
		LogI << "Done initializing keyboardConfiguration.";

		keyboard.model()->setupKeyMappings();

		this->initializeKeyColors(keyboard);

		return true;
	}


#pragma mark - Class-specific

	// if the row map doesn't get it, they should be in "keys/other_keys"
	void DefaultKeyboardConfiguration::initializeKeys(Keyboard &keyboard, const RowMap &rowMap, PropTree &pt)
	{
		// start with the labels, then work out other details enclosed
		// key labels...
		vector<string> keyLabels;
		for (RowMap::const_iterator iter = rowMap.begin(); iter != rowMap.end(); ++iter) {
			float xOffset = 0; // from xStart

			BOOST_FOREACH(PropTreeValType &v, pt.get_child("keys." + iter->first)) {

				const string &label = v.first;
				const RowInfo &rowInfo = iter->second;

				// "first" contains the labels; second is for the data (Structured)
				LogD << label;
				keyLabels.push_back(label);

				LogD << "Initializing key positions and sizes...";
				xOffset = initializeKeyPositionAndSize(keyboard, label, rowInfo, xOffset, pt);

				LogD << "Initializing key display labels...";
				initializeKeyDisplayLabels(keyboard, label, rowInfo, pt);

				// this->keysToRow[label] = rowInfo.rowLabel;

				std::vector<string> &keys(this->keysInRows[rowInfo.rowLabel]);
				keys.push_back(label);
			}
		}

		keyboard.model()->setKeyLabels(keyLabels);
	}


	void DefaultKeyboardConfiguration::initializeOtherKeys(Keyboard &keyboard, PropTree &pt)
	{
		const string OtherKeysPathRoot("keys.other_keys");
		vector<string> otherKeyLabels;

		PropTree &perOtherKeysRoot = pt.get_child(OtherKeysPathRoot);
		BOOST_FOREACH(PropTreeValType &v, perOtherKeysRoot) {

			const string &keyLabel(v.first); // an example: "mod:shift_right"

			otherKeyLabels.push_back(keyLabel);

			float x = pt.get<float>(OtherKeysPathRoot + "." + keyLabel + ".x", 0);
			float y = pt.get<float>(OtherKeysPathRoot + "." + keyLabel + ".y", 0);

			// whatever is defined in the json as (x,y) for these keys will be directly translated to the canvas
			const RowInfo emptyRowInfo { x, y, 0, "" }; // rowLabel field is ""

			initializeKeyPositionAndSize(keyboard, keyLabel, emptyRowInfo, 0, pt);
		}

		vector<string> existingLabels(keyboard.model()->getKeyLabels());
		existingLabels.reserve(existingLabels.size() + otherKeyLabels.size()); // preallocate memory
		existingLabels.insert(existingLabels.end(), otherKeyLabels.begin(), otherKeyLabels.end());

		// concatenate, not overwrite
		keyboard.model()->setKeyLabels(existingLabels);
	}


	void DefaultKeyboardConfiguration::initializeKeyColors(Keyboard &keyboard)
	{
		std::shared_ptr<KeyboardModel> model(keyboard.model());

		// set up colors

		// could i tell the relative position of keys given their labels?
		vector<string> keyLabels = model->getKeyLabels();
		for (string label: keyLabels) {
			// all the key labels are assumed to be in the correct order: key:001, 002, and so on

			string rowLabel;
			size_t offsetInRow;

			// locate the rowlabel of the string
			for (auto &pair : keysInRows) {
				offsetInRow = 0;
				std::vector<string> &keyLabelsInRow(pair.second);
				for (string keyLabel : keyLabelsInRow) {
					if (keyLabel == label) { // found
						rowLabel = pair.first;
						break;
					}
					offsetInRow += 1;
				}
				if (!rowLabel.empty()) {
					break;
				}
			}


			if (!rowLabel.empty()) {
				size_t numberOfKeys(keysInRows[rowLabel].size());
				size_t keyOffset(offsetInRow);

				RGBByte startColor;
				if (rowLabel == "row_0") {
					// startColor = { 56, 77, 123 };
					startColor = { 100, 100, 100 };
				} else if (rowLabel == "row_1") {
					startColor = { 56, 77, 123 };
					// startColor = { 64, 88, 141 };
				} else if (rowLabel == "row_2") {
					startColor = { 64, 88, 141 };
					// startColor = { 72, 99, 158 };
				} else {
					startColor = { 72, 99, 158 };
					// startColor = { 80, 110, 175 };
				}

				RGBByte rgb = utilities::nextRGBByteHueShift(startColor, 360 * ((float) keyOffset / numberOfKeys));
				model->setColorForKey(rgb, label);
			} else {
				model->setColorForKey({ 100,100,100 }, label);
			}

			// i need a an offset from the row (for each key) so i can add something hue shift to the base color
			// also i need the number of elements in a row
		}
	}


#pragma mark - Other methods involving setup

	void initializeKeyboardMetadata(Keyboard &keyboard, PropTree &pt)
	{
		const shared_ptr<KeyboardModel>& model = keyboard.model();
		
		string label = pt.get<string>("label", "No label");
		model->setLabel(label);
		
		string description = pt.get<string>("description", "No description");
		model->setDescription(description);
		
		// default key size
		KeySize keysize = { 0, 0 };
		keysize.width = pt.get<float>("default_key_size.width", 0.0f);
		keysize.height = pt.get<float>("default_key_size.height", 0.0f);
		
		LogD << "Default key size (in pct): " << keysize.width << " x " << keysize.height;
		
		model->setKeySize(keysize);
		
		KeyboardSize kbSize = { 0.0f, 0.0f };
		kbSize.width = pt.get<float>("default_keyboard_size.width", 0.0f);
		kbSize.height = pt.get<float>("default_keyboard_size.height", 0.0f);
		
		LogD << "Keyboard size (in pct): " << kbSize.width << " x " << kbSize.height;
		model->setKeyboardSize(kbSize);
	}
	
	
	float initializeKeyPositionAndSize(Keyboard &keyboard, const string &label,
									   const RowInfo &rowInfo, float xOffset, PropTree &pt)
	{
		// note: pt is at a single key (e.g., "key:z")
		const shared_ptr<KeyboardModel> &model = keyboard.model();

		// ----- key size
		// includes the trailing dot.
		const string pathPrefix(rowInfo.rowLabel.empty() ? "keys.other_keys." : ("keys." + rowInfo.rowLabel + "."));

		float width = pt.get<float>(pathPrefix + label + ".size.width", 0);
		float height = pt.get<float>(pathPrefix + label + ".size.height", 0);

		// key size not defined
		if (width == 0 || height == 0) {
			const KeySize &defaultKeySize(keyboard.model()->getKeySize());
			LogD << "No custom size for key " << label << " is defined, using defaults: " << defaultKeySize;
			width = defaultKeySize.width;
			height = defaultKeySize.height;
		} else {
			LogD << "Key size for key " << label << ": (" << width << " x " << height << ")";
		}

		KeySize size = { width, height };
		model->setKeySize(label, size);

		// ----- whether it uses any special sprite.
		string spriteVersion = pt.get<string>(pathPrefix + label + ".version", "");
		if (spriteVersion == "long") {
			model->setKeySpriteType(label, KeySpriteType::Elongated);
		} else if (spriteVersion == "shift") {
			model->setKeySpriteType(label, KeySpriteType::Shift);
		} else {
			model->setKeySpriteType(label, KeySpriteType::Normal);
		}

		// ----- key position
		// note: assume that the anchor point of the sprites and for these coordinates is (0,0)
		float positionX = xOffset + rowInfo.hGap;

		KeyboardPoint point = { rowInfo.xOffset + positionX, rowInfo.yOffset };
		LogD2 << "Key Position for key " << label << ": (" << point.x << ", " << point.y << ")";
		model->setKeyPosition(label, point);

		return positionX + size.width;
	}

	
	// what is shown on the key views themselves
	void initializeKeyDisplayLabels(Keyboard &keyboard, const string &label, const RowInfo &rowInfo,
									PropTree &pt)
	{
		const shared_ptr<KeyboardModel>& model = keyboard.model();
		string displayLabel(pt.get<string>("keys." + rowInfo.rowLabel + "." + label +
										   ".key_display", ""));
		model->setKeyDisplayLabel(label, displayLabel);
		LogI << boost::format("The display label for %1% is %2%") % label % displayLabel;
	}
	
	
	void initializeKeyPrintables(Keyboard &keyboard, const string &label, const RowInfo &rowInfo,
								 PropTree &pt)
	{
		const shared_ptr<KeyboardModel>& model = keyboard.model();
		
		// handle the printables
		vector <string> printables;
		PropTree &perKeyPrintablesRoot = pt.get_child("keys." + rowInfo.rowLabel + "." + label +
													  ".printables");
		LogD << "Printables for key: " << label;
		BOOST_FOREACH(PropTreeValType &w, perKeyPrintablesRoot) {
			const string &printable = w.second.data();
			LogD1 << printable;
			printables.push_back(printable);
		}
		model->addPrintable(label, printables);
	}


	RowMap readRowInfo(PropTree &pt)
	{
		RowMap ret;
		PropTree &perRowRoot = pt.get_child("rows");
		BOOST_FOREACH(PropTreeValType &v, perRowRoot) {
			const string &label = v.first;
			RowInfo rInfo;
			rInfo.xOffset = pt.get<float>("rows." + label + ".x_offset", 0);
			rInfo.yOffset = pt.get<float>("rows." + label + ".y_offset", 0);
			rInfo.hGap = pt.get<float>("rows." + label + ".button_hgap", 0);
			rInfo.rowLabel = label;

			ret[label] = rInfo; // add to map
		}
		return ret;
	}
}
