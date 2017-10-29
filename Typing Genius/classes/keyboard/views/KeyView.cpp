//
// Created by Aldrich Co on 8/16/13.
//

#include "KeyView.h"
#include "Keyboard.h"
#include "ScreenResolutionHelper.h"
#include "DebugSettingsHelper.h"
#include "Utilities.h"
#include "KeyboardModel.h"
#include "KeyModel.h"
#include "KeyboardView.h"
#include "Glyph.h"
#include "GlyphMap.h"
#include "GameState.h"
#include <sstream>


namespace ac
{
	using std::string;
	using std::stringstream;
	
	const char *KeyCostFont = "fontatlases/key-cost-fonts.fnt";
	
	const char *KeyUpSpriteFrameName = "key-up.png";
	const char *KeyDownSpriteFrameName = "key-down.png";
	
	const char *ModKeyUpSpriteFrameName = "mod-key-up.png";
	const char *ModKeyDownSpriteFrameName = "mod-key-down.png";

	const char *KeyUpLongSpriteFrameName = "key-up-long.png";
	const char *KeyDownLongSpriteFrameName = "key-down-long.png";

	const char *KeyAltSpriteFrameName = "alt-key.png";

	const char *KeySpriteFrameName = "kb-blank.png";

	struct KeyViewImpl
	{
		KeyViewImpl(KeyboardView *keyboardView, KeyView *keyView, string label, string display,
					CCSpriteBatchNode *keyLabelsSpriteBatchNode, bool isModifier) :

		layerColor(), keyboardView(keyboardView), keyView(keyView), label(label), display(display),
		keySpriteUp(), keySpriteDown(), isEnabled(true), isDown(false), glyph(-1),
		altGlyph(-1), hintColor(), isModifier(isModifier), isInAltMode(false),
		keyLabelsSpriteBatchNodeRef(keyLabelsSpriteBatchNode), glyphSpriteRef(),
		altGlyphSpriteRef(), altColor(), keySpriteAltMode(), keySpriteAltModeCost(), keySize(CCSizeZero)
		{ }

		/** colored layer for display purposes only */
		CCLayerColor *layerColor;

		/** parent view */
		KeyboardView *keyboardView; // you'll need to add children to this view instead of the CCSpriteBatchNode for some sprites like "key cost"
		KeyView *keyView;

		string label;

		bool isDown;
		bool isEnabled;
		bool isInAltMode;
		bool isModifier;
		
		/** what is shown on the keyview */
		string display;

		// comes from the parent view (KeyboardView). Don't create one yourself
		CCSpriteBatchNode *keyLabelsSpriteBatchNodeRef;
		CCSprite *glyphSpriteRef;
		CCSprite *altGlyphSpriteRef;
		
		CCSprite *keySpriteUp;
		CCSprite *keySpriteDown;

		CCSprite *keySpriteAltMode; // lazily created for performance
		CCLabelBMFont *keySpriteAltModeCost; // has to be tilted and place in the lower right hand corner of keySpriteAltMode

		ccColor3B hintColor;
		ccColor3B brightenedHintColor;
		ccColor3B altColor;

		Glyph glyph;
		Glyph altGlyph;

		void toggleGlyphSpriteVisibility(bool isInAltMode);
		void toggleHintColorization(bool isInAltMode);

		CCSize keySize;
	};


	// constructor
	KeyView::KeyView(KeyboardView *keyboardView, string label, string display, CCSpriteBatchNode *keyLabelsSpriteBatchNode,
					 bool isModifier)
	{
		pImpl.reset(new KeyViewImpl(keyboardView, this, label, display, keyLabelsSpriteBatchNode, isModifier));
		// LogD << "Entered KeyView constructor! " << this;
	}


	// destructor
	KeyView::~KeyView()
	{
		// LogD << "Entered KeyView destructor! " << this;
	}


	KeyView* KeyView::createWithKeyboardView(KeyboardView *keyboardView, string label, string display,
											 CCSpriteBatchNode *keyLabelsSpriteBatchNode)
	{
		KeyView *pRet = new KeyView(keyboardView, label, display, keyLabelsSpriteBatchNode,
									utilities::keyIsAModifier(label));

		if (pRet && pRet->init()) {
			pRet->autorelease();
			return pRet;
		} else {
			delete pRet;
			pRet = NULL;
			return NULL;
		}
	}


