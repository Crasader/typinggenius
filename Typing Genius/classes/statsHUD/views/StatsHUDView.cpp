//
//  StatsHUDView.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "StatsHUDView.h"
#include "GameState.h"
#include "MCBCallLambda.h"
#include "ScoreKeeper.h"
#include "ScreenResolutionHelper.h"
#include "StatsHUD.h"
#include "Utilities.h"
#include "VisualEffectsHelper.h"
#include "Player.h"
#include "StatsHUDModel.h"
#include "SimpleAudioEngine.h"
#include "DebugSettingsHelper.h"

namespace ac {

	using std::shared_ptr;
	using std::static_pointer_cast;

	// at this level and below, the timer goes RED
	const int WarningPctUpperBound = 20; // has to be in range 0..100

	const char *DefaultFont = "fontatlases/avenir-condensed.fnt"; // for the headline
	const char *TimerFont = "fontatlases/avenir-timer.fnt";
	const char *GenericBlockSprite = "stats/generic-block.png";
	
	const char *TimeBarSpriteImage = "stats/timebar.png";

	const char *LevelGaugeBGImage = "stats/level-gauge-bg.png";
	const char *LevelGaugeImage = "stats/level-gauge.png";

	// sound effects
	const char *SFXCombo = "sfx/combo.mp3";
	const char *SFXLevelUp = "sfx/level-up.mp3";
	const char *SFXTimerWarn = "sfx/time-running-out.mp3";
	const char *SFXGameOver = "sfx/game-over.mp3";

	const ccColor3B TimerBarAccesssoryColorBad = ccc3(127, 0, 0);
	const ccColor3B TimerBarAccesssoryColorGood = ccc3(0, 127, 0);
	
	using MCBPlatformSupport::MCBCallLambda;

#pragma mark - pImpl Definition
	
	struct StatsHUDViewImpl : CCObject
	{
		StatsHUDViewImpl(StatsHUDView *shView) :
		shView(shView),
		layerColor(),
		modelStatsUpdateSignalConnection(),
		totalBlocksInProgress(0),
		blocksActuallyAddedInProgress(0),
		blockSizeInProgress(CCSizeZero),
		progressBorder(), progressTimer(), timerAnimationBar(),
		headlineLabel(), subHeadlineLabel(), timerLabel(), scoreAtStartOfLevel(),
		activeAccuracyLabel(), activeScoreLabel(), playerLevelLabel(), currencyCountLabel(),
		visualEffectsHelper(), levelGauge(), levelGaugeBG(), maxTimeThisLevel(0)
		{
		}

		StatsHUDView *shView;
		
		CCLayerColor *layerColor;

		// should be animated.
		CCLabelBMFont *headlineLabel;
		CCLabelBMFont *subHeadlineLabel;

		// time progress
		CCSprite *progressBorder;
		CCProgressTimer *progressTimer;
		CCProgressTimer *timerAnimationBar;
		CCLabelBMFont *timerLabel;

		// level gauge
		CCSprite *levelGaugeBG;
		CCProgressTimer *levelGauge;


		CCLabelBMFont *activeScoreLabel;
		CCLabelBMFont *activeAccuracyLabel;
		CCLabelBMFont *playerLevelLabel;
		CCLabelBMFont *currencyCountLabel;

		VisualEffectsHelper visualEffectsHelper;

		CCTimer *timer;
		
		float maxTimeThisLevel;

		// signal for model
		sign_conn_t modelStatsUpdateSignalConnection;
		
		void switchColor();
		
		// for block progress
		size_t totalBlocksInProgress;// all the blocks if they were full
		CCSize blockSizeInProgress;

		size_t scoreAtStartOfLevel;
		
		size_t blocksActuallyAddedInProgress;

		CCAction *fadeInBlock(CCSprite *block);

		// adds the blocks on the top to provide indication of progress
		void updateProgress(const StatsHUDModel &model);
		
		void updatePlayerLevel(const StatsHUDModel &model);
		void updateCurrencyCount(const StatsHUDModel &model);

		void showTimers(bool show);
		void fadeOffHeadline();
		void showGameCompleteStats(const StatsHUDModel &);
		void showNewLevelAnnouncement(const StatsHUDModel &);
		void updateScoreAndAccuracy(const StatsHUDModel &, int delta = 0);
		void reportStreakFinished(const StatsHUDModel &, const StatsHUDModelUpdateInfo &);
		void reportBlockedBlock(const StatsHUDModel &);

		void updateLevelProgress(const StatsHUDModel &, float);

