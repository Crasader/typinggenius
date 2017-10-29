//
//  AppContext.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 7/19/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "appcontext.h"

// boost library for working with json and configuration files
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// boost libraries for working with regular expressions (regex)
#include <boost/regex.hpp>

namespace ac {
	
	using namespace std;
	using boost::property_tree::ptree;
	using boost::property_tree::read_json;
	using boost::property_tree::write_json;
	
	
	void AppContext::performLibrariesDemo()
	{
		doSomeLoggingStuff();
		doSomeJSONStuff();
		doSomeRegexStuff();
	}
	
	
	void doSomeLoggingStuff()
	{
		LogD4 << "Debug4";
		LogD3 << "Debug3";
		LogD2 << "Debug2";
		LogD1 << "Debug1";
		LogD << "Debug";
		LogI << "Info";
		LogW << "Warning";
		LogE << "Error";
	}
	
	
	void doSomeJSONStuff()
	{
		LogI << "Performing Json boost sample code";
		// Write json.
		ptree pt;
		pt.put ("foo", "bar");
		std::ostringstream buf;
		write_json (buf, pt, false);
		std::string json = buf.str(); // {"foo":"bar"}
		
		// Read json.
		ptree pt2;
		std::istringstream is (json);
		read_json (is, pt2);
		std::string foo = pt2.get<std::string> ("foo");
		
		LogI << json;
		LogI << foo;
	}
	
	
	void doSomeRegexStuff()
	{
		LogI << "Performing Regex boost sample code";
		
		boost::regex reg("(A.*)"); // 'A' with 0 or more characters that follow.
		
		string nonmatching = "This expression could match from A and beyond."; // not really
		string matching = "A little lamb";
		
		bool b = boost::regex_match(nonmatching, reg);
		LogI << "Regex nonmatching is matched: " << b;
		
		b = boost::regex_match(matching, reg);
		LogI << "Regex matching is matched: " << b;
	}
	
}