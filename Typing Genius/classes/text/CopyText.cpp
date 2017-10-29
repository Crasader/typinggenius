//
//  CopyText.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/16/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "CopyText.h"
#include "DebugSettingsHelper.h"
#include "KeypressTracker.h"
#include "BlockCanvasView.h" // to learn how many blocks can fit within a row.
#include "ScreenResolutionHelper.h"
#include "KeyboardModel.h"
#include "Keyboard.h"
#include "ScoreKeeper.h"
#include "PlayerLevel.h"
#include "Player.h"
#include "GlyphMap.h"
#include "GameState.h"
// #include "BlockTypesetter.h"

namespace ac {

#pragma mark - pImpl Definition

	struct CopyTextImpl
	{
		// ctor
		CopyTextImpl(CopyText &copyTextRef):
		copyTextRef(copyTextRef),
		visibleBlocksPerRow(0),
		copyStringOffset(0),
		unitsToAdvance(0),
		unitsToAdvanceSaved(0),
		unitsToMistakeHL(0),
		spaceKeyIsUsed(false)
		{
#if BOOST_TEST_TARGET
			visibleBlocksPerRow = 12;
#else
			visibleBlocksPerRow = BlockCanvasView::blockCountPerRow();
#endif
		}

		GlyphString copyString; // the text to be copied.
		GlyphString enteredString; // temporarily store the string entered here. Intended for checking purposes.

		size_t unitsToAdvance; // observers can use this info to determine what to do after keypress
		size_t unitsToAdvanceSaved;
		size_t unitsToMistakeHL; // typically 1, when the user hits a block incorrectly.
		bool spaceKeyIsUsed;
		bool isBlockedByEncasement;

		size_t copyStringOffset; // offset into the copy string representing the first letter of the visible block
		size_t visibleBlocksPerRow; // and theres one row
		
		void inputKeyWithValue(const GlyphString &gs); // present printable for checking.
		void loadCopyString(size_t);
		void performCheck(); // this makes the check and decides whether to advance the cursor or not
		// (by the appropriate amount). That's all it does

		void clearEnteredString();

		inline ScoreKeeper &scoreKeeper() { return GameState::getInstance().scoreKeeper(); }
		CopyText &copyTextRef;

		void notifyGameStateOfGameEndState();
	};


#pragma mark - Lifetime

	CopyText::CopyText()
	{
		pImpl.reset(new CopyTextImpl(*this));
	}


	CopyText::~CopyText() {}


	void CopyText::reset()
	{
		pImpl.reset(new CopyTextImpl(*this));
		pImpl->loadCopyString(GameState::getInstance().player().getLevel());
		Notif::send("CopyText_LoadedString");
	}


	void CopyText::setBlocksPerLine(size_t blocksPerLine)
	{
		pImpl->visibleBlocksPerRow = blocksPerLine;
	}


	size_t CopyText::getBlocksPerLine() const
	{
		return pImpl->visibleBlocksPerRow;
	}


#pragma mark - Listener to KeyboardModel

	void CopyText::keyEventTriggered(string &label, KeyPressState &state, const Glyph &glyph)
	{
		if (state == KeyPressState::Down) {
			LogD1 << "Key pressed down, with code: " << glyph.getCode();
			
			// this may modify unitsToAdvance and other internal values
			if (glyph.getCode() < 0) {
				LogI << "nothing assigned to key with label " << label;
				return;
			}

			pImpl->inputKeyWithValue(glyph);

			if (pImpl->unitsToAdvance > 0) {
				Notif::send("CopyText_Preadvance");
				return;
			}

			if (pImpl->unitsToMistakeHL > 0) {

				if (pImpl->isBlockedByEncasement) {
					Notif::send("CopyText_Blocked");
				} else {
					Notif::send("CopyText_Mistake");
				}
				pImpl->clearEnteredString();
				return;
			}
			
		} else if (state == KeyPressState::Up) {

			// don't assume that everything went well.
			pImpl->copyStringOffset += pImpl->unitsToAdvance;
			LogD3 << "copy string offset now at " << pImpl->copyStringOffset;

			if (pImpl->unitsToAdvance > 0) {
				if (pImpl->spaceKeyIsUsed) {
					Notif::send("CopyText_AdvanceWithSpace");
				} else {
					Notif::send("CopyText_Advance");
				}
			}

			// this only covers the case where the player ran out of blocks to clear, but not when the user ran out
			// of time. StatsHUD which controls the ingame timer can also notify GameState.
			if (GameState::getInstance().isGameStarted() && pImpl->copyStringOffset >= pImpl->copyString.size()) {
				pImpl->notifyGameStateOfGameEndState();
				Notif::send("CopyText_AllCleared");
			}

			// wait for the animation signal
			pImpl->clearEnteredString();
		}
	}


