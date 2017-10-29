//
//  KeypressTracker.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 10/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "KeypressTracker.h"
#include "KeyView.h"
#include "GameState.h"
#include "CopyText.h"
#include "Utilities.h"

namespace ac {
	
	using std::pair;
	using std::shared_ptr;
	using std::static_pointer_cast;

	// if at least one modifier key is held this returns true.
	static bool hasModifierKeys(const set<string> &keys);

	typedef deque<KeyEvent>::const_iterator deqIt;
	
	inline bool operator== (const KeyEvent &kev1, const KeyEvent &kev2)
	{
		return (kev1.key == kev2.key && kev1.type == kev2.type);
	}

	
	// will start from the end, going backward, and return keyBuffer.begin if not found.
	bool locateKeyEvent(const KeyEvent &kev, const deque<KeyEvent> &keyBuffer);

	// first member of pair are for those entering touch, second are for those leaving touch
	// pair<set<string>, set<string>> keysAboutToEnterAndLeaveTouch(set<string> keysHeldDown, const deque<KeyEvent> &keyBuffer);
	set<string> keysEnteringTouch(set<string> keysHeldDown, const deque<KeyEvent> &keyBuffer);
	
	set<string> keysDown(const map<CCTouch *, string>& tracker);
	
	
	KeypressTracker::KeypressTracker()
	{
		reset();
	}


	/** 
	 @param touch one touch object
	 @param label keyLabel
	 @param type one of four possible values
	 */
	void KeypressTracker::trackTouchEvent(CCTouch *touch, const string &label, TouchType type, bool isMapped = true)
	{
		// you have to decide how to handle non assigned values
		// return value should take its cue from addKeypressToBuffer.
		static string Empty = "";
		set<string> oldKeys;
		set<string> newKeys;

		switch (type) {
			case TouchType::TouchBegan:
				tracker[touch] = label;

				if (isMapped) {
					newKeys.insert(label);
					lastKeyDown = label;
				}
				break;

			case TouchType::TouchEnded: 	case TouchType::TouchCancelled:
			{
				string keyLifted(tracker[touch]);
				// AC 2014.1.14: sometimes the tracker[touch] returns empty string, a mystery I've yet to solve.
				oldKeys.insert(keyLifted.empty() ? label : keyLifted);
				tracker.erase(touch);
			} break;
			case TouchType::TouchMoved:
				// because the copytext can pop off the last key entered, while the key is down and dragged
				// the same key can be kept added as new
				if (label == lastKeyDown) { // more like last key down
					break;
				}
				if (tracker[touch] != Empty) { // not previously empty (touch was at some other key prior)
					if (tracker[touch] != label) { // pointed at a key different from the one now

						if (isMapped) {
							oldKeys.insert(tracker[touch]);
						}

						if (label != Empty) {

							if (isMapped) {
								newKeys.insert(label);
							}
						}
					}
				} else { // dragged into key (label) from nothing
					if (label != Empty) {
						if (isMapped) {
							newKeys.insert(label);
						}
					}
				}
				tracker[touch] = label;
				break;
			default: break;
		}
		
		if ((newKeys.size() + oldKeys.size()) > 0) {
			std::stringstream report;
			for (auto i : oldKeys) {
				keyPressBuffer.push_back({i, TouchType::TouchEnded});
				report << " -" << i << "]";
			}
			for (auto i : newKeys) {
				keyPressBuffer.push_back({i, TouchType::TouchBegan});
				report << " +" << i << "]";
			}

			LogD2 << "report: [" << report.str();

			// set the key states here!

			// areKeysInvolved
			shared_ptr<KeypressTrackerUpdateInfo> pInfo(new KeypressTrackerUpdateInfo);
			pInfo->newKeysSize = newKeys.size();
			pInfo->oldKeysSize = oldKeys.size();

			pInfo->touchType = type;
			pInfo->label = label;

			if (hasModifierKeys(newKeys)) {
				LogD2 << "modifier keys held!";
				Notif::send("KeypressTracker_ModKeyPressed");
			}

			if (hasModifierKeys(oldKeys)) {
				LogD2 << "modifier keys released!";
				// Notif::send("KeypressTracker_ModKeyReleased");
			}

			Notif::send("KeypressTracker_RequiresUIRefresh", pInfo);

			// this may modify tracker, which kbView relies upon to properly set the key states (up or down)
			GameState::getInstance().copyText().tryProcessingNextBufferedInput();
		}
	}


	set<string> KeypressTracker::keysInDownState() const
	{
		return keysDown(this->tracker);
	}


	set<string> keysDown(const map<CCTouch *, string>& tracker)
	{
		static string Empty = "";
		set<string> ret;
		for (auto &kv : tracker) {
			if (kv.second != Empty) {
				LogD1 << "inserting " << kv.second << " into list of keys down";
				ret.insert(kv.second);
			}
		}
		return ret;
	}


	void KeypressTracker::reset()
	{
		this->tracker.clear();
		this->keyPressBuffer.clear();
	}


	const KeyEvent KeypressTracker::removeNextKeyEventFromBuffer()
	{
		KeyEvent ret = keyPressBuffer.front();
		keyPressBuffer.pop_front();
		
		// assumes there are only two types that are stored in the keypress buffer
		string type;
		switch (ret.type) {
			case TouchType::TouchBegan: type = "Began"; break;
			default: type = "End"; break;
		}
		LogD << "removing from buffer key event '" << ret.key << "' with type: " << type;

		if (ret.key == "") {
			LogW << "hmm";
		}

		return ret;
	}
	
	
	bool locateKeyEvent(const KeyEvent &kev, const deque<KeyEvent> &keyBuffer)
	{
		for (deqIt it = keyBuffer.end(); it!= keyBuffer.begin(); it--) {
			if (*it == kev) {
				return true;
			}
		}
		return false;
	}
	
	
	set<string> keysEnteringTouch(set<string> keysHeldDown, const deque<KeyEvent> &keyBuffer)
	{
		// handle entering
		set<string> entering; // in keysHeldDown but not in keyBuffer
		for (set<string>::const_iterator it = keysHeldDown.begin(); it != keysHeldDown.end(); ++it) {
			bool found(false);
			for (deqIt bufit = keyBuffer.begin(); bufit!= keyBuffer.end(); bufit++) {
				if (bufit->key == *it) {
					found = true;
					break;
				}
			}
			
			if (!found) {
				entering.insert(*it);
			}
		}
		return entering;
	}


	bool hasModifierKeys(const set<string> &keys)
	{
		for (const auto &key : keys) {
			if (utilities::keyIsAModifier(key)) {
				return true;
			}
		}
		return false;
	}
}
