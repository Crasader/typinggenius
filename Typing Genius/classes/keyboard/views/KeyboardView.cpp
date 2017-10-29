//
//  keyboardview.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/15/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "KeyboardView.h"
#include "BlockTypesetter.h"
#include "CopyText.h"
#include "DebugSettingsHelper.h"
#include "GameState.h"
// #include "GlyphMap.h"
#include "Keyboard.h"
#include "KeyboardModel.h"
#include "KeypressTracker.h"
#include "KeyView.h"
#include "ScreenResolutionHelper.h"
#include "SimpleAudioEngine.h"
#include "TextureHelper.h"
#include "Utilities.h"
#include "Player.h"
#include <boost/algorithm/string.hpp>

namespace ac
{
	
#pragma mark - pImpl Structure
	
	const char *KeyboardSpriteFrameName = "kb-placeholder-bg.png";
	const char *SFXKeyDown = "sfx/soft-key-down.mp3";
	const char *SFXKeyUp = "sfx/soft-key-up.mp3";

	using std::set;

	struct KeyboardViewImpl
	{
		KeyboardViewImpl(KeyboardView *kbView) : keyViewMap(), layerColor(), spriteBatchNode(), kbBGSprite(),
		keyBounds(), keyLabelsSpriteBatchNode(), keyLabelSpriteMap(), kbView(kbView), touchLocations(),
		shouldPlaySFX(false)
		{
#ifndef BOOST_TEST_TARGET
			shouldPlaySFX = !DebugSettingsHelper::sharedHelper().boolValueForProperty("disable_sfx");
#endif
		}

		
		bool setupKeys(KeyboardView *kbView);
		void switchColor();

		KeyboardView *kbView;

		CCLayerColor *layerColor;
		// this layer is not the canvas, KeyboardView is.
		// this is only shown so we can see something.
				
		CCSpriteBatchNode *spriteBatchNode;
		CCSpriteBatchNode *keyLabelsSpriteBatchNode;
		
		CCSprite *kbBGSprite; // child of the spritebatchnode

		bool shouldPlaySFX;

		// string is label
		map<std::string, KeyView *> keyViewMap;

		// useful for identifying which keys are hit
		map<KeyView *, CCRect> keyBounds;

		// map for keylabels to their sprites
		map<string, CCSprite *> keyLabelSpriteMap;
		
		void changePressStateOfKeysToDown(set<string> keyLabels);

		inline bool isPointInKeyBounds(KeyView *keyView, const CCPoint &point) {
			return this->keyBounds[keyView].containsPoint(point);
		}
		
		inline KeypressTracker &keypressTracker() const { return GameState::getInstance().keypressTracker(); }

		// sends input signal to KPT, bypassing KBM
		void registerTouchEvents(CCSet *touches, TouchType type);

		// tracks the last location per touch.
		map<CCTouch *, CCPoint> touchLocations;

		void addGlyphsToKeys();


		void enableAltMode(bool enabled);
	};
	
	
#pragma mark - Lifetime
	
	KeyboardView::KeyboardView()
	{
		LogD << "Entered KeyboardView constructor...";
		pImpl.reset(new KeyboardViewImpl(this));
	}


	KeyboardView::~KeyboardView()
	{
		LogD << "Entered KeyboardView destructor...";
		pImpl->keyViewMap.clear();
		pImpl->keyBounds.clear();
		pImpl->keyLabelSpriteMap.clear();
		this->removeAllChildrenWithCleanup(true);
	}


