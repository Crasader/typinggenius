//
//  BlockCanvasModel.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/12/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "BlockCanvasModel.h"

#include "BlockCanvasView.h"
#include "BlockChain.h"
#include "BlockModel.h"
#include "CopyText.h"
#include "GameState.h"
#include "Glyph.h"
#include "Player.h"
#include "ScoreKeeper.h"
#include "StatsHUDModel.h"
#include "Utilities.h"

namespace ac {

#pragma mark - pImpl Definition


	struct BlockCanvasModelImpl
	{
		BlockChain row1Blox;

		BlockCanvasModel *bcModel;

		BlockCanvasModelImpl(BlockCanvasModel *bcModel) : bcModel(bcModel), row1Blox()
		{
			
		}

		// frogs and ice: index parameters refer to those in the visible block chain, not the copy string (in CopyText)
		bool hasObstructionAtIndex(size_t index) const;
		size_t encasementLevelAtIndex(size_t index) const;
		void reduceEncasementLevelAtIndex(size_t index, size_t amount);
	};


#pragma mark - LifeTime

	BlockCanvasModel::BlockCanvasModel()
	{
		pImpl.reset(new BlockCanvasModelImpl(this));
	}

	
	BlockCanvasModel::~BlockCanvasModel()
	{
	}


#pragma mark - Locate and Obtain String Lines

	BlockChain &BlockCanvasModel::getBlockChain()
	{
		return pImpl->row1Blox;
	}


	bool BlockCanvasModelImpl::hasObstructionAtIndex(size_t index) const
	{
		// obtain the glyphstring from.. copytext's copystring
		const CopyText &ct(GameState::getInstance().copyText());
		const GlyphString &gs(ct.copyString());

		const size_t copyStringOffset = index + ct.curOffset();

		return (gs.hasObstructionAtIndex(copyStringOffset));
	}


	size_t BlockCanvasModelImpl::encasementLevelAtIndex(size_t index) const
	{
		const CopyText &ct(GameState::getInstance().copyText());
		const GlyphString &gs(ct.copyString());
		const size_t copyStringOffset = index + ct.curOffset();
		return (gs.encasementLevelAtIndex(copyStringOffset));
	}


	void BlockCanvasModelImpl::reduceEncasementLevelAtIndex(size_t index, size_t amount)
	{
		CopyText &ct(GameState::getInstance().copyText());
		GlyphString &gs(ct.copyString());
		const size_t copyStringOffset = index + ct.curOffset();
		gs.reduceEncasementLevelAtIndex(copyStringOffset, amount);
	}

	
#pragma mark - Respond to Notifs Events

	void BlockCanvasModel::notifCallback(const string &code, std::shared_ptr<void> data) {
		if ("StatsHUDModel_EndTimer" == code) {
			LogI << "ran out of time... dim the glyphs on the BCV";
			Notif::send("BlockCanvasModel_EndTimer");
		}

		// ScoreKeeper
		else if ("ScoreKeeper_Score" == code) {
			// extract latest score delta from sk
			std::shared_ptr<ScoreKeeperUpdateInfo> info = std::static_pointer_cast<ScoreKeeperUpdateInfo>(data);
			std::shared_ptr<BlockCanvasModelUpdateInfo> pInfo(new BlockCanvasModelUpdateInfo);
			pInfo->scoreUpdateDelta = info->scoreDelta; // ScoreKeeperUpdateInfo
			pInfo->curStreakLevel = info->curStreakLevel;
			Notif::send("BlockCanvasModel_PerBlockScoreDelta", pInfo);
		}

		// BlockCanvasView
		else if ("BlockCanvasView_FinishedPopAnimation" == code) {
			Notif::send("BlockCanvasModel_DoneAnimation");

		} else if ("BlockCanvasView_EncasementDowngraded" == code) {
			std::shared_ptr<BlockCanvasViewUpdateInfo> pInfo = std::static_pointer_cast<BlockCanvasViewUpdateInfo>(data);
			pImpl->reduceEncasementLevelAtIndex(pInfo->blockIndex,
												pImpl->encasementLevelAtIndex(pInfo->blockIndex));

		} else if ("BlockCanvasView_ObstructionRemoved" == code) {
			reportObstructionRemoved();

		}

		// CopyText
		else if ("CopyText_LoadedString" == code) {
			updateBlockChain(GameState::getInstance().copyText().getVisibleString());
			Notif::send("BlockCanvasModel_Load");

		} else if ("CopyText_Advance" == code) {
			updateBlockChain(GameState::getInstance().copyText().getVisibleString());
			Notif::send("BlockCanvasModel_Advance");

		} else if ("CopyText_AdvanceWithSpace" == code) {
			updateBlockChain(GameState::getInstance().copyText().getVisibleString());
			Notif::send("BlockCanvasModel_AdvanceWithSpace");

		} else if ("CopyText_Preadvance" == code) {
			Notif::send("BlockCanvasModel_Highlight");

		} else if ("CopyText_Mistake" == code) {
			updateBlockChain(GameState::getInstance().copyText().getVisibleString());
			Notif::send("BlockCanvasModel_Mistake");

		}
	}



#pragma mark - Perform operations on GameState based on events happening in the block canvas

	void BlockCanvasModel::reportObstructionRemoved()
	{
		GameState &gs(GameState::getInstance());
		// ... time to tell the GameState player that it has a new froggie
		gs.player().addToCurrencyOwned(1);
		gs.player().syncStatsToDB(gs.scoreKeeper());
		Notif::send("BlockCanvasModel_CurrencyCollected");
	}


#pragma mark - Convenience Methods from GameState
	
	size_t BlockCanvasModel::unitsToAdvance() const
	{
		return GameState::getInstance().copyText().unitsToAdvance();
	}


	size_t BlockCanvasModel::unitsWithMistake() const
	{
		return GameState::getInstance().copyText().unitsToMistakeHL();
	}


	size_t BlockCanvasModel::totalBlocksDisplayable() const
	{
		return GameState::getInstance().copyText().getBlocksPerLine();
	}


	size_t BlockCanvasModel::visibleStringSize() const
	{
		return GameState::getInstance().copyText().getVisibleString().size();
	}


	float BlockCanvasModel::slideBackTime() const
	{
		size_t playerLevel = GameState::getInstance().player().getLevel();
		return PlayerLevel::slideBackTimeForPlayerLevel(playerLevel);
	}


#pragma mark - Initialize the Row of Blocks

	void BlockCanvasModel::updateBlockChain(const GlyphString &str)
	{
		pImpl->row1Blox.setString(str);
		// now set properties for the blockmodels in the row1blox here using info available
		// to BCM
		// use itemAt() and size()

		for (size_t i = 0; i < pImpl->row1Blox.size(); i++) {
			BlockModel &blockM(pImpl->row1Blox.itemAt(i));

			blockM.setHasObstruction(pImpl->hasObstructionAtIndex(i));
			blockM.setEncased(pImpl->encasementLevelAtIndex(i) > 0);

			if (blockM.hasObstruction()) {
				float obstructionDelay = utilities::randomFloat(minObstructionDelay, maxObstructionDelay);
				LogD << "Assigning an obstruction intro delay of " << obstructionDelay <<
					" to block with index " << i;
				blockM.setObstructionIntroductionDelay(obstructionDelay);
				blockM.setObstructionTimeDuration(obstructionDuration);
			}
		}
	}
}
