//
//  PlayerTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/3/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>

#include "Player.h"
#include "PlayerLevel.h"
#include "GameState.h"
#include "BlockCanvas.h"

#include "sqlite3.h"

namespace ac {
	
	struct PlayerLevelTestFixture
	{
		const double diff_thresh = 0.0000001;
		GameState &gs() { return GameState::getInstance(); }
		
		PlayerLevelTestFixture() {
			BlockCanvas::getInstance().setUp();
			// gs().resetGameState();
		}
		
		~PlayerLevelTestFixture() {
			BlockCanvas::getInstance().tearDown();
		}
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(PlayerLevelTests, PlayerLevelTestFixture)
	
	BOOST_AUTO_TEST_CASE(ProgressPerBlockComputations)
	{
		size_t playerLevel;
		
		playerLevel = 1;
		BOOST_REQUIRE_CLOSE_FRACTION(PlayerLevel::levelProgressPerBlock(playerLevel), 1/30.0f, diff_thresh);
		
		playerLevel = 4;
		BOOST_REQUIRE_CLOSE_FRACTION(PlayerLevel::levelProgressPerBlock(playerLevel), 1/40.0f, diff_thresh);
		
		playerLevel = 5;
		BOOST_REQUIRE_CLOSE_FRACTION(PlayerLevel::levelProgressPerBlock(playerLevel), 1/40.0f, diff_thresh);
		
		playerLevel = 9;
		BOOST_REQUIRE_CLOSE_FRACTION(PlayerLevel::levelProgressPerBlock(playerLevel), 1/50.0f, diff_thresh);
	}
	
	
	BOOST_AUTO_TEST_CASE(LevelTimeComputations)
	{
		gs().player().setLevel(4); // assuming 1 second per block here...
		BOOST_REQUIRE_CLOSE_FRACTION(gs().getTimeRemainingValueForLevel(), 40.0, diff_thresh);
		
		gs().player().setLevel(10);
		BOOST_REQUIRE_CLOSE_FRACTION(gs().getTimeRemainingValueForLevel(), 60.0, diff_thresh);
	}
	
	
	BOOST_AUTO_TEST_CASE(CumulativeStreakBonuses)
	{
		const size_t playerLevel = 1;
		const size_t streak = 9;
		// on top of x1, that is
		float cumBonus = PlayerLevel::cumulativeProgressBonusForStreakLevel(streak, playerLevel);
		float perBlock = PlayerLevel::levelProgressPerBlock(playerLevel);
		float totalAddedProgress = cumBonus * perBlock; // fraction of 1
		
		BOOST_REQUIRE_GE(totalAddedProgress, 0); // nonsense
		return;
	}


	BOOST_AUTO_TEST_CASE(SqliteTests)
	{
		sqlite3 *pDB = NULL;
		// char *errMsg = NULL;
		int result;

		// also check other CCFileUtils functions
		string dbPath = CCFileUtils::sharedFileUtils()->getWritablePath();
		dbPath.append("test.db");

		string sqlStatement;


		// result = sqlite3_open(dbPath.c_str(), &pDB); // may fail!

		// this one creates the db if not yet exist
		result = sqlite3_open_v2(dbPath.c_str(), &pDB,
								 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);

		if (SQLITE_OK != result) {
			// error!
			LogE << "Unable to open";
		}

		sqlite3_close(pDB);


	}
	
	
	BOOST_AUTO_TEST_SUITE_END()
}