		void setScoreLabelValue(CCNode *node, void *value);
	};
	
#pragma mark - Lifetime
	
	
	StatsHUDView::StatsHUDView()
	{
		LogI << "Inside StatsHUDView Constructor";
		pImpl.reset(new StatsHUDViewImpl(this));
	}
	
	
	StatsHUDView::~StatsHUDView()
	{
		LogI << "Inside StatsHUDView Destructor!";
	}
	
	
	bool StatsHUDView::init()
	{
		bool ret = false;
		do {
			this->ignoreAnchorPointForPosition(false);
			this->setAnchorPoint(CCPoint(0, 1));
			ret = true;
		} while (0);
		return ret;
	}
	
	
	void StatsHUDView::onEnter()
	{
		LogI << "Entering StatsHUDView";

		// in pct
		CCSize sz(utilities::visibleSize());

		const float width = sz.width;
		const float height = utilities::isTabletFormFactor() ? 320 : 140;

		CCSize hudSize(width, height);
		
		this->setContentSize(hudSize);
		// note: this is in percent!
		// TODO - move the values to the configuration file
		NeutralPoint point = { 0, 100 };
		this->setPosition(rel2WindowPoint(point));

		// compute for the size of each block that makes up the progress bar.
		pImpl->totalBlocksInProgress = StatsHUD::getInstance().model()->totalBlocksDisplayable();
		float scaledBlockWidth = this->getContentSize().width / pImpl->totalBlocksInProgress;
		pImpl->blockSizeInProgress = CCSize(scaledBlockWidth, scaledBlockWidth);

		// timer
		const float progressX = utilities::isTabletFormFactor() ? 30 : 15;
		const CCPoint progressPosition(progressX, /*height - scaledBlockWidth - 15*/ height - 30);
		const CCPoint anchorPoint(0, 0.5);

		pImpl->progressBorder = CCSprite::create("stats/timebar-bg.png");
		pImpl->progressBorder->setPosition(progressPosition);
		pImpl->progressBorder->setAnchorPoint(anchorPoint);

		pImpl->progressTimer = CCProgressTimer::create(CCSprite::create(TimeBarSpriteImage));
		pImpl->progressTimer->setPosition(progressPosition);
		pImpl->progressTimer->setAnchorPoint(anchorPoint);

		pImpl->progressTimer->setType(kCCProgressTimerTypeBar);
		pImpl->progressTimer->setMidpoint(ccp(0, 0.5));
		pImpl->progressTimer->setBarChangeRate(ccp(1, 0));
		pImpl->progressTimer->setPercentage(100);
		
		
		// mistake bar
		CCSprite *timerAnimationBarSprite = CCSprite::create(TimeBarSpriteImage);
		timerAnimationBarSprite->setColor(TimerBarAccesssoryColorBad); // could be "good"
		pImpl->timerAnimationBar = CCProgressTimer::create(timerAnimationBarSprite);
		
		pImpl->timerAnimationBar->setPosition(progressPosition);
		pImpl->timerAnimationBar->setAnchorPoint(anchorPoint);
		
		pImpl->timerAnimationBar->setType(kCCProgressTimerTypeBar);
		pImpl->timerAnimationBar->setMidpoint(ccp(0, 0.5));
		pImpl->timerAnimationBar->setBarChangeRate(ccp(1, 0));
		pImpl->timerAnimationBar->setPercentage(100); // going to be hidden and reset at some point.
		pImpl->timerAnimationBar->setVisible(false);


		const float progressMargin = utilities::isTabletFormFactor() ? 10 : 5;
		pImpl->timerLabel = CCLabelBMFont::create("00:00:00", TimerFont);
		pImpl->timerLabel->setAnchorPoint(anchorPoint);
		pImpl->timerLabel->setPosition(pImpl->progressTimer->getContentSize().width + progressPosition.x +
									   progressMargin,
									   progressPosition.y);

		this->addChild(pImpl->progressBorder);
		this->addChild(pImpl->timerAnimationBar);
		this->addChild(pImpl->progressTimer);
		this->addChild(pImpl->timerLabel);


		// level gauge
		const CCPoint levelGaugePosition(hudSize.width - 10, hudSize.height * 0.5);
		const CCPoint levelGaugeAnchorPoint(ccp(1, 0.5));

		pImpl->levelGaugeBG = CCSprite::create(LevelGaugeBGImage);
		pImpl->levelGaugeBG->setPosition(levelGaugePosition);
		pImpl->levelGaugeBG->setAnchorPoint(levelGaugeAnchorPoint);

		pImpl->levelGauge = CCProgressTimer::create(CCSprite::create(LevelGaugeImage));
		pImpl->levelGauge->setPosition(levelGaugePosition);
		pImpl->levelGauge->setAnchorPoint(levelGaugeAnchorPoint);

		pImpl->levelGauge->setType(kCCProgressTimerTypeBar);
		pImpl->levelGauge->setMidpoint(ccp(0.5, 0));
		pImpl->levelGauge->setBarChangeRate(ccp(0, 1));
		pImpl->levelGauge->setPercentage(0);

		this->addChild(pImpl->levelGaugeBG);
		this->addChild(pImpl->levelGauge);

		// the headline and subheadline
		pImpl->headlineLabel = CCLabelBMFont::create("Type off the leftmost block to begin", DefaultFont);
		this->addChild(pImpl->headlineLabel);
		pImpl->subHeadlineLabel = CCLabelBMFont::create(" ", TimerFont);
		this->addChild(pImpl->subHeadlineLabel);

		float headlineLabelHeight(pImpl->headlineLabel->getContentSize().height);
		float subHeadlineLabelHeight(pImpl->subHeadlineLabel->getContentSize().height);

		// the y-offset that would allow the two labels when stacked together, to be centered
		// vertically within the hud
		float y = 0.5 * (hudSize.height - (headlineLabelHeight + subHeadlineLabelHeight));

		// we're talking anchorPoints of (0.5, 0.5) here
		float headlineMidpointY = y + subHeadlineLabelHeight + (headlineLabelHeight * 0.5);
		float subHeadlineMidpointY = y + (subHeadlineLabelHeight * 0.5);

		pImpl->headlineLabel->setPosition(ccp(hudSize.width / 2, headlineMidpointY));
		pImpl->subHeadlineLabel->setPosition(ccp(hudSize.width / 2, subHeadlineMidpointY));

		// score and accuracy
		pImpl->activeScoreLabel = CCLabelBMFont::create(" " /* [1] */, TimerFont);
		// [1] the space is needed as the positioning of the other labels depend on this one
		pImpl->activeScoreLabel->setAnchorPoint(anchorPoint);
		CCPoint scorePosition(progressX, progressPosition.y - pImpl->progressTimer->getContentSize().height - 5);
		pImpl->activeScoreLabel->setPosition(scorePosition);
		this->addChild(pImpl->activeScoreLabel);

		pImpl->activeAccuracyLabel = CCLabelBMFont::create(" ", TimerFont);
		pImpl->activeAccuracyLabel->setAnchorPoint(anchorPoint);
		pImpl->activeAccuracyLabel->setPosition(scorePosition.x, scorePosition.y - pImpl->activeScoreLabel->getContentSize().height);
		this->addChild(pImpl->activeAccuracyLabel);

		pImpl->playerLevelLabel = CCLabelBMFont::create(" ", TimerFont);
		pImpl->playerLevelLabel->setAnchorPoint(anchorPoint);
		pImpl->playerLevelLabel->setPosition(scorePosition.x,
				pImpl->activeAccuracyLabel->getPosition().y - pImpl->activeAccuracyLabel->getContentSize().height);
		this->addChild(pImpl->playerLevelLabel);


		pImpl->currencyCountLabel = CCLabelBMFont::create(" ", TimerFont);
		pImpl->currencyCountLabel->setAnchorPoint(anchorPoint);
		pImpl->currencyCountLabel->setPosition(scorePosition.x,
											 pImpl->playerLevelLabel->getPosition().y - pImpl->playerLevelLabel->getContentSize().height);
		this->addChild(pImpl->currencyCountLabel);

		pImpl->showTimers(false); // hide timer until started // AC 2013.11.13: show for now.

		// now that the Stat HUD components have been fully initialized, ask the model to
		// give you the values for level, score and accuracy.
		const StatsHUDModel &model(*StatsHUD::getInstance().model());
		pImpl->updatePlayerLevel(model);
		pImpl->updateCurrencyCount(model);
		pImpl->updateScoreAndAccuracy(model, 0);

		CCLayer::onEnter();

	}
	
	
	void StatsHUDView::onExit()
	{
		LogI << "Exiting StatsHUDView";
		CCLayer::onExit();
	}
	
	
#pragma mark - Access the labels
	
