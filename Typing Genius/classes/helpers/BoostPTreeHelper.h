//
//  BoostPTreeHelper.h
//  Typing Genius
//
//  Created by Aldrich Co on 9/11/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#pragma once

#include "Utilities.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace ac {
	namespace utilities {
		
		using boost::property_tree::ptree;
		using boost::property_tree::json_parser_error;
		
		typedef boost::property_tree::ptree PropTree;
		typedef boost::property_tree::ptree::value_type PropTreeValType;
		
		// if PropTree has error, pt.empty() is true
		PropTree getPropertyTreeFromJSONFileBundle(const string &filename);
	}
}