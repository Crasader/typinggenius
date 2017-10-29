//
//  MVC.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/11/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//
//	Base classes for MVC architecture. Make sure to understand the code and read the comments.

#pragma once

#include "ACTypes.h"
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include "Notif.h" // -- M and V classes already are subclasses of NotifListener

namespace ac {
	
	using std::shared_ptr;
	using std::string;
	using boost::bind;
	
	using boost::signals2::connection;
	typedef connection sign_conn_t;
	
	class Model;
	class View;
	class Configuration;


	class SignalsSubscriber // to be subclassed by Model and View
	{
		// conn = signal.connect(bind(...))
		//		should now be
		// registerSignal(signal.connect(bind(...)))

	public:
		void registerSignal(sign_conn_t &conn) {
			this->conns.push_back(conn);
		}

		void deregisterSignals() {
			// "foreach conn disconnect" may not be needed based on RAII rules!
			for (const auto &conn : conns) {
				conn.disconnect();
			}

			conns.clear();
		}

	private:
		// collection of conns; you only keep these around so you can unsubscribe to their signals
		std::vector<sign_conn_t> conns;

	};


	// abstract, might need to be a template! otherwise how will models and views get exposed properly?
	template<class M, class V, class C /* C for configuration! */>
	class Controller : public SignalsSubscriber
	{
	public:
		// static singleton getters should be defined in subclasses, like this:
		// static Keyboard& getInstance() {
		//     static Keyboard instance(new KeyboardModel, new KeyboardView);
		//     return instance;
		// }
		
		// virtual destructor
		virtual ~Controller() {};
		
		// note: neither are const because subclasses might be asked to lazily recreate them when null
		virtual V *view();
		virtual const shared_ptr<M> &model();
		
		virtual void setModel(const shared_ptr<M> & model) { _model = model; }
		virtual void setView(V *view) { _view = view; } // should you retain this?
		
		// when (re)launched each method is called again; note that the Controller itself as a singleton remains alive
		// throughout. Not sure if these are still needed.
		virtual void setUp() = 0;
		virtual void tearDown() = 0;
				
	private:
		
		shared_ptr<M> _model;
		V* _view; // let Cocos2d memory management take over
		
		// noncopyable
		Controller(const Controller &);
		Controller &operator=(const Controller &);
		
	protected:
		// subclass is expected to obtain a suitable configuration to load.
		Controller(shared_ptr<M> model, V* view) : _model(model), _view(view) {
			// subclassers should call loadConfig(defaultConfig) in their constructors
		}
		
		Controller() : Controller(nullptr, nullptr) {}
		
		// upon construction of Controller this will be called, overriders will have access to model() and view().
		// Controller is expected to use a Configuration to prepopulate values. We won't keep a Configuration
		// object alive past the initialization stage; the info it supplies should already have been transferred to
		// the model (or view).
		// later we need to allow dynamic reloading of configuration file
		// IMPORTANT: can be called several times without repercussions!
//		virtual void loadConfig(shared_ptr<C> config) = 0;
	};
	
	

	
	
	class Model : public NotifListener
	{
	public:
		virtual ~Model() {}
	protected:
		Model() {}
	};
	
	
	// a view should inherit this alongside a CCNode class.
	class View : public NotifListener
	{
	public:
		virtual ~View() {}
		
	protected:
		View() {}
	};
	
	
	
	
	// it's a helper class of Controller that may know intimate details about its Views and Models
	class Configuration
	{
	public:
		
		// each subclass is expected to provide factory method like this:
		// static shared_ptr<Configuration> createConfiguration(const string &config);
		// -- this chooses the correct subclass to use.
		
		// subclasses should implement this but specify the precise controller to be passed(?)
		// virtual void initialize(Controller &controller) = 0;
		
		// each config may have an associated file name that will be read and loaded
		virtual const string configFileName() const = 0;
		
		Configuration() {}
		virtual ~Configuration() {}
		
	private:
		Configuration(const Configuration &);
		Configuration &operator=(const Configuration &);
	};
}