	void StatsHUDView::setCharsTypedCount(size_t typedCharCount)
	{
	}


	void StatsHUDView::setAccuracyValue(size_t typedCharCount, size_t mistakesCount)
	{
		float accuracyPct;

		if (typedCharCount == 0) {
			accuracyPct = 0;
		} else {
			accuracyPct = 100.0f * (typedCharCount - mistakesCount) / (float)typedCharCount;
		}
	}
	
	
#pragma mark - Cocos2d Timer
	
	// NOTE: this only affects the timer label besides the timer bar. It uses the GameState timer object. The timer
	// bar is controlled independently by running an action on it.
		
	void StatsHUDView::startTimerWithCountdown()
	{
		this->schedule(schedule_selector(StatsHUDView::timerUpdate), 0.1);
	}
	
	
	void StatsHUDView::timerUpdate(float delta)
	{
		long timeRemainVal(GameState::getInstance().getTimeRemaining()); // millis
		string timeRemaining(GameState::formattedTimeVal(timeRemainVal));
		pImpl->timerLabel->setString(timeRemaining.c_str());
	}


	// this is the real timer.
	void StatsHUDView::startTimerCountdown(float seconds, float startPercent = 100.0f) // shouldn't i ask GameState for the seconds here?
	{
		LogI << ":: starting timer countdown at " << seconds << " sec, startPercent = " << startPercent;
		
		pImpl->progressTimer->stopAllActions();
		pImpl->progressTimer->getSprite()->setColor(ccc3(255, 255, 255)); // this resets the color if that's necessary

		stopTimerCountdown();
		pImpl->showTimers(true);
		
		if (startPercent == 100.0f) { // initial setting of the max time (this func can be called when deducting time)
			pImpl->maxTimeThisLevel = seconds;
		}
		
		startTimerWithCountdown(); // this only updates the GS version of the timer.
		
		CCArray *actionSequence = CCArray::create();
		
		CCCallFunc *timerExpiredFunc = CCCallFunc::create(this, callfunc_selector(StatsHUDView::timerExpiredCallback));
		
		auto setTimerToWarnMode = MCBCallLambda::create([=] () {
			pImpl->progressTimer->getSprite()->setColor(ccc3(255, 0, 0)); // 255, 255, 255 is neutral

		});

		auto playTimerWarnModeSFX = MCBCallLambda::create([=] () {
			if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXTimerWarn);
			}
		});
		
		if (startPercent > WarningPctUpperBound) {
			float duration1 = seconds * (startPercent - WarningPctUpperBound) / startPercent;
			float duration2 = seconds * WarningPctUpperBound / startPercent;

			CCFiniteTimeAction *doPhase1 = CCProgressFromTo::create(duration1, startPercent, WarningPctUpperBound);
			CCSpawn *doPhase2 = CCSpawn::create(setTimerToWarnMode, playTimerWarnModeSFX,
												CCProgressFromTo::create(duration2, WarningPctUpperBound, 0),
												NULL);
			
			actionSequence->addObject(doPhase1);
			actionSequence->addObject(doPhase2);
			
		} else {
			// skip the niceties
			actionSequence->addObject(CCSpawn::create(setTimerToWarnMode, CCProgressFromTo::create(
																								   // seconds * startPercent / 100.0f,
																								   seconds,
																								   startPercent, 0), NULL));
		}
		actionSequence->addObject(timerExpiredFunc);
		pImpl->progressTimer->runAction(CCSequence::create(actionSequence));
	}
	

	// live change to active timer
	void StatsHUDView::resetTimeInCountdown(float newDuration, const TimerResetAnimationType &animationType)
	{
		// current percentage before the new time
		float startPercentage = pImpl->progressTimer->getPercentage();



		// I'll be using the mistake bar for the animations when adding time as well.
		pImpl->timerAnimationBar->stopAllActions();

		if (TimerResetAnimationType::Decrease == animationType) {

			// these set of animations work for mistakes
			CCDelayTime *delayTime = CCDelayTime::create(0.15);
			pImpl->timerAnimationBar->setPercentage(startPercentage);
			CCProgressFromTo *runDownQuickly = CCProgressFromTo::create(0.3, startPercentage, 0);
			auto hideTheBar = MCBCallLambda::createWithDelay(0.1, [=] () {
				pImpl->timerAnimationBar->setVisible(false);
			});
			pImpl->timerAnimationBar->setVisible(true);
			pImpl->timerAnimationBar->getSprite()->setOpacity(255);
			pImpl->timerAnimationBar->runAction(CCSequence::create(delayTime,
															CCEaseIn::create(runDownQuickly, 2),
															hideTheBar, NULL));
			pImpl->timerAnimationBar->setColor(TimerBarAccesssoryColorBad);

			// restart timer with the new duration
			startTimerCountdown(newDuration, 100.0f * newDuration / pImpl->maxTimeThisLevel /* start percentage */);

		} else if (TimerResetAnimationType::Increase == animationType) {

			const float animationDuration = 0.2;

			newDuration -= animationDuration; // spend this time doing the pre-animation

			const float newPercentage = 100.0f * newDuration / pImpl->maxTimeThisLevel;
			CCFiniteTimeAction *runUp = CCEaseIn::create(
				CCProgressFromTo::create(animationDuration, startPercentage, newPercentage), 2);

			pImpl->progressTimer->runAction(CCSequence::create(runUp, MCBCallLambda::create([=] {
				startTimerCountdown(newDuration, 100.0f * newDuration / pImpl->maxTimeThisLevel /* start percentage */);
			}), NULL));

		}
	}
	
	
	void StatsHUDView::stopTimerCountdown()
	{
		this->unschedule(schedule_selector(StatsHUDView::timerUpdate));
	}


	void StatsHUDView::timerExpiredCallback()
	{
		LogI << boost::format("SHV: timer expired! (animation)");
		// inform the model.
		Notif::send("StatsHUDView_TimerExpired");
		this->unschedule(schedule_selector(StatsHUDView::timerUpdate));
	}
	
	
