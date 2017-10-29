//
//  GlyphStringTests.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 11/3/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "Glyph.h"

namespace ac {
	
	struct GlyphStringTestFixture
	{
		GlyphStringTestFixture() {}
		~GlyphStringTestFixture() {}
	};
	
	
	BOOST_FIXTURE_TEST_SUITE(GlyphStringTests, GlyphStringTestFixture)
		
	BOOST_AUTO_TEST_CASE(GlyphCodeRetrievalAndModification)
	{
		int code = 235;
		Glyph a(code);
		
		BOOST_REQUIRE_EQUAL(a.getCode(), code);
		
		int newCode = 456;
		a.setCode(newCode);
		
		BOOST_REQUIRE_EQUAL(a.getCode(), newCode);
	}
	
	
	BOOST_AUTO_TEST_CASE(GlyphStringContentOperations)
	{
		GlyphString gs;
		
		BOOST_REQUIRE_EQUAL(gs.size(), 0);
		
		// append 2 new glyphs
		
		gs.append(Glyph(267));
		gs.append(Glyph(222));
		
		BOOST_REQUIRE_EQUAL(gs.size(), 2);
		Glyph g(222);
		
		gs.append(g);
		
		BOOST_REQUIRE_EQUAL(gs.size(), 3);
		
		gs.clear();
		
		BOOST_REQUIRE_EQUAL(gs.size(), 0);

		// resize and append
		gs.clear();


		gs.append(Glyph(1));
		gs.append(Glyph(2));
		gs.append(Glyph(3));
		gs.append(Glyph(4));

		gs.resize(2);

		GlyphString gs2;
		gs2.append(Glyph(1)); gs2.append(Glyph(2));
		BOOST_REQUIRE_EQUAL(gs, gs2); // by now both gs1 and gs2 have two elements

		gs.resize(20);// what will the other 18 elements be filled with? apparently random values.

		BOOST_REQUIRE_EQUAL(gs.size(), 20);
		BOOST_REQUIRE_EQUAL(gs[4], Glyph(-1));
	}
	
	
	BOOST_AUTO_TEST_CASE(GlyphStringCopyCtorAndAssignOp)
	{
		GlyphString gs;
		gs.append(Glyph(12));
		gs.append(Glyph(79));
		gs.append(Glyph(104));
		
		GlyphString gs2(gs);
		BOOST_REQUIRE_EQUAL(gs2.size(), 3);
		BOOST_REQUIRE_EQUAL(gs2[1].getCode(), 79);

		BOOST_REQUIRE(gs == gs2);
		
		// assignment operator
		GlyphString gs3;
		gs3 = gs;
		BOOST_REQUIRE_EQUAL(gs3.size(), 3);
		BOOST_REQUIRE_EQUAL(gs3[1].getCode(), 79);

		BOOST_REQUIRE(gs == gs3);
	}
	
	
	BOOST_AUTO_TEST_CASE(GlyphStringAccessOperations)
	{
		// array indexing
		GlyphString gs;
		gs.append(Glyph(12));
		gs.append(Glyph(79));
		
		BOOST_REQUIRE_NO_THROW(gs[0]);
		BOOST_REQUIRE_THROW(gs[4], std::out_of_range);
	
		gs.clear();
		
		int codes[] = { 1,2,3,4,5,6,7,8,9,10 };
		
		for (int i : codes) {
			gs.append(Glyph(i));
		}
		
		BOOST_REQUIRE_EQUAL(gs.size(), 10);
		
		// substring
		GlyphString gs2 = gs.substr(2, 5); // codes 3-7
		
		BOOST_REQUIRE_EQUAL(gs2.size(), 5);

		BOOST_REQUIRE(gs != gs2);
		
		BOOST_REQUIRE_EQUAL(gs2[0].getCode(), 3);
		BOOST_REQUIRE_EQUAL(gs2[1].getCode(), 4);
		BOOST_REQUIRE_EQUAL(gs2[2].getCode(), 5);
		BOOST_REQUIRE_EQUAL(gs2[3].getCode(), 6);
		BOOST_REQUIRE_EQUAL(gs2[4].getCode(), 7);
		
		// exceptional cases
		BOOST_REQUIRE_THROW(gs.substr(-1, 0), std::out_of_range);
		BOOST_REQUIRE_THROW(gs.substr(50, 5), std::out_of_range);
		
		GlyphString gs3 = gs.substr(8, 20);
		BOOST_REQUIRE_EQUAL(gs3.size(), 2); // only glyphs remain from idx 8
	}


	BOOST_AUTO_TEST_CASE(RandomGlyphGenerationXXX)
	{
		GlyphString glyphString;

		// prepare the set of g
		std::vector<Glyph> glyphs;
		int glyphCodes[] = { 3, 4, 5, 6, 7, 8, 9, 0 };
		for (int g : glyphCodes) {
			glyphs.push_back(Glyph(g));
		}

		glyphString.generateRandom(100, glyphs, { 0.7, 0.5, 0.3 });
		// std::cout << "\n" << glyphString << std::endl;
		return;
	}

	
	
	BOOST_AUTO_TEST_SUITE_END()
}