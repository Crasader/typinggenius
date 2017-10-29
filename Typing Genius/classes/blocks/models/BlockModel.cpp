//
//  BlockModel.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockModel.h"


namespace ac {

#pragma mark - Lifetime

	BlockModel::BlockModel() : BlockModel(Glyph(0))
	{
		LogI << "Now in BlockModel constructor";
	}


	BlockModel::BlockModel(const Glyph &glyph) : glyph(glyph), obstructionIntroductionDelay(0), obstructionTimeDuration(0)
	{
		LogI << "Now in BlockModel constructor";
	}


	BlockModel::~BlockModel()
	{
		LogI << "Now in BlockModel destructor";
	}


#pragma mark - Getters

	const Glyph &BlockModel::getGlyph() const
	{
		return this->glyph;
	}


	void BlockModel::setGlyph(const Glyph &glyph)
	{
		this->glyph = glyph;
	}


	void BlockModel::setHasObstruction(bool hasObstruction)
	{
		this->isObstructed = hasObstruction;
	}


	bool BlockModel::hasObstruction() const
	{
		return this->isObstructed;
	}

}