#pragma mark - Event processing


	void StatsHUDView::notifCallback(const string &code, shared_ptr<void> data)
	{
		const StatsHUDModel &model(*(StatsHUD::getInstance().model()));

		// respond to SHModel events
		if ("StatsHUDModel_AddTime" == code) {

			resetTimeInCountdown(GameState::getInstance().getTimeRemaining() / 1000.0, TimerResetAnimationType::Increase);

		} else if ("StatsHUDModel_AllCleared" == code) {

			stopTimerCountdown();
			pImpl->showGameCompleteStats(model);

		} else if ("StatsHUDModel_Blocked" == code) {

			pImpl->reportBlockedBlock(model);

		} else if ("StatsHUDModel_CurrencyCollected" == code) {

			pImpl->updateCurrencyCount(model);

		} else if ("StatsHUDModel_CurrencyConsumed" == code) {

			pImpl->updateCurrencyCount(model);

		} else if ("StatsHUDModel_EndTimer" == code) {

			// force it to go to zero regardless of what GS::TimeRemaining says
			pImpl->timerLabel->setString(GameState::formattedTimeVal(0).c_str());
			pImpl->showGameCompleteStats(model);

		} else if ("StatsHUDModel_LevelProgressUpdate" == code) {

			shared_ptr<StatsHUDModelUpdateInfo> pInfo = static_pointer_cast<StatsHUDModelUpdateInfo>(data);
			pImpl->updateLevelProgress(model, pInfo->levelProgressDelta);
			pImpl->updatePlayerLevel(model);

		} else if ("StatsHUDModel_Mistake" == code) {

			resetTimeInCountdown(GameState::getInstance().getTimeRemaining() / 1000.0, TimerResetAnimationType::Decrease);
			pImpl->updateScoreAndAccuracy(model);

		} else if ("StatsHUDModel_NewLevel" == code) {

			startTimerCountdown(model.getTimerLength()); // restarts the timer.
			pImpl->showNewLevelAnnouncement(model);

		} else if ("StatsHUDModel_ScoreOrAccuracyUpdate" == code) {

			shared_ptr<StatsHUDModelUpdateInfo> pInfo = static_pointer_cast<StatsHUDModelUpdateInfo>(data);
			pImpl->updateScoreAndAccuracy(model, pInfo->scoreDelta);

		} else if ("StatsHUDModel_StartTimer" == code) {

			startTimerCountdown(GameState::getInstance().getTimeRemaining() / 1000.0f);
			pImpl->scoreAtStartOfLevel = GameState::getInstance().player().getTotalScore();
			pImpl->updatePlayerLevel(model);
			pImpl->updateCurrencyCount(model);
			pImpl->fadeOffHeadline();

		} else if ("StatsHUDModel_StreakFinished" == code) {

			if (!GameState::getInstance().isGameOver()) {
				shared_ptr<StatsHUDModelUpdateInfo> pInfo = static_pointer_cast<StatsHUDModelUpdateInfo>(data);
				pImpl->reportStreakFinished(model, *pInfo);
			}

		} else if ("StatsHUDModel_UpdateProgress" == code) {
			// do nothing...

		} else if ("GameState_Timer_DeductTime" == code) {

			// play a sound when the time deduction directly causes timer to goes below the threshold
			shared_ptr<GameStateTimerEventInfo> pInfo = static_pointer_cast<GameStateTimerEventInfo>(data);
			const float newTimeRemaining = GameState::getInstance().getTimeRemaining() / 1000.0f;
			const float oldTimeRemaining = newTimeRemaining + pInfo->delta; // time amount prior to the deduction
			const float timeRemainingWarnThreshold = WarningPctUpperBound * pImpl->maxTimeThisLevel / 100.0f;

			if (newTimeRemaining <= timeRemainingWarnThreshold && oldTimeRemaining > timeRemainingWarnThreshold) {
				if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXTimerWarn);
				}
			}

		}
	}


	void StatsHUDViewImpl::updateProgress(const StatsHUDModel &model)
	{
		// this info is not reliably when you're eliminating blocks very rapidly // increasing is all you can guarantee
		// since copytext already has advanced by 1 (or whatever offset) when you call it at this point, we subtract by 1
		size_t copyTextOffset(model.blocksConsumedSoFar());
		size_t blocksToAdd = copyTextOffset - blocksActuallyAddedInProgress;

		const int startRGB = 150;
		const int rGBLength = 97; // number of steps

		for (size_t i = 0; i < blocksToAdd; i++)
		{
			// add the blocks here. I should be able to control how many there are in run time.
			CCSprite *genericBlock = CCSprite::create(GenericBlockSprite);
			utilities::scaleNodeToSize(genericBlock, blockSizeInProgress);
			genericBlock->setAnchorPoint(CCPointZero);
			shView->addChild(genericBlock);

			// this value is accurate, but the positions may overlap of copyTextOffset returns the same (old?) value.

			CCPoint position(shView->progressPositionAtIndex(blocksActuallyAddedInProgress));
			genericBlock->setPosition(position);

			// do the fadein animation on them
			CCAction *fadeUnit = fadeInBlock(genericBlock);
			genericBlock->runAction(fadeUnit);

			// change color so that the blocks appear in a gradient
			int rgb = startRGB + rGBLength * ((float) blocksActuallyAddedInProgress / (float) totalBlocksInProgress);
			genericBlock->setColor(ccc3(rgb, rgb, rgb));

			blocksActuallyAddedInProgress++;
		}
	}
	
	
	CCSize StatsHUDView::progressBlockSize() const
	{
		return pImpl->blockSizeInProgress;
	}
	
		
	CCPoint StatsHUDView::progressPositionAtIndex(size_t idx) const
	{
		return ccp(
			idx * pImpl->blockSizeInProgress.width,
			this->getContentSize().height - pImpl->blockSizeInProgress.height
		);
	}


