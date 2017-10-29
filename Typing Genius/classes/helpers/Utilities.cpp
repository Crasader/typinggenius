//
//  Utilities.cpp
//  Typing Genius
//
//  Created by Aldrich Co on 8/25/13.
//  Copyright (c) 2013 Aldrich Co. All rights reserved.
//

#include "Utilities.h"

namespace ac
{
	namespace utilities
	{
		// note: using time(0) as seed is not cryptographically secure, so don't use this for
		// similar purposes

		const RGBByte randomRGBByte()
		{
			boost::random::uniform_int_distribution<> rUbyte(0,255); // give it something darker

			unsigned char r = rUbyte(rng);
			unsigned char g = rUbyte(rng);
			unsigned char b = rUbyte(rng);
			
			// LogD << "rgb(" << (int)r << "," << (int)g << "," << (int)b << ")";
			return (RGBByte) { r, g, b };
		}
		
		
		const RGBByte nextRGBByteHueShift(const RGBByte &in, float H /* hue shift in degrees up to 359*/)\
		{
			
			float VSU = cos(H*M_PI/180);
			float VSW = sin(H*M_PI/180);
			
			int r = (.299+.701*VSU+.168*VSW)*in.r
			+ (.587-.587*VSU+.330*VSW)*in.g
			+ (.114-.114*VSU-.497*VSW)*in.b;
			int g = (.299-.299*VSU-.328*VSW)*in.r
			+ (.587+.413*VSU+.035*VSW)*in.g
			+ (.114-.114*VSU+.292*VSW)*in.b;
			int b = (.299-.3*VSU+1.25*VSW)*in.r
			+ (.587-.588*VSU-1.05*VSW)*in.g
			+ (.114+.886*VSU-.203*VSW)*in.b;
			
			RGBByte ret;
			ret.r = r;
			ret.g = g;
			ret.b = b;
			return ret;
		}


		HSVFloat rgb2Hsv(const RGBFloat &in)
		{
			HSVFloat    out;
			double      min, max, delta;

			min = in.r < in.g ? in.r : in.g;
			min = min  < in.b ? min  : in.b;

			max = in.r > in.g ? in.r : in.g;
			max = max  > in.b ? max  : in.b;

			out.v = max;                                // v
			delta = max - min;
			if( max > 0.0 ) {
				out.s = (delta / max);                  // s
			} else {
				// r = g = b = 0                        // s = 0, v is undefined
				out.s = 0.0;
				out.h = NAN;                            // its now undefined
				return out;
			}
			if( in.r >= max )                           // > is bogus, just keeps compilor happy
				out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
			else
				if( in.g >= max )
					out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
				else
					out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

			out.h *= 60.0;                              // degrees

			if( out.h < 0.0 )
				out.h += 360.0;
			
			return out;
		}


		RGBFloat hsv2Rgb(const HSVFloat &in)
		{
			double      hh, p, q, t, ff;
			long        i;
			RGBFloat    out;

			if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
				out.r = in.v;
				out.g = in.v;
				out.b = in.v;
				return out;
			}
			hh = in.h;
			if(hh >= 360.0) hh = 0.0;
			hh /= 60.0;
			i = (long)hh;
			ff = hh - i;
			p = in.v * (1.0 - in.s);
			q = in.v * (1.0 - (in.s * ff));
			t = in.v * (1.0 - (in.s * (1.0 - ff)));

			switch(i) {
				case 0:
					out.r = in.v;
					out.g = t;
					out.b = p;
					break;
				case 1:
					out.r = q;
					out.g = in.v;
					out.b = p;
					break;
				case 2:
					out.r = p;
					out.g = in.v;
					out.b = t;
					break;
				case 3:
					out.r = p;
					out.g = q;
					out.b = in.v;
					break;
				case 4:
					out.r = t;
					out.g = p;
					out.b = in.v;
					break;
				case 5: default:
					out.r = in.v;
					out.g = p;
					out.b = q;
					break;
			}
			return out;
		}


		const size_t random(size_t max)
		{
			boost::random::uniform_int_distribution<> rSizeT(0, max);
			return rSizeT(rng);
		}

		const float randomFloat(float min, float max)
		{
			static boost::random::uniform_real_distribution<> randomFloat(0, 1);
			float zeroToOne = randomFloat(utilities::rng); // random float bet. 0 & 1
			return min + zeroToOne * (max - min);
		}


	}
}