	bool KeyboardView::init()
	{
		LogD << "Inside init!";
		bool ret = false;

		do {
			const float midpoint(utilities::visibleSize().width / 2);
			this->setAnchorPoint(CCPoint(0.5, 0));
			this->ignoreAnchorPointForPosition(false);
			this->setPosition(CCPoint(midpoint, 0));

			KeyboardSize kbSize = Keyboard::getInstance().model()->getKeyboardSize();

			pImpl->layerColor = CCLayerColor::create(ccc4(25, 25, 25, 255));
			CC_BREAK_IF(!pImpl->layerColor);

			CCSize layerSize(kbSize.width, kbSize.height);

			this->setContentSize(layerSize);
			pImpl->layerColor->setContentSize(layerSize);
			pImpl->layerColor->setPosition(0, 0);

			int zOrder = 0;
			int tag = 1;
			this->addChild(pImpl->layerColor, zOrder, tag);

			LogD << ">>> Creating sprite batch node for keyboard";
			pImpl->spriteBatchNode = CCSpriteBatchNode::create(utilities::textureForKeyboard(), 128);

			pImpl->spriteBatchNode->setPosition(CCPointZero);
			pImpl->spriteBatchNode->ignoreAnchorPointForPosition(false);
			pImpl->spriteBatchNode->setContentSize(layerSize);

			CC_BREAK_IF(!pImpl->spriteBatchNode);
			this->addChild(pImpl->spriteBatchNode);

			// keyboard background
			pImpl->kbBGSprite = CCSprite::createWithSpriteFrameName(KeyboardSpriteFrameName);
			CC_BREAK_IF(!pImpl->kbBGSprite);
			utilities::scaleNodeToSize(pImpl->kbBGSprite, layerSize);
			pImpl->kbBGSprite->ignoreAnchorPointForPosition(false);
			pImpl->kbBGSprite->setAnchorPoint(CCPointZero);
			pImpl->kbBGSprite->setPosition(CCPointZero);
			pImpl->spriteBatchNode->addChild(pImpl->kbBGSprite);

			LogD << ">>> Creating sprite batch node for keylabels";
			pImpl->keyLabelsSpriteBatchNode = CCSpriteBatchNode::create(utilities::textureForKeyLabels());

			pImpl->keyLabelsSpriteBatchNode->setPosition(CCPointZero);
			pImpl->keyLabelsSpriteBatchNode->ignoreAnchorPointForPosition(false);
			pImpl->keyLabelsSpriteBatchNode->setContentSize(layerSize);

			CC_BREAK_IF(!pImpl->keyLabelsSpriteBatchNode);
			this->addChild(pImpl->keyLabelsSpriteBatchNode);

			CC_BREAK_IF(!pImpl->setupKeys(this));

			ret = true;
		} while (0);
		return ret;
	}


	bool KeyboardViewImpl::setupKeys(KeyboardView *kbView)
	{
		// keyviews are created and added.
		// this method needs to be told the exact way in which the keys (including how many)
		// would be laid out.

		const shared_ptr<KeyboardModel>& model = Keyboard::getInstance().model();
		const std::vector<string> &keyLabels(model->getKeyLabels());

		// half the default key size
		// const float offset = 40;
		float offset = 1.2f * model->getKeySize().width;
		
		if (offset == 0.0f) {
			offset = 30.0f;
		}

		
		LogD << "KeyLabels size: " << keyLabels.size();
		// set them randomly within the area...
		for (int i = 0; i < keyLabels.size(); i++) {
			
			const string &keyLabel(keyLabels[i]);
			if (true /*model->hasGlyphForKeyLabel(keyLabel) */) {

				// const Glyph &glyph = model->getGlyphForKeyLabel(keyLabel);

				// LogD << "Creating a key with label: " << keyLabel << ". It will have code " << glyph.getCode();

				string display(model->getKeyDisplayLabel(keyLabel));

				KeyView *keyView = KeyView::createWithKeyboardView(kbView, keyLabel, display, keyLabelsSpriteBatchNode);
				if (keyView) {
					// arbitrary positions
					const KeyboardPoint &keyPoint = model->getKeyPosition(keyLabel);
					const KeyboardSize &keySize = model->getKeySize(keyLabel);

					keyView->setPosition(ccp(keyPoint.x, keyPoint.y));

					this->spriteBatchNode->addChild(keyView);

					// store in map
					keyViewMap[keyLabel] = keyView;

					CCRect bounds;
					bounds.origin = ccp(keyPoint.x, keyPoint.y);
					bounds.size = CCSize(keySize.width, keySize.height);

					keyView->setKeySize(bounds.size);
					keyBounds[keyView] = bounds;

					try {
						RGBByte color(model->getColorForKey(keyLabel));
						keyView->setHintColor(ccc3(color.r, color.g, color.b));
					} catch (std::out_of_range e) {
						LogD << "only keys with hint color defined will get them, which does not include: " << keyLabel;
					}
					
				}
			}
		}

		addGlyphsToKeys();
		return true;
	}