#pragma mark - Headline actions

	void StatsHUDViewImpl::fadeOffHeadline()
	{
		headlineLabel->runAction(CCFadeOut::create(0.5));
		subHeadlineLabel->runAction(CCFadeOut::create(0.5));
	}
	
	void StatsHUDViewImpl::showNewLevelAnnouncement(const StatsHUDModel &model)
	{
		CCArray *actionSequence = CCArray::create();
		size_t playerLevel(model.getPlayerLevel());

		if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXLevelUp);
		}
		
		static boost::format levelAnnounceStr("Good Job! Now at level %d");
		levelAnnounceStr % playerLevel;
		
		headlineLabel->setString(levelAnnounceStr.str().c_str());
		actionSequence->addObject(CCFadeIn::create(0.3));
		actionSequence->addObject(CCDelayTime::create(0.7));
		actionSequence->addObject(CCFadeOut::create(0.5));
		
		headlineLabel->runAction(CCSequence::create(actionSequence));
	}


	void StatsHUDViewImpl::showGameCompleteStats(const StatsHUDModel &model)
	{
		// hide the blocks, show the timer.
		if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXGameOver);
		}
		
		float timeRemaining = GameState::getInstance().getTimeRemaining() / 1000.0f;

		// for the headline
		static boost::format successMessage("Done in %.1f sec!");
		static boost::format failureMessage("Try again! Your score: %d");

		// this is for the statsHud label
		static boost::format scoreFmt("SCORE: %s");

		// score this round: with all the bonuses (not including the player total score)

		// animation order: oldScore + sessionScore.score -> +bonusForTimeRemaining -> +bonusForAccuracy;
		const SessionScore &sessScore(GameState::getInstance().scoreKeeper().getSessionScore());

		size_t preBonusPostGameScore = this->scoreAtStartOfLevel + sessScore.score;

		string message;

		// what if there's no bonus for time remaining?
		this->activeScoreLabel->setString((scoreFmt % sessScore.totalScore()).str().c_str());

		// actions here to be performed by the headlineLabel.
		CCArray *actionsInSequence = CCArray::create();

		if (timeRemaining <= 0.0f) { // fail: ran out of time

			message = (failureMessage % preBonusPostGameScore).str();

			headlineLabel->setString(message.c_str());

			actionsInSequence->addObject(CCDelayTime::create(1));
			actionsInSequence->addObject(MCBCallLambda::create([=] () {
				// "Press any key to continue"
				subHeadlineLabel->setOpacity(0);
				subHeadlineLabel->setString("PRESS A KEY TO CONTINUE");
				subHeadlineLabel->runAction(CCFadeIn::create(1));
				Notif::send("StatsHUDView_PostGameSubheadlineShown");
			}));

		} else { // success! 2014-01-07 not really used currently

			float originalCountdownTotal = model.getTimerLength(); // in seconds
			float percentage = this->progressTimer->getPercentage();
			float secElapsed = originalCountdownTotal - originalCountdownTotal * (percentage / 100.0f);

			// success: finished a level
			// message will be fed into another boost::format object
			message = (successMessage % secElapsed).str();

			const string messageFmt(message + " Score: %d");
			const size_t steps = 20; // this is only the max, could be less

			// bonus for time remaining. Coordinate headline and subheadline animations.
			if (sessScore.bonusForTimeRemaining > 0) {

				const float duration = 2.2f; // intended duration for trickle and drain (done simultaneously)

				// this gives a compound "sequence" type action consisting of a headline animation and a callback to trigger
				// the corresponding subheadline action

				// 1a. trickle the score into intended value
				// format string has to get extra param
				const size_t low = preBonusPostGameScore;
				const size_t high = low + sessScore.bonusForTimeRemaining;
				vector<string> scoreSteps(this->visualEffectsHelper.stepwiseNumericValuesBasedOnRange
										  (low, high, steps, messageFmt));
				float delay = 0; // delay per step used in trickleEffect
				float timePerTrickleStep = 0; // you can only guess but on simulator even zero is too small
				if (scoreSteps.size() > 1) {
					delay = (duration - timePerTrickleStep * scoreSteps.size()) / (scoreSteps.size() - 1);
				}
				CCFiniteTimeAction *trickle = this->visualEffectsHelper.trickleEffectOnNumericLabel(scoreSteps, delay);

				// 1b. drain the timer bar
				auto drain = MCBCallLambda::create([=] () {
					this->visualEffectsHelper.runDrainEffectOnTimer(this->progressTimer, duration);
				});

				// 1c. duration for this is defined in runGalleryHeadline
				auto doSHLGallery = MCBCallLambda::create([=] () {
					const float pauseRatio = 0.75; // the rest of the duration split up between appear / disappear
					boost::format fmt("Time Remaining (%s): +%d pts");
					fmt % GameState::formattedTimeVal(1000.0f * (originalCountdownTotal - secElapsed), false) %
					sessScore.bonusForTimeRemaining;
					this->visualEffectsHelper.runGalleryHeadline(subHeadlineLabel, fmt.str(), duration, pauseRatio);
				});

				actionsInSequence->addObject(CCSpawn::create(doSHLGallery, trickle, drain, NULL));
			}

			// if there's a bonus for accuracy....
			if (sessScore.bonusForAccuracy > 0) {

				const float duration = 2.2f; // intended duration for trickle

				// format string has to get extra param
				const size_t low = preBonusPostGameScore + sessScore.bonusForTimeRemaining;
				const size_t high = low + sessScore.bonusForAccuracy;
				vector<string> scoreSteps(this->visualEffectsHelper.stepwiseNumericValuesBasedOnRange
										  (low, high, steps, messageFmt));
				CCFiniteTimeAction *trickle = this->visualEffectsHelper.trickleEffectOnNumericLabel(scoreSteps);

				auto doSHLGallery = MCBCallLambda::create([=] () {
					const float pauseRatio = 0.75; // the rest of the duration split up between appear / disappear
					boost::format fmt("Accuracy Bonus (%.1f%%): +%d pts");
					fmt % (GameState::getInstance().scoreKeeper().getAccuracy() * 100) % sessScore.bonusForAccuracy;
					this->visualEffectsHelper.runGalleryHeadline(subHeadlineLabel, fmt.str(), duration, pauseRatio);
				});


				if (actionsInSequence->count() > 0) {
					// already have another action before this.. insert a delay
					actionsInSequence->addObject(CCDelayTime::create(1));
				}
				actionsInSequence->addObject(CCSpawn::create(doSHLGallery, trickle, NULL));
			}

			if (actionsInSequence->count() > 0) {
				actionsInSequence->addObject(CCDelayTime::create(1));
			}

			// "Press any key to continue"
			actionsInSequence->addObject(MCBCallLambda::create([=] () {
				subHeadlineLabel->setString("PRESS A KEY TO CONTINUE");
				subHeadlineLabel->setOpacity(0);
				subHeadlineLabel->runAction(CCFadeIn::create(0.7));
				Notif::send("StatsHUDView_PostGameSubheadlineShown");
			}));
		}
		headlineLabel->runAction(CCSpawn::create(CCFadeIn::create(0.5), CCSequence::create(actionsInSequence), NULL));
	}


	void StatsHUDViewImpl::reportBlockedBlock(const StatsHUDModel &model)
	{
		CCArray *actionSequence = CCArray::create();

		headlineLabel->setOpacity(0);
		headlineLabel->stopAllActions();

		headlineLabel->setString("Tap and hold to melt the ice block");
		actionSequence->addObject(CCFadeIn::create(0.2));
		actionSequence->addObject(CCDelayTime::create(0.4));
		actionSequence->addObject(CCFadeOut::create(0.3));

		headlineLabel->runAction(CCSequence::create(actionSequence));
	}


	void StatsHUDViewImpl::updateScoreAndAccuracy(const StatsHUDModel &model, int scoreDelta)
	{
		static boost::format scoreFmt("SCORE: %s");
		static boost::format accuracyFmt("ACCURACY: %.1f%%");

		// accuracy
		accuracyFmt % (100 * model.getAccuracy());
		this->activeAccuracyLabel->setString(accuracyFmt.str().c_str());

		// Score
		static const size_t iterations = 10;
		int newScore = model.getScore();
		int origScore = newScore - scoreDelta;

		if (scoreDelta > 0) {
			vector<string> stringValues;
			vector<string> ranges = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(origScore, newScore, iterations, "%d");
			for (string range : ranges) {
				stringValues.push_back((scoreFmt % range).str());
			}
			this->activeScoreLabel->runAction(this->visualEffectsHelper.trickleEffectOnNumericLabel(stringValues, 0.05));
		} else {
			this->activeScoreLabel->setString((scoreFmt % newScore).str().c_str());
		}
	}
	
	
	void StatsHUDViewImpl::reportStreakFinished(const StatsHUDModel &model, const StatsHUDModelUpdateInfo &info)
	{
		if (info.curStreakLevel < 5) return;

		// slope-based computation
//		const size_t lowStreakLevel = 5;
//		const size_t highStreakLevel = 12;
//
//		const float lowVolumeMult = 0.05; // multiplied to 1
//		const float highVolumeMult = 0.6;
//
//		const float volumeSlope = (highVolumeMult - lowVolumeMult) / (highStreakLevel - lowStreakLevel);
//		const float volume = lowVolumeMult + volumeSlope * (info.curStreakLevel - lowStreakLevel);
		// AC 2013.12.17: note: can't adjust volume in this way!

		CocosDenshion::SimpleAudioEngine *audio(CocosDenshion::SimpleAudioEngine::sharedEngine());

		// louder when streak is longer
		// audio->setEffectsVolume(volume);
		// LogI << boost::format("0. volume: %.2f") % audio->getEffectsVolume();
		if (!DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx")) {
			audio->playEffect(SFXCombo);
		}

		// restore old
		// audio->setEffectsVolume(1);
		// LogI << boost::format("1. volume: %.2f") % audio->getEffectsVolume();
		
		const size_t playerLevel = model.getPlayerLevel();
		const float cumulProgressBonus = PlayerLevel::cumulativeProgressBonusForStreakLevel(info.curStreakLevel, playerLevel);
		float cumulativeProgressBonus = 100.0f * cumulProgressBonus * PlayerLevel::levelProgressPerBlock(playerLevel);
		
		static boost::format fmt("Streak x %d. Next level bonus: ");
		fmt % info.curStreakLevel;
		
		this->headlineLabel->setOpacity(0);
		this->headlineLabel->setVisible(true);
		
		string finalFormat(fmt.str() + "+%.1f%%");
		
		vector<string> steps = VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(0, cumulativeProgressBonus, 10,
																					  finalFormat);
		CCFiniteTimeAction *trickle = this->visualEffectsHelper.trickleEffectOnNumericLabel(steps);
		
		
		auto hideIt = MCBCallLambda::createWithDelay(0.5, [=]() {
			this->headlineLabel->setOpacity(0);
		});
		
		CCSequence *actions = CCSequence::create(CCFadeIn::create(0.125), trickle,
												 hideIt, NULL);
		
		this->headlineLabel->runAction(actions);
	}


	void StatsHUDViewImpl::updateLevelProgress(const StatsHUDModel &model, float levelProgressDelta)
	{
		float levelProgress = GameState::getInstance().scoreKeeper().getLevelProgress();
		float ccNewPercentage = 100.0f * levelProgress;
		levelGauge->stopAllActions();
		float currentPercentage = levelGauge->getPercentage();
		
		LogI << "updating level progress from: " << currentPercentage << " to " << ccNewPercentage;
		
		const float duration = ccNewPercentage > currentPercentage ? 0.3 : 0.8;
		levelGauge->runAction(CCProgressFromTo::create(duration, currentPercentage, ccNewPercentage));
	}


	void StatsHUDViewImpl::setScoreLabelValue(CCNode *node, void *value) {
		CCLabelBMFont *label = static_cast<CCLabelBMFont *>(node); // this must always be true.
		string *s = static_cast<string *>(value);
		label->setString(s->c_str());
		CC_SAFE_DELETE(s);
	}


	void StatsHUDViewImpl::updatePlayerLevel(const StatsHUDModel &model)
	{
		static boost::format lvlFmt("LEVEL: %d");
		string levelLabelString = (lvlFmt % model.getPlayerLevel()).str();
		this->playerLevelLabel->setString(levelLabelString.c_str());
	}


	void StatsHUDViewImpl::updateCurrencyCount(const StatsHUDModel &model)
	{
		static boost::format currFmt("# OF FROGS: %d");
		string currAmountString = (currFmt % model.getCurrencyAmount()).str();
		this->currencyCountLabel->setString(currAmountString.c_str());
	}


	void StatsHUDViewImpl::showTimers(bool show)
	{
		progressBorder->setVisible(show);
		progressTimer->setVisible(show);
		timerLabel->setVisible(show);
	}


	void StatsHUDViewImpl::switchColor()
	{
		utilities::RGBByte rByte = utilities::randomRGBByte();
		this->layerColor->setColor(ccc3((GLubyte)rByte.r, (GLubyte)rByte.g, (GLubyte)rByte.b));
	}


	CCAction *StatsHUDViewImpl::fadeInBlock(CCSprite *block)
	{
		block->setOpacity(0); // initially
		return CCFadeIn::create(0.3);
	}
}