	void CopyTextImpl::notifyGameStateOfGameEndState()
	{
		scoreKeeper().recordPostSessionTimeRemaining(GameState::getInstance().getTimeRemaining() / 1000.0f);
		scoreKeeper().recordPostSessionAccuracy();
		bool finished = true;
		GameState::getInstance().stop(finished); // pauses the running timer.
	}


#pragma mark - My Location

	// not yet used it seems
	size_t CopyText::curOffset() const
	{
		return pImpl->copyStringOffset;
	}


#pragma mark - Checking
	
	void CopyTextImpl::inputKeyWithValue(const GlyphString &gs)
	{
		LogI << "appending " << gs;

		bool inTestMode(false);
#ifdef BOOST_TEST_TARGET
		inTestMode = true;
#endif

		if (!inTestMode) {
			if (!GameState::getInstance().isGameStarted()) {
				// when game timer runs out the copyString is cleared (see GameState::stop())
				if (copyString.size() > 0 && copyStringOffset == 0) {
					LogI << boost::format("game has started!");
					// notify listeners: game has started.
					Notif::send("CopyText_FirstPress");
				} else {
					// game is finished, need to reset
					return; // <----------- exit
				}
			} else {
				
			}
		}
		
		enteredString.append(gs);
		performCheck();
	}
	

	void CopyTextImpl::performCheck()
	{
		size_t enteredLength = enteredString.size();
		// hope this doesn't overflow.
		
		if (copyStringOffset >= copyString.size()) {
			LogW << "CopyString offset now past the size";
			return;
		}

		GlyphString toBeCompared(copyString.substr(copyStringOffset, enteredLength));
		
		if (toBeCompared.size() < 1) {
			LogW << "You've reached the end of the string. Escaping";
			return;
		}

		bool isGodMode = GameState::getInstance().isGodMode();
		bool isCorrect = isGodMode || toBeCompared == enteredString; // or make the appropriate type of check
		bool isSpace = enteredLength == 1 && enteredString[0].getCode() == 0;
		this->spaceKeyIsUsed = false;

		// can't be cleared without gradually 'peeling away' the encasement
		this->isBlockedByEncasement = copyString.encasementLevelAtIndex(copyStringOffset) > 0;

		if (this->isBlockedByEncasement) {

			unitsToMistakeHL = enteredLength;
			LogD2 << "Current block is encased. It's not going anywhere unless you break the encasement.";

		} else if (isSpace && !isGodMode) { // spacebar special case: won't contribute to streak, mistake, or scoring
			unitsToAdvance = enteredLength;
			unitsToAdvanceSaved = 0;
			spaceKeyIsUsed = true;
			LogD << "(Spacebar) advancing cursor by " << unitsToAdvance;
			LogI << boost::format("copy string offset now at %d") % copyStringOffset;

		} else if (isCorrect) {
			// then correct, advance by length of entered
			unitsToAdvance = enteredLength;
			unitsToAdvanceSaved = 0;
			LogD << "advancing cursor by " << unitsToAdvance;
			LogI << boost::format("copy string offset now at %d") % copyStringOffset;
			
			this->scoreKeeper().recordStreakIncrement(enteredLength);
			this->scoreKeeper().recordBlockClear(enteredLength);

		} else {
			// incorrect: do not advance, blink the block glyph, report mistake to scorekeeper
			unitsToMistakeHL = enteredLength;
			LogD2 << "The key you should be entering is " << toBeCompared;
			this->scoreKeeper().recordMistakenAttempt(enteredLength);
		}
	}


	void CopyText::registerStreakFinished()
	{
		pImpl->scoreKeeper().recordStreakReset();
	}


	size_t CopyText::unitsToAdvance() const
	{
		return pImpl->unitsToAdvance;
	}


	size_t CopyText::unitsToMistakeHL() const
	{
		return pImpl->unitsToMistakeHL;
	}
	
	
	void CopyTextImpl::clearEnteredString()
	{
		enteredString.clear();

		unitsToAdvanceSaved += unitsToAdvance;
		unitsToAdvance = 0;
		unitsToMistakeHL = 0;
	}
	
	
	// take a key event from the KPT, process it, and then call this again
	// remember, once a mistake is found when checking, the buffer is emptied.
	void CopyText::tryProcessingNextBufferedInput()
	{
		LogD << "CopyText: I can get more buffered input!";
		
		KeypressTracker &kpt(GameState::getInstance().keypressTracker());
		
		if (kpt.hasElementsInBuffer()) {
			std::shared_ptr<KeyboardModel> kbm(Keyboard::getInstance().model());

			KeyEvent kev(kpt.removeNextKeyEventFromBuffer());

			KeyPressState pressState = kev.type == TouchType::TouchBegan ? KeyPressState::Down : KeyPressState::Up;

			if (!kbm->isInAltMode()) { // normal behavior

				if (kbm->hasGlyphForKeyLabel(kev.key)) {
					const Glyph &glyph = kbm->getGlyphForKeyLabel(kev.key);
					// perform the keyEventTriggered to kick off checking
					keyEventTriggered(kev.key, pressState, glyph);
				}
			} else {
				// trigger special powers
				if (!kev.key.empty() && !utilities::keyIsAModifier(kev.key) && kev.type == TouchType::TouchEnded) {
					LogI << "sending alt command for key " << kev.key;
					// now send a Notif along with the key label. (KBM or somebody should take notice)
					Notif::send("CopyText_TriggerAltKey", std::make_shared<KeyEvent>(kev));
				}
			}
		}
	}


#pragma mark - Getting (parts of) the Copy String

