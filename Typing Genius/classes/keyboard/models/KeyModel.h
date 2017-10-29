//
//  KeyModel.h
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Model class of a pressable key. Would need to consider things like the following:
//	- press state (up or down)
//	- highlight state (glow?)
//	- level of "wear"
//	- "stickiness" factor
//	- caps lock, shift
//	- key label appearance (font, rotation?) - might be better handled elsewhere.
//	- autorepeat
//	- handle key presses

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstddef>
#include "ACTypes.h"
#include "Glyph.h"

namespace ac {

	using std::string;
	using std::vector;


	// pertains to shape
	enum class KeySpriteType
	{
		Normal,
		Elongated, // for spacebar
		Shift // for shift modifier keys
	};

	class KeyModel {
	public:

		KeyModel(string label, const Glyph &);

		KeyModel() : KeyModel("", Glyph(-1))
		{

		}

		// copy constructor and assignment operator
		KeyModel(const KeyModel& other) : KeyModel(other.getLabel(), other.getGlyph())
		{
		}

		KeyModel& operator=(const KeyModel& other) {
			this->label = other.getLabel();
			this->glyph = other.getGlyph();
			return *this;
		}
		
		~KeyModel()
		{
			// LogD << "Destructor for KeyModel called";
		}

		void setPrintables(const std::vector<std::string> &printables) { this->printables = printables; }
		vector<string>& getPrintables() { return this->printables; }

		inline const string& getLabel() const { return label; }

		inline void setGlyph(const Glyph &glyph) { this->glyph = glyph; }
		inline const Glyph &getGlyph() const { return glyph; }


		void keyPressed();
		void keyReleased();
		
		const KeyPressState getPressedState() const { return state; }

	private:
		/* non-human readable, could contain anything but should uniquely identify a key within a keyboard */
		string label;


		Glyph glyph;

		KeyPressState state;

		/*
		 Depending on shift state, or other factors, a key would print one of two (or perhaps more?) characters.
		 For example, "5" and "%". Some keys don't have printables, such as Backspace and Shift.
		 */
		vector<string> printables;
	};
	
	
	
}