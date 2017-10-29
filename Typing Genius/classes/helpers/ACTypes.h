/*
 * ACTypes.h
 * Typing Genius
 *
 * Created by Aldrich Co on 9/6/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 * @brief contains useful types; is free of cocos2d-x dependencies
 *
 */


#pragma once

#include "cocos2d.h"

namespace ac
{
	USING_NS_CC; // for window size mainly.
	
	struct NeutralSize { float width; float height; };
	struct NeutralPoint { float x; float y; };
	
	inline std::ostream& operator<<(std::ostream& out, const NeutralSize& sz)
	{
		out << "{ w: " << sz.width << ", h: " << sz.height << " }";	
		return out;
	}
		
	inline std::ostream& operator<<(std::ostream& out, const NeutralPoint& pt)
	{
		out << "{ x: " << pt.x << ", y: " << pt.y << " }";
		return out;
	}
	
	struct CursorCoords { unsigned int row; unsigned int column; };
	
	inline bool sizesEqual(NeutralSize a, NeutralSize b) { return a.width == b.width && a.height == b.height; }
	inline bool pointsEqual(NeutralPoint a, NeutralPoint b) { return a.x == b.x && a.y == b.y; }
	
	inline CCSize getWindowSize() { return CCDirector::sharedDirector()->getWinSize(); }
	
	// note: there may be others (e.g., mid-press)
	enum class KeyPressState { Up /* not pressed */, Down	/* held down */, Other };
	
	enum class CarriageMoveDirection { Up, Down, Left, Right };
	
	typedef NeutralSize KeySize;

	typedef NeutralSize KeyboardSize;

	typedef NeutralPoint KeyboardPoint;
	
	typedef NeutralSize CarriageSize;
	
	typedef NeutralPoint CarriagePoint;
	
	// size of a typed character (this defines the carriage offsets and cursor points as well)
	typedef NeutralSize CharSize;
	
	// for keyboard
	enum class TouchType
	{
		TouchBegan,
		TouchEnded,
		TouchMoved,
		TouchCancelled
	};
	
	struct KeyEvent
	{
		std::string key;
		TouchType type;
	};
	

	inline float rel2WindowWidth(const float relXPct)
	{
		return getWindowSize().width * relXPct / 100.0f;
	}

	inline float rel2WindowHeight(const float relYPct)
	{
		return getWindowSize().height * relYPct / 100.0f;
	}
	
	inline CCSize rel2WindowSize(const NeutralSize &sz)
	{
		CCSize wSize(getWindowSize());
		return { wSize.width * sz.width / 100.0f, wSize.height * sz.height / 100.0f };
	}
	
	inline CCPoint rel2WindowPoint(const NeutralPoint &pt)
	{
		CCSize wSize(getWindowSize());
		return { wSize.width * pt.x / 100.0f, wSize.height * pt.y / 100.0f };
	}

	inline float rel2ParentWidth(const float relXPct, CCNode *parent)
	{
		return parent->getContentSize().width * relXPct / 100.0f;
	}

	inline float rel2ParentHeight(const float relYPct, CCNode *parent)
	{
		return parent->getContentSize().height * relYPct / 100.0f;
	}
	
	inline CCSize rel2ParentSize(const NeutralSize &sz, CCNode *parent)
	{
		CCSize parSize(parent->getContentSize());
		return { parSize.width * sz.width / 100.0f, parSize.height * sz.height / 100.0f };
	}
	
	inline CCPoint rel2ParentPoint(const NeutralPoint &pt, CCNode *parent)
	{
		CCSize parSize(parent->getContentSize());
		return { parSize.width * pt.x / 100.0f, parSize.height * pt.y / 100.0f };
	}
}