	bool KeyView::init()
	{
		bool ret = false;

		do {
			// Default KeyModel Size, if not supplied directly by KeyView for the key.

			CC_BREAK_IF(!CCSprite::initWithSpriteFrameName(KeySpriteFrameName));

			const shared_ptr<KeyboardModel> &model = Keyboard::getInstance().model();
			KeySize keySize = model->getKeySize(pImpl->label);
			
			// you have to use actual sizes here. Shift to CCSize
			CCSize sz(keySize.width, keySize.height);

			const KeySpriteType &spriteType = model->getKeySpriteType(pImpl->label);
			bool isLong = spriteType == KeySpriteType::Elongated;
			bool isMod = spriteType == KeySpriteType::Shift;

			const char *keyUpFrame;
			const char *keyDownFrame;

			if (isMod) {
				keyUpFrame = ModKeyUpSpriteFrameName;
				keyDownFrame = ModKeyDownSpriteFrameName;
			} else if (isLong) {
				keyUpFrame = KeyUpLongSpriteFrameName;
				keyDownFrame = KeyDownLongSpriteFrameName;
			} else {
				keyUpFrame = KeyUpSpriteFrameName; // normal
				keyDownFrame = KeyDownSpriteFrameName;
			}


			// subsprites
			pImpl->keySpriteUp = CCSprite::createWithSpriteFrameName(keyUpFrame);
			CC_BREAK_IF(!pImpl->keySpriteUp);
			
			pImpl->keySpriteDown = CCSprite::createWithSpriteFrameName(keyDownFrame);
			CC_BREAK_IF(!pImpl->keySpriteDown);
			
			pImpl->keySpriteUp->setAnchorPoint(CCPointZero);
			pImpl->keySpriteUp->setPosition(CCPointZero);

			pImpl->keySpriteDown->setAnchorPoint(CCPointZero);
			pImpl->keySpriteDown->setPosition(CCPointZero);

			this->addChild(pImpl->keySpriteDown);
			this->addChild(pImpl->keySpriteUp);

			// keyUp(); // needed?
			ret = true;
			
		} while (0);
		return ret;
	}


	/** This is the right place to add touch responder requests, not in init(). */
	void KeyView::onEnter()
	{
		CCSprite::onEnter();
	}


	void KeyView::onExit()
	{
		CCSprite::onExit();
	}
	
	
	void KeyView::setHintColor(const ccColor3B &color)
	{
		namespace clr = utilities;

		pImpl->keySpriteUp->setColor(color);
		pImpl->keySpriteDown->setColor(color);

		pImpl->hintColor = color;

		clr::HSVFloat hsv = clr::rgb2Hsv(clr::rgbByteToFloat(clr::rgbByteFromCcc3(color)));

		// for the alt color
		clr::HSVFloat hsv2 = hsv;
		hsv2.s = 0;
		pImpl->altColor = clr::ccc3FromRGB(clr::rgbFloatToByte(clr::hsv2Rgb(hsv2)));

		// brighter for non shift buttons
		const float colorValueIncrease = utilities::keyIsAModifier(pImpl->label) ? 0.2 : 0.5;
		hsv.v = MIN(1, hsv.v + colorValueIncrease);
		
		pImpl->brightenedHintColor = clr::ccc3FromRGB(clr::rgbFloatToByte(clr::hsv2Rgb(hsv)));
	}
	
	
	const ccColor3B &KeyView::getHintColor() const
	{
		return pImpl->isInAltMode ? pImpl->altColor : pImpl->hintColor;

		// return pImpl->keySpriteUp->getColor();
	}


	const string& KeyView::getLabel() const
	{
		return pImpl->label;
	}


	void KeyView::setOpacity(float opacity)
	{
		pImpl->keySpriteDown->setOpacity(255 * opacity);
		pImpl->keySpriteUp->setOpacity(255 * opacity);
	}


