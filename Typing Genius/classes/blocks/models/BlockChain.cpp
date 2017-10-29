//
//  BlockChain.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/14/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockChain.h"
#include "BlockModel.h"
#include "Glyph.h"

namespace ac {

	// object at index
	BlockModel &BlockChain::itemAt(size_t index)
	{
		return this->elements[index];
	}


	void BlockChain::addItem(BlockModel &item)
	{
		this->elements.push_back(item);
	}


	size_t BlockChain::size() const
	{
		return this->elements.size();
	}


	void BlockChain::setString(const GlyphString &str)
	{
		size_t size = str.size();

		// vector operation
		this->elements.resize(size);

		// some failsafes needed
		for (size_t i = 0; i < size; i++) {
			BlockModel &model(this->itemAt(i));

			// change the model the char is pointing at, and then cause the view to follow suit
			model.setGlyph(str[i]);
		}
	}
}