	GlyphString CopyText::getVisibleString() const
	{
		// avoid exception
		if (pImpl->copyStringOffset >= pImpl->copyString.size()) {
			GlyphString empty;
			return empty;
		}
		
		GlyphString visibleString(pImpl->copyString.substr(pImpl->copyStringOffset, pImpl->visibleBlocksPerRow));
		LogD4 << boost::format("The visible string: %s") % visibleString;
		return visibleString;
	}


	size_t CopyText::remainingCharCount(bool preAdvance) const
	{
		// copyStringOffset reflects post check reality
		size_t rightEdge = pImpl->copyStringOffset + pImpl->visibleBlocksPerRow;
		if (preAdvance) {
			LogD << "saved units advanced: " << pImpl->unitsToAdvanceSaved;
			rightEdge -= pImpl->unitsToAdvanceSaved; // pImpl->unitsToAdvance by this point has been cleared to 0
		}

		if (rightEdge >= pImpl->copyString.size()) {
			return 0;
		} else {
			return pImpl->copyString.size() - rightEdge;
		}
	}


#pragma mark - Initializing Copy String


	void CopyTextImpl::loadCopyString(size_t playerLevel = 1)
	{
		DebugSettingsHelper &debug(DebugSettingsHelper::sharedHelper());

		std::vector<Glyph> usedGlyphs(GameState::getInstance().glyphMap().glyphsUsed(playerLevel));

		int noOfGlyphsToGenerate = debug.intValueForProperty("glyphs_to_generate");

		// should be read from a function in PlayerLevel
		vector<float> repeatChances = PlayerLevel::glyphRepeatChances(playerLevel);
		this->copyString.generateRandom(noOfGlyphsToGenerate, usedGlyphs, repeatChances);

		this->copyString.generateObstructions();
		this->copyString.generateEncasements(noOfGlyphsToGenerate, 0);
		// LogD << "(Random) Copy string Loaded: " << this->copyString;
	}


	void CopyText::reComposeCopyText(size_t playerLevel)
	{
		DebugSettingsHelper &debug(DebugSettingsHelper::sharedHelper());
		GlyphMap &gm(GameState::getInstance().glyphMap());
		
		gm.loadGlyphToKeyMappings(playerLevel);
		std::vector<Glyph> usedGlyphs(gm.glyphsUsed(playerLevel));

		// one past the visibility
		size_t indexOfRightEdge = pImpl->copyStringOffset + getVisibleString().size();
		size_t noOfGlyphsToGenerate = debug.intValueForProperty("glyphs_to_generate") - indexOfRightEdge;

		// (Optional) subtract this amount by what's visible.

		vector<float> repeatChances = PlayerLevel::glyphRepeatChances(playerLevel);

		GlyphString appendee;
		appendee.generateRandom(noOfGlyphsToGenerate, usedGlyphs, repeatChances);

		pImpl->copyString.resize(indexOfRightEdge);
		pImpl->copyString.append(appendee);

		pImpl->copyString.generateObstructions();
		pImpl->copyString.generateEncasements(noOfGlyphsToGenerate, indexOfRightEdge);

		LogI << "Appended new string";
	}

	
#pragma mark - Access the internal copy string
	
	void CopyText::setCopyString(const GlyphString &newCopyStr)
	{
		pImpl->copyString = newCopyStr;
		Notif::send("CopyText_LoadedString");
	}


	void CopyText::clearCopyString()
	{
		pImpl->copyString.clear();
		Notif::send("CopyText_ClearedString");
	}


	GlyphString &CopyText::copyString()
	{
		return pImpl->copyString;
	}


	const GlyphString &CopyText::copyString() const
	{
		return pImpl->copyString;
	}


#pragma mark - Score and related Stats
	
	float CopyText::getProgress() const
	{
		// curOffset / glyf string size
		float progress = (float) curOffset() / pImpl->copyString.size();
		return progress;
	}
}