	void KeyView::keyDown()
	{
		if (pImpl->isDown) return;

		pImpl->keySpriteDown->setOpacity(255);
		pImpl->keySpriteUp->setOpacity(0);
		pImpl->isDown = true;

		pImpl->keySpriteDown->stopAllActions();
		pImpl->keySpriteUp->stopAllActions();

		const ccColor3B &targetColor = pImpl->brightenedHintColor;

		if (!pImpl->isInAltMode) {
			if (!pImpl->isModifier) {

				const float duration = 0.3;
				CCTintTo *tintAction1 = CCTintTo::create(duration, targetColor.r, targetColor.g, targetColor.b);
				CCTintTo *tintAction2 = CCTintTo::create(duration, targetColor.r, targetColor.g, targetColor.b);

				pImpl->keySpriteDown->setColor(pImpl->hintColor);
				pImpl->keySpriteUp->setColor(pImpl->hintColor);

				pImpl->keySpriteDown->runAction(CCEaseIn::create(tintAction1, 5));
				pImpl->keySpriteUp->runAction(CCEaseIn::create(tintAction2, 5));

			} else {
				pImpl->keySpriteDown->setColor(targetColor);
				pImpl->keySpriteUp->setColor(targetColor);
			}
		}
	}
	
	
	void KeyView::keyUp()
	{
		if (!pImpl->isDown) return;

		pImpl->keySpriteDown->setOpacity(0);
		pImpl->keySpriteUp->setOpacity(255);
		pImpl->isDown = false;


		const ccColor3B &origColor = this->getHintColor(); // pImpl->hintColor;

		pImpl->keySpriteDown->stopAllActions();
		pImpl->keySpriteUp->stopAllActions();

		if (!pImpl->isModifier) {

			// Up sprite shown
			const float duration = 4;

			CCTintTo *revertAction1 = CCTintTo::create(duration, origColor.r, origColor.g, origColor.b);
			CCTintTo *revertAction2 = CCTintTo::create(duration, origColor.r, origColor.g, origColor.b);

			pImpl->keySpriteDown->setColor(pImpl->brightenedHintColor);
			pImpl->keySpriteUp->setColor(pImpl->brightenedHintColor);

			pImpl->keySpriteDown->runAction(CCEaseOut::create(revertAction1, 1));
			pImpl->keySpriteUp->runAction(CCEaseOut::create(revertAction2, 1));

		} else {

			pImpl->keySpriteDown->setColor(origColor);
			pImpl->keySpriteUp->setColor(origColor);
		}


	}


	// not really adding to the keyview as child per se, but to the batch sprite node.
	void KeyView::addGlyphSpriteToKeyView(CCSprite *glyphSprite, const CCPoint &position)
	{
		if (!pImpl->glyphSpriteRef) {
			pImpl->keyLabelsSpriteBatchNodeRef->addChild(glyphSprite);
			glyphSprite->setPosition(position);
			pImpl->glyphSpriteRef = glyphSprite;
		}
	}


	// not really adding to the keyview as child per se, but to the batch sprite node.
	void KeyView::addAltGlyphSpriteToKeyView(CCSprite *glyphSprite, const CCPoint &position)
	{
		if (!pImpl->altGlyphSpriteRef) {
			pImpl->keyLabelsSpriteBatchNodeRef->addChild(glyphSprite);
			glyphSprite->setPosition(position);
			pImpl->altGlyphSpriteRef = glyphSprite;
		}
	}


	void KeyView::setKeySize(const CCSize &size)
	{
		pImpl->keySize = size;
		utilities::scaleNodeToSize(pImpl->keySpriteDown, size);
		utilities::scaleNodeToSize(pImpl->keySpriteUp, size);
	}


	const CCSize &KeyView::getKeySize() const
	{
		return pImpl->keySize;
	}


	bool KeyView::isDown() const
	{
		return pImpl->isDown;
	}


	bool KeyView::isEnabled() const
	{
		return pImpl->isEnabled;
	}
	
	
	void KeyView::setEnabled(bool enabled)
	{
		pImpl->isEnabled = enabled;
	}


	bool KeyView::isModifier() const
	{
		return pImpl->isModifier;
	}


	bool KeyView::isInAltMode() const
	{
		return pImpl->isInAltMode;
	}


	void KeyView::enableAltMode(bool altMode)
	{
		pImpl->isInAltMode = altMode;
		pImpl->toggleGlyphSpriteVisibility(altMode);
		pImpl->toggleHintColorization(altMode);
	}


	void KeyView::setGlyph(const Glyph &glyph)
	{
		pImpl->glyph = glyph;
	}


	const Glyph &KeyView::getGlyph() const
	{
		return pImpl->glyph;
	}


	void KeyView::setAltGlyph(const Glyph &glyph)
	{
		pImpl->altGlyph = glyph;
	}