	// adds regular and alt-glyphs to the keys (well, actually to the BSN assoc with the KeyView)
	void KeyboardViewImpl::addGlyphsToKeys()
	{
		const shared_ptr<KeyboardModel> &model(Keyboard::getInstance().model());
		const std::vector<string> &keyLabels(model->getKeyLabels());

		const size_t playerLevel = GameState::getInstance().player().getLevel();

		for (const string &keyLabel : keyLabels) {

			KeyView *keyView(keyViewMap[keyLabel]);
			const CCRect bounds = keyBounds[keyView]; // will be used to position the keyview's glyph

			if (utilities::keyIsAModifier(keyLabel)) {
				// only add the glyph if it isn't found there yet.

				CCSprite *glyphSprite = keyView->glyphSpriteRef();
				if (!glyphSprite) {

					// special symbol with two hands palmed together, with magic balls.
					string sfn = utilities::keyGlyphCodeToSpriteFrameName(84);

					LogI << boost::format("The spriteframename: %s") % sfn;
					CCSprite *glyphSprite = CCSprite::createWithSpriteFrameName(sfn.c_str());
					glyphSprite->setColor(ccc3(255, 255, 255));

					CCPoint offset(-2, 1);
					CCPoint position(bounds.origin.x + (bounds.size.width / 2) + offset.x,
									 bounds.origin.y + (bounds.size.height / 2) + offset.y);

					keyView->addGlyphSpriteToKeyView(glyphSprite, position);
					keyView->setOpacity(1);
				}

			} else {

				const Glyph &glyph = model->getGlyphForKeyLabel(keyLabel); // no glyph but has keylabel with mod:
				LogD3 << "keylabel " << keyLabel << " gets code: " << glyph.getCode() << " with level " << glyph.getLevel();
				string sfn = utilities::keyGlyphCodeToSpriteFrameName(glyph.getCode());

				// add regular glyphs to key
				if (sfn.size() > 0) {
					// keyviews that previously would have a glyph code of -1 could get reassigned to another on a new level
					keyView->setGlyph(glyph);

					if (glyph.getCode() > 0) {
						LogI << boost::format("The spriteframename: %s") % sfn;

						CCSprite *glyphSprite = keyView->glyphSpriteRef();
						if (!glyphSprite) {
							glyphSprite = CCSprite::createWithSpriteFrameName(sfn.c_str());
							const CCPoint offset(-2, 1);
							const CCPoint position(bounds.origin.x + (bounds.size.width / 2) + offset.x,
												   bounds.origin.y + (bounds.size.height / 2) + offset.y);
							glyphSprite->setPosition(position);
							keyView->addGlyphSpriteToKeyView(glyphSprite, position); // glyphSprite now = to keyView->glyphSpriteRef
							keyView->setOpacity(1);
						}

						if (glyph.getLevel() == playerLevel) {
							glyphSprite->setColor(ccc3(255, 255, 255));
							glyphSprite->setOpacity(0); // get ready to animate
							glyphSprite->runAction(CCEaseOut::create(CCFadeIn::create(1), 4));
							keyView->setEnabled(true);
						} else if (glyph.getLevel() < playerLevel) { // enabled for this level
							glyphSprite->setColor(ccc3(255, 255, 255));
							glyphSprite->setOpacity(255);
							keyView->setEnabled(true);
						} else { // dimmed.
							glyphSprite->setColor(ccc3(0, 0, 0));
							glyphSprite->setOpacity(255 * 0.15);
							keyView->setEnabled(false);
						}

					} else { // special case for the space bar (no visible glyph symbol)
						LogI << "No glyph to be assigned for the space bar.";
					}
					
				} else {
					keyView->setEnabled(false);
				}


				// add alt glyphs to keys
				if (model->hasAltGlyphForKeyLabel(keyLabel)) {
					const Glyph &altGlyph = model->getAltGlyphForKeyLabel(keyLabel);
					LogD << "keylabel " << keyLabel << " gets alt code: " << altGlyph;
					sfn = utilities::keyGlyphCodeToSpriteFrameName(altGlyph.getCode());

					if (sfn.size() > 0) {
						keyView->setAltGlyph(altGlyph);
						if (altGlyph.getCode() > 0) {
							CCSprite *glyphSprite = keyView->altGlyphSpriteRef();
							if (!glyphSprite) {
								LogI << boost::format("The alt spriteframename: %s") % sfn;
								glyphSprite = CCSprite::createWithSpriteFrameName(sfn.c_str());

								glyphSprite->setColor(ccc3(255, 255, 255));
								const CCPoint offset(-2, 1);
								const CCPoint position(bounds.origin.x + (bounds.size.width / 2) + offset.x,
													   bounds.origin.y + (bounds.size.height / 2) + offset.y);

								glyphSprite->setPosition(position);
								keyView->addAltGlyphSpriteToKeyView(glyphSprite, position);
								keyView->setOpacity(1);
							}

							if (altGlyph.getLevel() <= playerLevel) { // enabled for this level
								glyphSprite->setColor(ccc3(255, 255, 255));
								glyphSprite->setOpacity(255);
							} else { // invisible
								glyphSprite->setOpacity(0);
							}

						}
					}
				}

			}
		}

		this->enableAltMode(false); // for now
	}


