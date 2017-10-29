//
//  BlockModel.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once
#include "ACTypes.h"
#include "Glyph.h"

namespace ac {

	struct BlockModelImpl;

	class BlockModel
	{
	public:

		BlockModel();
		BlockModel(const Glyph &glyph);
		virtual ~BlockModel();

		const NeutralSize &getSize() const;

		const Glyph &getGlyph() const;
		void setGlyph(const Glyph &glyph);

		void setHasObstruction(bool);
		bool hasObstruction() const;


		inline float getObstructionIntroductionDelay() const {
			return this->obstructionIntroductionDelay;
		}

		inline float getObstructionTimeDuration() const {
			return this->obstructionTimeDuration;
		}

		inline void setObstructionTimeDuration(float duration) {
			this->obstructionTimeDuration = duration;
		}

		inline void setObstructionIntroductionDelay(float delay) {
			this->obstructionIntroductionDelay = delay;
		}

		// see note below for 'encased'
		inline bool isEncased() const { return this->encased; }
		inline void setEncased(bool encased) { this->encased = encased; }


	private:
		// a unique pointer pImpl would complicate things here, as BlockModel
		// is meant to be added to a vector in a BlockChain.
		Glyph glyph;

		bool isObstructed;

		/** Encased in ice, that is. note: only used to decide whether the block is shown 
		 *	with ice during creation. Ice will melt without touching this variable, and setting
		 *	this to false will not make the block ice encasement disappear.
		 */
		bool encased;

		/** 
		 *	after sliding in, how long before the frog shows up
		 */
		float obstructionIntroductionDelay;

		/**
		 *	time after being shown before an obstruction is removed (and unavailable to collect)
		 */
		float obstructionTimeDuration;
	};

}