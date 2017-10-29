//
//	KeyView
//	Typing Genius
//	Created by Aldrich Co on 8/16/13.
//
//
//	@brief represents an physical / visible key; doesn't contain the glyphs (they have to be in the BSN)
//	tightly coupled with KeyboardView (its parent)

#pragma once

#include <iostream>
#include "cocos2d.h"
#include "ACTypes.h"

namespace ac
{
	USING_NS_CC;
	using std::string;

	// fwd declares
	class KeyViewImpl;
	class KeyboardView;
	class Glyph;
	
	const ccColor4B KeyColorLayerColor = ccc4(255, 255, 255, 255);
	const KeySize DefaultKeySize = { 5, 8 };

	class KeyView : public CCSprite
	{
	public:

		KeyView(KeyboardView *keyboardView, string code, string display, CCSpriteBatchNode *keyLabelsSpriteBatchNode,
				bool isModifier = false);

		virtual ~KeyView();

		// creates a static autoreleased create function for this class.
		static KeyView* createWithKeyboardView(KeyboardView *keyboardView,
											   string label, string display,
											   CCSpriteBatchNode *keyLabelsSpriteBatchNode);

		// simulates a keypress event
		virtual void keyDown();
		virtual void keyUp();
		
		void setHintColor(const ccColor3B &);
		const ccColor3B &getHintColor() const;
				
		virtual void onEnter();
		virtual void onExit();

		bool isDown() const;

		// enabledness only affects the appearance of the keyview
		bool isEnabled() const;
		void setEnabled(bool);

		bool isModifier() const;

		bool isInAltMode() const;
		void enableAltMode(bool altMode);

		const string& getLabel() const;

		void setOpacity(float opacity);

		void setGlyph(const Glyph &);
		const Glyph &getGlyph() const;

		void setAltGlyph(const Glyph &);
		const Glyph &getAltGlyph() const;

		CCSprite *glyphSpriteRef() const;
		CCSprite *altGlyphSpriteRef() const;

		void setKeySize(const CCSize &size);
		const CCSize &getKeySize() const;

		// not really adding to the keyview as child per se, but to the batch sprite node.
		void addGlyphSpriteToKeyView(CCSprite *glyphSprite, const CCPoint &position);
		void addAltGlyphSpriteToKeyView(CCSprite *glyphSprite, const CCPoint &position);

	private:
		std::unique_ptr<KeyViewImpl> pImpl;

		bool init(); // used by create()
	};

}