	void KeyboardView::showNewKeySymbols()
	{
		pImpl->addGlyphsToKeys();
	}



	void KeyboardView::onEnter()
	{
		LogD << "On onEnter of KeyboardView";
		// pImpl->switchColor();
		// CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);

		CCDirector::sharedDirector()->getTouchDispatcher()->addStandardDelegate(this, 0);
		CCLayer::onEnter();
	}


	void KeyboardView::onExit()
	{
		LogD << "On onExit of KeyboardView";
		CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);

		CCLayer::onExit();
	}


#pragma mark - Touch Responder

	const string &KeyboardView::keyLabelIntersectingPoint(const CCPoint &point)
	{
		static string Empty = "";
		for (auto &kv : pImpl->keyBounds) {
			if (pImpl->isPointInKeyBounds(kv.first, point)) {
				return kv.first->getLabel();
			}
		}
		return Empty;
	}


	void KeyboardViewImpl::changePressStateOfKeysToDown(set<string> keyLabels)
	{
		if (keyLabels.size() > 0) {
			LogD2 << "changing key press state to down";
			for (const string &label : keyLabels) {
				LogD4 << boost::format("label to set to down (everything else is going up): %s") % label;
			}
		}
		
		for (auto &kv : keyViewMap) {
			if (keyLabels.find(kv.first) != keyLabels.end()) {
				if (shouldPlaySFX && !kv.second->isDown()) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXKeyDown);
				}
				kv.second->keyDown();
			} else {
				if (shouldPlaySFX && kv.second->isDown()) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SFXKeyUp);
				}
				kv.second->keyUp();
			}
		}
	}


	void KeyboardView::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
	{
		pImpl->registerTouchEvents(pTouches, TouchType::TouchBegan);
	}


	void KeyboardView::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
	{
		pImpl->registerTouchEvents(pTouches, TouchType::TouchMoved);
	}


	void KeyboardView::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
	{
		pImpl->registerTouchEvents(pTouches, TouchType::TouchEnded);
	}


	void KeyboardView::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent)
	{
		pImpl->registerTouchEvents(pTouches, TouchType::TouchCancelled);
	}


	// sends input signal to KPT, bypassing KBM
	void KeyboardViewImpl::registerTouchEvents(CCSet *touches, TouchType type)
	{
		for (CCSetIterator iter = touches->begin(); iter != touches->end(); ++iter) {
			CCTouch *touch = static_cast<CCTouch *>(*iter);
			CCPoint location = CCDirector::sharedDirector()->convertToGL(touch->getLocationInView());

			bool shouldRegisterKeypress = true;
			// this is a touch move event, only notify if it has moved far enough from its
			// last position
			if (type == TouchType::TouchMoved) {
				shouldRegisterKeypress = false;
				if (touchLocations.find(touch) != touchLocations.end()) {
					// prior touch entry was found
					float distance(touchLocations[touch].getDistance(location));
					if (distance > 7) {
						LogI << boost::format("location: (%.0f, %.0f) recorded: (%.0f, %.0f)") % location.x % location.y % touchLocations[touch].x % touchLocations[touch].y;
						LogD2 << "distance moved was: " << distance;
						shouldRegisterKeypress = true;
					}
				}
				touchLocations[touch] = location;
			} else if (type == TouchType::TouchEnded) {
				touchLocations.erase(touch);
			} else if (type == TouchType::TouchBegan) {
				touchLocations[touch] = location;
			}


			if (shouldRegisterKeypress) {

				std::shared_ptr<KeyboardViewTouchInfo> pInfo(new KeyboardViewTouchInfo);
				pInfo->label = kbView->keyLabelIntersectingPoint(location);
				pInfo->touch = touch;
				pInfo->type = type;

				Notif::send("KeyboardView_KeyPress", pInfo);
			}
		}

		// changePressStateOfKeysToDown(keypressTracker().keysInDownState());
	}


