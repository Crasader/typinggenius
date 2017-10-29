//
//  KeyModel.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "KeyModel.h"

namespace ac {

	/** Constructor */
//	KeyModel::KeyModel(string label) : label(label)
//	{
//		this->state = KeyPressState::Up;
//	}


	/** Constructor */
	KeyModel::KeyModel(string label, const Glyph &glyph) : label(label), glyph(glyph)
	{
		this->state = KeyPressState::Up;
	}


	void KeyModel::keyPressed()
	{
		// LogI << this->printables[0];
		this->state = KeyPressState::Down;
	}


	void KeyModel::keyReleased()
	{
		// LogI << this->printables[0];
		this->state = KeyPressState::Up;
	}
}