//
//  BoostPTreeHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/11/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//


#include "BoostPTreeHelper.h"
#include "Utilities.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace ac {
	namespace utilities {
		
		PropTree getPropertyTreeFromJSONFileBundle(const string &filename) {
			PropTree pt;
			
			string fullPath = utilities::getFullPathForFilename(filename);
			LogD << "(getPropertyTreeFromJSONFileBundle) full path: " << fullPath;
			
			bool hasError = false;
			try {
				read_json(fullPath, pt);
			} catch (const json_parser_error &obj) {
				LogE << "Problem reading configuration file: " << obj.what();
				hasError = true;
			}
			return pt;
		}
		
		

	}
}