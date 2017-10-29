//
//  IntroScene.h
//  Typing Genius
//
//  Created by Aldrich Co on 1/8/14.
//  Copyright (c) 2014 Aldrich Co. All rights reserved.
//

#ifndef __Typing_Genius__IntroScene__
#define __Typing_Genius__IntroScene__

#include "Notif.h"
#include "cocos2d.h"

namespace ac {

	USING_NS_CC;
	struct IntroLayerImpl;

	class IntroLayer : public CCLayer, public NotifListener
	{
	public:
		virtual bool init();

		virtual void onEnter();
		virtual void onExit();

		IntroLayer();
		~IntroLayer();

		static CCScene *scene();
		CCScene *sceneWithThisLayer();

		// preprocessor macro for "static create()" constructor
		CREATE_FUNC(IntroLayer);

		// NotifListener callback
		void notifCallback(const string &code, std::shared_ptr<void> data);


	private:
		std::unique_ptr<IntroLayerImpl> pImpl;

	};

}

#endif /* defined(__Typing_Genius__IntroScene__) */
