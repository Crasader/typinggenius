//
//  BlockChain.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/14/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	A row of Blocks stringed together (meant to be typed one after the other).

#pragma once

namespace ac {

	using std::string;
	class BlockModel;
	class GlyphString;

	// the equivalent of a string.
	class BlockChain
	{
	public:
		// object at index
		BlockModel &itemAt(size_t index);

		void addItem(BlockModel &item);

		size_t size() const;

		void setString(const GlyphString &str);

	private:
		std::vector<BlockModel> elements;
	};
	
	
}