#pragma mark - Alternative Mode

	void KeyboardViewImpl::enableAltMode(bool enabled)
	{
		// hide normal glyphs, show alt glyphs (do a fade)
		// change transparency for keys based on whether they're mapped
		const shared_ptr<KeyboardModel>& model = Keyboard::getInstance().model();
		const std::vector<string> &keyLabels(model->getKeyLabels());

		for (const string &keyLabel : keyLabels) {
			KeyView *keyView(keyViewMap[keyLabel]);
			keyView->enableAltMode(enabled);
		}
	}


#pragma mark - Getters

	KeypressTracker &KeyboardView::keypressTracker()
	{
		return GameState::getInstance().keypressTracker();
	}


	KeyView *KeyboardView::getKeyFromLabel(const std::string &label) const
	{
		return pImpl->keyViewMap[label];
	}

	
#pragma mark - Events

	void KeyboardView::notifCallback(const string &code, std::shared_ptr<void> data)
	{
		if ("KeyboardModel_NewLevel" == code) {
			showNewKeySymbols();
		}

		else if ("KeypressTracker_RequiresUIRefresh" == code) {
			pImpl->changePressStateOfKeysToDown(keypressTracker().keysInDownState());
		}

		else if ("CopyText_Mistake" == code) {
			// forget everything pressed after a mistake is registered
			keypressTracker().reset();
		}


		else if ("KeyboardModel_AltModeToggled" == code) {
			std::shared_ptr<bool> isAltModeOn = std::static_pointer_cast<bool>(data);
			pImpl->enableAltMode(*isAltModeOn);
		}
	}


#pragma mark - Miscellaneous

	void KeyboardViewImpl::switchColor() {
		utilities::RGBByte rByte = utilities::randomRGBByte();
		this->layerColor->setColor(ccc3((GLubyte)rByte.r, (GLubyte)rByte.g, (GLubyte)rByte.b));
	}
}