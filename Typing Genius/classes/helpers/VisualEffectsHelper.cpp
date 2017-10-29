//
//  VisualEffectsHelper.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/28/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "VisualEffectsHelper.h"
#include "MCBCallLambda.h"
#include <cmath>

namespace ac {
	
	using namespace MCBPlatformSupport;
	
	void VisualEffectsHelper::runDrainEffectOnTimer(CCProgressTimer *progressTimer, float duration = 1.0f)
	{
		float pct = progressTimer->getPercentage();
		auto oink = MCBCallLambda::create([=] () {
			LogI << "OINK";
		});
		progressTimer->runAction(CCSequence::create(CCProgressFromTo::create(duration, pct, 0),
										oink, NULL));
	}
	

	CCFiniteTimeAction *VisualEffectsHelper::trickleEffectOnNumericLabel(vector<string> values, float delay, bool enableGrow)
	{
		CCArray *actions = CCArray::create();

		// form the actions
		for (const string &val : values) {

			// you can delete at the end of the callback (setLabelUnitStringCallback)
			string *pString = new string(val);

			// this is something that I hope can be replaced with a lambda-style callback method
			CCCallFuncND *func = CCCallFuncND::create(this,
				callfuncND_selector(VisualEffectsHelper::setLabelUnitStringCallback), pString);

			actions->addObject(func);

			if (delay > 0) {
				actions->addObject(CCDelayTime::create(delay));
			}
		}
		
		// the last thing added shouldn't be a delay (these should only be used in between callfuncs)
		if (actions->count() > 1) {
			// because we add them by pairs
			if (delay > 0) {
				actions->removeLastObject();
			}

			if (enableGrow) {
				CCFiniteTimeAction *enlarge = CCEaseOut::create(CCScaleTo::create(0.02, 1.1), 1);
				CCFiniteTimeAction *revertScale = CCEaseOut::create(CCScaleTo::create(0.02, 1), 1);
				actions->insertObject(enlarge, 0);
				actions->addObject(revertScale);
			}
		}
		
		CCSequence *actionSequence = CCSequence::create(actions);
		return CCEaseOut::create(actionSequence, 1);
	}

	
	/** 
	 *	@brief Do the setting of the string here.
	 */
	void VisualEffectsHelper::setLabelUnitStringCallback(CCNode *node, void *data)
	{
		CCLabelBMFont *label = static_cast<CCLabelBMFont *>(node);
		string *s = static_cast<string *>(data);
		label->setString(s->c_str());
		CC_SAFE_DELETE(s);
	}


//	vector<string> VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(int min, int max, size_t iterations,
//																		  const string &formatString)
//	{
//		vector<string> ret;
//		if (min > max) {
//			LogD << "Min larger than Max in stepwiseNumericValuesBasedOnRange. What are you thinking?";
//			return ret;
//		}
//
//		if (iterations < 1) {
//			LogD << "Iterations should be greater than 0. What are you thinking?";
//			return ret;
//		}
//
//		stringstream ss;
//
//		boost::format fmt(formatString);
//		bool needsFormat = fmt.size() > 0;
//
//		int delta = max - min; // could be zero.
//		if (delta < 1) {
//			ss << max;
//			ret.push_back(ss.str());
//		} else {
//			iterations = MIN(delta, iterations);
//			for (size_t i = 0; i < 1 + iterations; i++) {
//				ss << (int)(min + (delta / iterations) * i);
//
//				if (needsFormat) {
//					fmt % ss.str();
//					ret.push_back(fmt.str());
//				} else {
//					ret.push_back(ss.str());
//				}
//
//				ss.clear(); ss.str(""); // clear it
//			}
//		}
//		return ret;
//	}
	
	vector<string> VisualEffectsHelper::stepwiseNumericValuesBasedOnRange(float min, float max,
																		  size_t iterations,
																		  const string &formatString)
	{
		vector<string> ret;
		if (min > max) {
			LogD << "Min larger than Max in stepwiseNumericValuesBasedOnRange. What are you thinking?";
			return ret;
		}
		
		if (iterations < 1) {
			LogD << "Iterations should be greater than 0. What are you thinking?";
			return ret;
		}
		
		stringstream ss;
		

		// I'll just assume there's only one format specifier in the whole string.
		bool needsFormat = formatString.size() > 0;

		float delta = max - min; // could be zero.
		if (delta == 0) {
			ss << max;
			ret.push_back(ss.str());
		} else {
			// iterations = MIN(delta, iterations);
			for (size_t i = 0; i < 1 + iterations; i++) {
				float val = (float)(min + (delta / iterations) * (float)i);
				if (needsFormat) {
					boost::format fmt(formatString);

					if (formatString.find("%d") != formatString.npos) { // forced to an int
						int valInt = val;
						const string toPush((fmt % valInt).str());
						string back;
						if (ret.size()>0) {
							back = ret.back();
						}

						// push only if not alike with the last.
						if (toPush != back) {
							ret.push_back(toPush);
						}
					} else {
						ret.push_back((fmt % val).str());
					}

				} else {
					ss << (float)(min + (delta / iterations) * i);
					ret.push_back(ss.str());
				}
				
				ss.clear(); ss.str(""); // clear it
			}
		}
		return ret;
	}



	void VisualEffectsHelper::runGalleryHeadline(CCLabelBMFont *label, const string &labelContent,
												 float duration, float showTimePct)
	{
		showTimePct = MAX(0, MIN(1, showTimePct)); // lock to 0..1 range

		const float pauseRatio = showTimePct; // the rest of the duration split up between appear / disappear
		const float showTime = pauseRatio * duration;
		const float totalMoveTime = duration - showTime; // 8:2 in favor of disappear

		CCSequence *subHLSequence = CCSequence::create(
			CCFadeIn::create(totalMoveTime * 0.2),
			CCDelayTime::create(showTime),
			CCFadeOut::create(totalMoveTime * 0.8), NULL
		);

		label->setString(labelContent.c_str());
		label->setOpacity(0); // prepare for fade in
		label->runAction(subHLSequence);
	}

}



















