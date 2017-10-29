//
//  CopyText.h
//  Typing Genius
//
//  Created by Aldrich Co on 10/16/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "ACTypes.h"
#include "Glyph.h"

#pragma once

namespace ac {

	using std::string;
	struct CopyTextImpl;
	
	class CopyText
	{
	public:
		CopyText();
		~CopyText();
		
		// resets the copy text state to the start position. Issued upon the use of the restart button in the main screen
		void reset();

		// called as a result of processing buffered input.
		void keyEventTriggered(string &label, KeyPressState &, const Glyph &);

		size_t curOffset() const; // position in the copy string that is affected by next keypress (starting from zero)
		float getProgress() const;
		
		GlyphString getVisibleString() const; // the string to be shown

		// number of glyphs to the right of the visible string; indicate whether figure represents
		// before or after the advance
		size_t remainingCharCount(bool preAdvance) const;

		// BlockCanvasModel uses this to determine how many blocks to slide in (and out);
		// done after checking what was entered.
		size_t unitsToAdvance() const;
		size_t unitsToMistakeHL() const;
		
		// event that objects like KeypressTracker or BlockCanvasModel can call
		void tryProcessingNextBufferedInput();

		// response to BlockCanvasView initializing the spriteFrameCache.
		void setBlocksPerLine(size_t blocksPerLine);
		size_t getBlocksPerLine() const;

		void setCopyString(const GlyphString &glyphString);
		void clearCopyString();

		GlyphString &copyString();
		const GlyphString &copyString() const;


		// The BlockCanvas UI notifies CopyText of this.
		void registerStreakFinished();


		/** 
		 *	@brief Reloads copy string so that the offscreen (not visible) glyphs get regenerated
		 *	based on the new player level. Glyphs in the "visible" range are not affected.
		 */
		void reComposeCopyText(size_t playerLevel);

	private:
		std::unique_ptr<CopyTextImpl> pImpl;
	};
}