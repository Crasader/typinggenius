/*
 * Player
 * Typing Genius
 *
 * Created by Aldrich Co on 11/21/13.
 * Copyright (c) 2013 Aldrich Co. All rights reserved.
 *
 */

#include "Player.h"
#include "sqlite3.h"
#include "cocos2d.h"
#include "ScoreKeeper.h"
#include "GameState.h"

namespace ac
{
	const char *DBPath = "Player.db";

	USING_NS_CC;
	using std::string;
	using std::vector;
	using std::map;

	static int showTableRowsCallback(void *NotUsed, int argc, char **argv, char **azColName);
	static vector<map<string, string>> doQuery(sqlite3 *database, const char* query);


#pragma mark - pImpl

	struct PlayerImpl
	{
		PlayerImpl(const std::string& playerName, const size_t level) :
		playerId(), playerName(playerName), level(level), currencyCollected(0),
		totalScore(0), pDB()
		{
			openDB();
			if (this->pDB) {
				initializeDB();
			}
		}

		// Player attributes
		size_t playerId; // for the db
		std::string playerName;

		PlayerLevel level; // represents current level

		// last session stats
		size_t totalScore; // across all levels, 1 playthrough
		long datePlayed;

		// career stats
		size_t topScore;
		size_t totalBlocksCleared;
		size_t totalCorrectCount; // is this same as totalBlocksCleared?
		size_t totalMistakeCount;
		size_t longestStreak;
		size_t currencyCollected; // would be in "X frogs"
		size_t topLevel;


		sqlite3 *pDB;

		void openDB();
		void closeDB();
		void initializeDB();
		int createNewDBEntry();
	};


#pragma mark - Lifetime

	Player::Player(const std::string& name, const size_t level)
	{
		pImpl.reset(new PlayerImpl(name, level));
	}


	// never called though, as Player is part of a major singleton (GameState).
	Player::~Player()
	{
		pImpl->closeDB();
	}


#pragma mark - Player Public Functions

	void Player::incrementPlayerLevel()
	{
		pImpl->level.incrementLevel();
	}


	void Player::resetPlayer()
	{
		pImpl->level.reset();
		pImpl->totalScore = 0;
	}

	const std::string &Player::getName() const
	{
		return pImpl->playerName;
	}


	size_t Player::getLevel() const
	{
		return pImpl->level.getLevel();
	}


	void Player::setLevel(size_t level)
	{
		pImpl->level.setLevel(level);
	}


	size_t Player::getTotalScore() const
	{
		return pImpl->totalScore;
	}


	void Player::addToTotalScore(size_t scoreToBeAdded)
	{
		pImpl->totalScore += scoreToBeAdded;
	}


	size_t Player::getCurrencyAmount() const
	{
		return pImpl->currencyCollected;
	}


	void Player::addToCurrencyOwned(size_t amount)
	{
		pImpl->currencyCollected += amount;
		LogI << "currency (frogs) now at " << pImpl->currencyCollected;
	}
	
	void Player::deductCurrencyOwned(size_t amount)
	{
		LogI << "deducting " << amount << " frogs from Player....";
		pImpl->currencyCollected -= amount;
	}


#pragma mark - Database Operations

	// shouldn't throw during construction, when this is called.
	void PlayerImpl::openDB()
	{
		sqlite3 *pDB = NULL;
		// also check other CCFileUtils functions
		string dbPath = CCFileUtils::sharedFileUtils()->getWritablePath();
		dbPath.append(DBPath);

		// this one creates the db if not yet exist
		int result = sqlite3_open_v2(dbPath.c_str(), &pDB,
									 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);

		if (SQLITE_OK != result) { // error!
			LogE << "Unable to open db";
			pDB = NULL;
		}

		this->pDB = pDB;
	}


	void PlayerImpl::closeDB()
	{
		if (this->pDB) {
			sqlite3_close_v2(this->pDB);
		}
	}
	

	// called during Player construction. Do not throw!
	void PlayerImpl::initializeDB()
	{
		using std::stoi;

		// An attempt to create the table will be made, if it does not exist yet. No errors if it does (but
		// discounting all other errors).
		const char *CreateTableSql =
		"CREATE TABLE IF NOT EXISTS Player ( "\
			"PlayerName         VARCHAR(15) , "\
			"CurrencyCollected  INT         NOT NULL, "\
			"TopScore           INT         NOT NULL, "\
			"TopLevel           INT         NOT NULL, "\
			"TotalBlocksCleared INT         NOT NULL, "\
			"TotalCorrectCount  INT         NOT NULL, "\
			"TotalMistakeCount  INT         NOT NULL, "\
			"LongestStreak      INT         NOT NULL, "\
			"DateLastPlayed     DATETIME    "\
		");";

		char *errMsg = NULL;
		int result = sqlite3_exec(pDB, CreateTableSql, showTableRowsCallback, 0, &errMsg);
		if (SQLITE_OK != result) {
			LogW << "SQL error (creating DB!)";
			sqlite3_free(errMsg);
		} else {
			LogI << "Initialized DB.. Player table is ready";
		}
		
		// now i assume there's only one player that's going to be loaded. it's the last one.
		// select that row, load up the Player fields with it.
		
		const char *GetPlayersSql = "SELECT rowid, * FROM Player ORDER BY ROWID DESC LIMIT 1";
		vector<map<string, string>> playerRows = doQuery(pDB, GetPlayersSql);
		if (playerRows.size() > 0) { // found something.
			map<string, string> lastPlayer(playerRows.front()); // the only element.
			this->playerId = stoi(lastPlayer["rowid"]);
			this->playerName = lastPlayer["PlayerName"];
			this->level.setLevel(1); // not yet figured how to resume level properly yet
			this->currencyCollected = stoi(lastPlayer["CurrencyCollected"]);
			this->topScore = stoi(lastPlayer["TopScore"]);
			this->totalBlocksCleared = stoi(lastPlayer["TotalBlocksCleared"]);
			this->totalCorrectCount = stoi(lastPlayer["TotalCorrectCount"]);
			this->totalMistakeCount = stoi(lastPlayer["TotalMistakeCount"]);
			this->longestStreak = stoi(lastPlayer["LongestStreak"]);
			this->datePlayed = lastPlayer["DateLastPlayed"].empty() ? 0 : stoi(lastPlayer["DateLastPlayed"]);

		} else {
			// empty, so create a new entry.
			this->playerId = createNewDBEntry();  // whose id is 1.
		}
	}


