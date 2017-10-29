//
//  MVC.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/11/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Base classes for MVC architecture. Make sure to understand the code and read the comments.

#include "MVC.h"

#include "BlockCanvasModel.h"
#include "BlockCanvasView.h"

#include "StatsHUDModel.h"
#include "StatsHUDView.h"

#include "KeyboardModel.h"
#include "KeyboardView.h"


namespace ac {

	template<class M, class V, class C>
	V *Controller<M, V, C>::view()
	{
		if (!_view) {
			_view = V::create();
			_view->retain();
		}
		return _view;
	}

	// note: neither are const because subclasses might be asked to lazily recreate them when null
	template<class M, class V, class C>
	const shared_ptr<M>& Controller<M, V, C>::model()
	{
		if (!_model) {
			_model.reset(new M);
		}
		return _model;
	}


#pragma mark - Template Instantiation

	template BlockCanvasView *Controller<BlockCanvasModel, BlockCanvasView, void>::view();
	template const shared_ptr<BlockCanvasModel>& Controller<BlockCanvasModel, BlockCanvasView, void>::model();

	class KeyboardConfiguration;
	template KeyboardView *Controller<KeyboardModel, KeyboardView, KeyboardConfiguration>::view();
	template const shared_ptr<KeyboardModel>& Controller<KeyboardModel, KeyboardView, KeyboardConfiguration>::model();

	template StatsHUDView *Controller<StatsHUDModel, StatsHUDView, void>::view();
	template const shared_ptr<StatsHUDModel>& Controller<StatsHUDModel, StatsHUDView, void>::model();
}