	const Glyph &KeyView::getAltGlyph() const
	{
		return pImpl->altGlyph;
	}


	CCSprite *KeyView::glyphSpriteRef() const
	{
		return pImpl->glyphSpriteRef;
	}


	CCSprite *KeyView::altGlyphSpriteRef() const
	{
		return pImpl->altGlyphSpriteRef;
	}


#pragma mark - Miscellaneous

	void KeyViewImpl::toggleGlyphSpriteVisibility(bool isInAltMode)
	{
		if (!this->isModifier) { // have special mode for modifier
			if (glyphSpriteRef) {
				glyphSpriteRef->setVisible(!isInAltMode);
			}

			if (altGlyphSpriteRef) {
				altGlyphSpriteRef->setVisible(isInAltMode);
			}
		}
	}

	void KeyViewImpl::toggleHintColorization(bool isInAltMode)
	{
		// in alt mode:
		// if not assigned, set color of key sprites to white / make semi-transparent
		this->keySpriteUp->stopAllActions();
		this->keySpriteDown->stopAllActions();

		if (isInAltMode) {
			// split into two camps: those who has the glyphs, and those who haven't
			const bool hasActivePower = altGlyphSpriteRef && (altGlyphSpriteRef->getOpacity() > 0);

			if (hasActivePower) {
				if (!this->keySpriteAltMode) { // create one if not done so yet
					this->keySpriteAltMode = CCSprite::createWithSpriteFrameName(KeyAltSpriteFrameName);
					this->keyView->addChild(this->keySpriteAltMode);
					this->keySpriteAltMode->setAnchorPoint(CCPointZero);
					this->keySpriteAltMode->setPosition(CCPointZero);
				}

				// also add the key cost
				// KeyCostFont
				if (!this->keySpriteAltModeCost) {
					const SpecialAbility &specialAbility = GameState::getInstance().glyphMap().
						specialAbilityForKey(this->label);
					
					stringstream ss;
					ss << specialAbility.cost;
					
					this->keySpriteAltModeCost = CCLabelBMFont::create(ss.str().c_str(), KeyCostFont);
					this->keySpriteAltModeCost->setColor(ccc3(0, 0, 0));
					this->keyboardView->addChild(this->keySpriteAltModeCost);
					
					this->keySpriteAltModeCost->setAnchorPoint(ccp(1, 0)); // lower right

					// position it
					CCPoint position(this->keyView->getPosition()); // anchored at lower left corner
					
					// hand-tweaked values
					CCPoint offset = utilities::isTabletFormFactor() ? ccp(-7, 0) : ccp(-4, 0);
					
					// note: this label is right-aligned
					position.x += keyView->getKeySize().width + offset.x; //  - offset.x - halfLabelWidth;
					position.y += offset.y; // y=0 as the label is taller than it looks (allowances for font descender)
					this->keySpriteAltModeCost->setPosition(position);
				}

				this->keySpriteAltModeCost->setVisible(true);
				this->keySpriteAltMode->setVisible(true);
				this->keySpriteUp->setVisible(false);
				this->keySpriteDown->setVisible(false);

			} else {
				if (this->keySpriteAltMode) {
					this->keySpriteAltMode->setVisible(false);
				}
				if (this->keySpriteAltModeCost) {
					this->keySpriteAltModeCost->setVisible(false);
				}

				this->keySpriteUp->setVisible(true);
				this->keySpriteDown->setVisible(true);

 				this->keySpriteUp->setOpacity(255 * (hasActivePower ? 1 : 0.25));
				this->keySpriteDown->setOpacity(255 * (hasActivePower ? 1: 0.25));

				this->keySpriteUp->setColor(hasActivePower ? this->hintColor : this->altColor);
				this->keySpriteDown->setColor(hasActivePower ? this->hintColor : this->altColor);
			}

		} else {
			// normal mode
			if (this->keySpriteAltMode) {
				this->keySpriteAltMode->setVisible(false);
			}
			if (this->keySpriteAltModeCost) {
				this->keySpriteAltModeCost->setVisible(false);
			}

			this->keySpriteUp->setVisible(true);
			this->keySpriteDown->setVisible(true);

			this->keySpriteUp->setOpacity(255);
			this->keySpriteDown->setOpacity(255);
			this->keySpriteUp->setColor(this->hintColor);
			this->keySpriteDown->setColor(this->hintColor);
		}


	}
}