	// int return value is the player id (0 when not success)
	int PlayerImpl::createNewDBEntry()
	{
		const char *InsertQuery =
		"INSERT INTO PLAYER \
		(PlayerName, CurrencyCollected, TopScore, TopLevel, TotalBlocksCleared, \
		TotalCorrectCount, TotalMistakeCount, LongestStreak, DateLastPlayed) \
		VALUES \
		('Unnamed', 0, 0, 0, 0, 0, 0, 0, NULL)";

		sqlite3_stmt *statement;

		if (SQLITE_OK == sqlite3_prepare_v2(this->pDB, InsertQuery, -1, &statement, NULL)) {
			if (SQLITE_DONE == sqlite3_step(statement)) {
				LogD1 << "Successfully created new Player entry in the DB";
				int rowId = sqlite3_last_insert_rowid(this->pDB);
				return rowId;
			} else {
				LogW << "Inserting player failed for some reason";
			}
		} else {
			LogW << "Problem preparing 'create player' insert statement";
		}

		return 0;
	}
	
	
	void Player::syncStatsToDB(const ScoreKeeper &sk)
	{
		if (!pImpl->pDB) {
			string error("Failed to open SQLite database!");
			throw error;
		}

		// current values of player are sent back to the db Player entry
		// has to be loaded

		pImpl->totalScore = sk.getTotalScore();
		pImpl->datePlayed = GameState::getTimeNow();
		pImpl->topScore = MAX(pImpl->topScore, sk.getTotalScore());
		pImpl->totalBlocksCleared += sk.getCorrectCount();
		pImpl->totalCorrectCount += sk.getCorrectCount();
		pImpl->totalMistakeCount += sk.getMistakeCount();
		pImpl->longestStreak = MAX(pImpl->longestStreak, sk.getLongestStreak());
		pImpl->topLevel = MAX(pImpl->topLevel, pImpl->level.getLevel());

		const char *UpdateQuery = // match bindings by row number starting from 1
		"UPDATE Player SET PlayerName = @PlayerName, CurrencyCollected = @CurrencyCollected, TopScore = @TopScore, TopLevel = @TopLevel, TotalBlocksCleared = @TotalBlocksCleared, TotalCorrectCount = @TotalCorrectCount, TotalMistakeCount = @TotalMistakeCount, LongestStreak = @LongestStreak, DateLastPlayed = @DateLastPlayed WHERE ROWID = @ROWID";

		sqlite3_stmt *stmt;
		int res = sqlite3_prepare_v2(pImpl->pDB, UpdateQuery, -1, &stmt, NULL);
		if (SQLITE_OK == res) {

			sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, "@PlayerName"), pImpl->playerName.c_str(), -1, NULL);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@CurrencyCollected"), pImpl->currencyCollected);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@TopScore"), pImpl->topScore);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@TopLevel"), pImpl->topLevel);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@TotalBlocksCleared"), pImpl->totalBlocksCleared);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@TotalCorrectCount"), pImpl->totalCorrectCount);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@TotalMistakeCount"), pImpl->totalMistakeCount);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@LongestStreak"), pImpl->longestStreak);

			sqlite3_bind_int64(stmt, sqlite3_bind_parameter_index(stmt, "@DateLastPlayed"), pImpl->datePlayed);

			sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@ROWID"), pImpl->playerId);

			if (SQLITE_DONE == sqlite3_step(stmt)) {
				LogD1 << "Successfully updated Player entry in the DB";
			} else {
				LogW << "Updating player failed for some reason";
			}
		} else {
			LogW << "Problem preparing 'update player' statement";
		}
	}


#pragma mark - Miscellaneous

	// query meant to select rows from a table. returns the results as a map of string-string
	static vector<map<string, string>> doQuery(sqlite3 *database, const char* query)
	{
		sqlite3_stmt *statement;
		vector<map<string, string>> resultsMap;
		if (sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK) {
			int cols = sqlite3_column_count(statement);
			int result = 0;
			while (true) {
				result = sqlite3_step(statement);
				if (result == SQLITE_ROW) {
					vector<string> values;
					map<string, string> valuesMap;
					for (size_t col = 0; col < cols; col++) {
						const string colName(sqlite3_column_name(statement, col));

						// could be NULL
						const char *colValue = (const char *)sqlite3_column_text(statement, col);

						// add the column values as strings to 'values'
						valuesMap[colName] = colValue ? string(colValue) : "";
					}
					resultsMap.push_back(valuesMap);
				} else {
					break;
				}
			}
			sqlite3_finalize(statement);
		}
		string error = sqlite3_errmsg(database);
		if (error != "not an error") {
			LogW << query << " " << error;
		}
		return resultsMap;
	}


	static int showTableRowsCallback(void *NotUsed, int argc, char **argv, char **azColName) {
		for (size_t i = 0; i < argc; i++) {
			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		}
		return 0;
	}
}

