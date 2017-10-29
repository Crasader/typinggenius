//
//  CopyTextLoader.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "CopyTextLoader.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp> // for boost::trim
#include "Utilities.h"
#include "DebugSettingsHelper.h"

namespace ac {
	
	using std::vector;
	using boost::property_tree::ptree;
	using boost::property_tree::json_parser_error;
	
#pragma mark - pImpl
	
	struct CopyTextLoaderImpl
	{
		CopyTextLoaderImpl() : lines(), pt(), loadingHasError(false), filename(), maxCharsPerLine(0) {}
		
		vector<string> lines;
		ptree pt;
		bool loadingHasError;
		
		string filename;
		size_t maxCharsPerLine;
	};
	
#pragma mark - Singleton
	
	CopyTextLoader& CopyTextLoader::getInstance()
	{
		static CopyTextLoader instance;
		return instance;
	}
	
#pragma mark - Lifetime
	
	CopyTextLoader::CopyTextLoader()
	{
		pImpl.reset(new CopyTextLoaderImpl);
	}
	
	
#pragma mark - Initialize
	void CopyTextLoader::loadTextFile(const string &filename)
	{
		LogI << "Loading text file copy from " << filename;
		pImpl->lines.clear();
		pImpl->loadingHasError = false;
		
		string fullPath = utilities::getFullPathForFilename(filename);
		try {
			read_json(fullPath, pImpl->pt);
		} catch (const json_parser_error &obj) {
			LogE << "Problem loading copy file: " << obj.what();
			pImpl->loadingHasError = true;
			return;
		}
		
		pImpl->filename = filename;
		
		// read lines into lines one by one.
		string content(pImpl->pt.get<string>("content", ""));
		
		if (!content.empty()) {
			std::stringstream ss(content);
			string to;
			
			while (std::getline(ss, to, '\n')) {
				// to : a single line
				
				// if the whole line fits within the param, push it all in
				// otherwise, only push the first n chars (not splitting within lexical words) and then repeat
				
				boost::char_separator<char> sep(" ");
				
				boost::tokenizer<boost::char_separator<char>> tokens(to, sep);
				
				std::ostringstream os;
				
				for (const auto& t : tokens) {
					if (pImpl->maxCharsPerLine != 0 &&
						t.size() + os.str().size() > pImpl->maxCharsPerLine) {

						// wrap
						if (!os.str().empty()) {
							LogD << "Pushed: " << os.str();
							pImpl->lines.push_back(boost::trim_copy(os.str()));
						}
						os.str("");
					}
					os << t << " ";
				}
				
				// remaining string
				if (!os.str().empty()) {
					LogD << "Pushed: " << os.str();
					pImpl->lines.push_back(boost::trim_copy(os.str()));
				}
			}
			
//			BOOST_FOREACH(const auto &string, pImpl->lines) {
//				LogI << string;
//			}
			
		}
	}
	
	
#pragma mark - Getters & Setters
	
	const string &CopyTextLoader::getLine(const size_t lineNumber) const
	{
		static string emptyString = "";
		
		size_t numLines = pImpl->lines.size();
		if (lineNumber < numLines) {
			LogD << boost::format("the line: [%s]") % pImpl->lines[lineNumber];
			return pImpl->lines[lineNumber];
		}
		
		return emptyString;
	}


	const string &CopyTextLoader::getRandomLine() const
	{
//		size_t numberOfLines(getNumberOfLines());
//		return getLine(utilities::random(numberOfLines - 1));
		
		return getLine(0);
	}
	
	
	const size_t CopyTextLoader::getNumberOfLines() const
	{
		return pImpl->lines.size();
	}
	
	
	const string& CopyTextLoader::loadedTextFile() const
	{
		return pImpl->filename;
	}
	
	
	void CopyTextLoader::setMaxCharsPerLine(const size_t maxChars)
	{
		pImpl->maxCharsPerLine = maxChars;
	}
	
	
#pragma mark - Miscellaneous
	
	bool CopyTextLoader::hasError()
	{
		return pImpl->loadingHasError;
	}	
}