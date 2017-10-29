//
//  CopyTextLoader.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/23/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

namespace ac {
	
	using std::string;
	using std::unique_ptr;
	
	struct CopyTextLoaderImpl;
	
	class CopyTextLoader
	{
	public:
		static CopyTextLoader& getInstance(); // singleton getter
		
		/** after calling this the loaded strings could be accessed through getLine and getNumberOfLines */
		void loadTextFile(const string &filename);
		
		// valid for the next loadTextFile call
		void setMaxCharsPerLine(const size_t maxChars);
		
		bool hasError(); // for tests
		
		const string &getLine(const size_t lineNumber) const;
		const string &getRandomLine() const;
		const size_t getNumberOfLines() const;
		
		// loader can be passed the strings from another class which returns
		// filenames (from the bundle) that
		const string& loadedTextFile() const;
		
	private:
		CopyTextLoader(); // use the public singleton getter instead
		
		// noncopyable
		CopyTextLoader(const CopyTextLoader &);
		CopyTextLoader &operator=(const CopyTextLoader &);
		
		// pImpl
		unique_ptr<CopyTextLoaderImpl> pImpl;
	};
	
	// utility function: don't think we should truncate though.
	inline string getFirstNChars(const string &source, size_t n)
	{
		string ret(source.substr(0, n));
		return ret;
	}
	
}