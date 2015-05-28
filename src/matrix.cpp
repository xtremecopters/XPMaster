#include <xpmcommon.h>
#include "rpc.h"
#include "matrix.h"


//=============================================================================
// LED Matrix RPC wrapper class
//=============================================================================
LEDMatrix::LEDMatrix()
:	mGIF({0, 0, 0, rpcGIFState::Stop}),
	width(0),
	height(0),
	bufferswaps(0)
{
	size_t i;


	// assign linear index position
	for(i=0; i<MATRIX_SCROLLERS; i++)
	{
		int *index = (int *)&mScrollers[i].index;
		*index = i;
	}
}
LEDMatrix::~LEDMatrix()
{
	
}

bool LEDMatrix::prepare()
{
	// query the display for necessary information and trigger framebuffer swap to get a completion callback
	rpc.send(rpcType::Display, rpcDisplay::Resolution, NULL, 0);

	// wait for response with swapbuffer message
	bool result = waitForVSync();
	if(!result)
		return result;

	// final initialization
	for(size_t i=0; i<MATRIX_SCROLLERS; i++)
		getScroller(i).prepare();

	// done
	return result;
}


void LEDMatrix::handleRPCDrawing(rpcDrawing cmd, uint8_t *data, size_t size)
{
	switch(cmd)
	{
		case rpcDrawing::ScrollEvent:
		{
			// TextScroller status changed
			
			struct
			{
				uint8_t  id, status;
				uint32_t value;
			} PACKED p; // parameters
			memcpy(&p, data, sizeof(p));
			
			if(p.id >= MATRIX_SCROLLERS)
				break;
			
			getScroller(p.id).statusUpdate((eScrollerEvent)p.status, p.value);
			break;
		}		
		default:
			break;
	}
}


//-----------------------------------------------------------------------------
// Display functions
//-----------------------------------------------------------------------------

void LEDMatrix::setBrightness(uint8_t foreground, uint8_t background)
{
	uint8_t data[2] = { foreground, background };
	rpc.send(rpcType::Display, rpcDisplay::Brightness, data, sizeof(data));
}

void LEDMatrix::swapBuffers(bool copy)
{
	uint8_t i = 0;

	
	if(copy)	i |= 0x01;
	
	rpc.send(rpcType::Display, rpcDisplay::SwapBuffers, &i, sizeof(i));
}

bool LEDMatrix::waitForVSync(size_t times, bool copy)
{
	// spin while polling RPC for requested vertical syncs
	while(times)
	{
		uint8_t i = (copy)? 1:0;
		if(!rpc.send(rpcType::Display, rpcDisplay::SwapBuffers, &i, sizeof(i)))
			return false;
		
		size_t swaps = bufferswaps;
		while(rpc.ok() && (swaps == bufferswaps))
			rpc.poll();

		times--;
	}

	return true;
}

bool LEDMatrix::safeSleep(size_t msec)
{
	int64_t ts;


	while(msec && !gm_Exit)
	{
		if(!rpc.ok())
			return false;
		
		ts = clock_getnstime(CLOCK_MONOTONIC);		// get a nanosecond time stamp
		rpc.poll((msec < 250)? msec : 250);			// prevent possibly taking twice as long as requested
		ts = clock_getnstime(CLOCK_MONOTONIC) - ts;	// get differential in nanoseconds

		// convert differential to milliseconds and decrement milliseconds remaining
		ts   /= 1000000;
		msec -= ((size_t)ts > msec)? msec : (size_t)ts;
		
	}

	return true;
}

void LEDMatrix::setMode(eDisplayState mode)
{
	uint8_t i = (uint8_t)mode;

	rpc.send(rpcType::Display, rpcDisplay::Mode, &i, sizeof(i));
}


//-----------------------------------------------------------------------------
// Drawing functions
//-----------------------------------------------------------------------------

void LEDMatrix::drawPixel(int16_t x, int16_t y, const rgb24& color)
{
	struct
	{
		int16_t  x, y;
		rgb24    color;
	} PACKED p = // parameters
	{
		x, y, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawPixel, &p, sizeof(p));
}

void LEDMatrix::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color)
{
	// test for more optimized draw operations
	if(x0 == x1)
	{
		drawFastVLine(x0, y0, y1, color);
		return;
	} else
	if(y0 == y1)
	{
		drawFastHLine(x0, x1, y0, color);
		return;
	}
	
	struct
	{
		int16_t  x0, y0, x1, y1;
		rgb24    color;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawLine, &p, sizeof(p));
}

void LEDMatrix::drawFastVLine(int16_t x, int16_t y0, int16_t y1, const rgb24& color)
{
	struct
	{
		int16_t  x, y0, y1;
		rgb24    color;
	} PACKED p = // parameters
	{
		x, y0, y1, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawFastVLine, &p, sizeof(p));
}

void LEDMatrix::drawFastHLine(int16_t x0, int16_t x1, int16_t y, const rgb24& color)
{
	struct
	{
		int16_t  x0, x1, y;
		rgb24    color;
	} PACKED p = // parameters
	{
		x0, x1, y, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawFastHLine, &p, sizeof(p));
}

void LEDMatrix::drawCircle(int16_t x, int16_t y, uint16_t radius, const rgb24& color)
{
	struct
	{
		int16_t  x, y, radius;
		rgb24    color;
	} PACKED p = // parameters
	{
		x, y, radius, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawCircle, &p, sizeof(p));
}

void LEDMatrix::fillCircle(int16_t x, int16_t y, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor)
{
	struct
	{
		int16_t  x, y, radius;
		rgb24    colorOutline, colorFill;
	} PACKED p = // parameters
	{
		x, y, radius, outlineColor, fillColor
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::FillCircle, &p, sizeof(p));
}

void LEDMatrix::drawEllipse(int16_t x, int16_t y, uint16_t radiusX, uint16_t radiusY, const rgb24& color)
{
	struct
	{
		int16_t  x, y, radiusX, radiusY;
		rgb24    color;
	} PACKED p = // parameters
	{
		x, y, radiusX, radiusY, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawEllipse, &p, sizeof(p));
}

void LEDMatrix::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& color)
{
	struct
	{
		int16_t  x0, y0, x1, y1, x2, y2;
		rgb24    color;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, x2, y2, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawTriangle, &p, sizeof(p));
}

void LEDMatrix::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& outlineColor, const rgb24& fillColor)
{
	struct
	{
		int16_t  x0, y0, x1, y1, x2, y2;
		rgb24    colorOutline, colorFill;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, x2, y2, outlineColor, fillColor
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::FillTriangle, &p, sizeof(p));
}

void LEDMatrix::drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color)
{
	struct
	{
		int16_t  x0, y0, x1, y1;
		rgb24    color;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, color
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawRectangle, &p, sizeof(p));
}

void LEDMatrix::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& outlineColor, const rgb24& fillColor)
{
	struct
	{
		int16_t  x0, y0, x1, y1;
		rgb24    colorOutline, colorFill;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, outlineColor, fillColor
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::FillRectangle, &p, sizeof(p));
}

void LEDMatrix::drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor)
{
	struct
	{
		int16_t  x0, y0, x1, y1, radius;
		rgb24    color;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, radius, outlineColor
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::DrawRoundRectangle, &p, sizeof(p));
}

void LEDMatrix::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor)
{
	struct
	{
		int16_t  x0, y0, x1, y1, radius;
		rgb24    colorOutline, colorFill;
	} PACKED p = // parameters
	{
		x0, y0, x1, y1, radius, outlineColor, fillColor
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::FillRoundRectangle, &p, sizeof(p));
}

void LEDMatrix::fillScreen(const rgb24& color)
{
	rpc.send(rpcType::Drawing, rpcDrawing::FillScreen, &color, sizeof(color));
}

void LEDMatrix::drawChar(int16_t x, int16_t y, const rgb24& charColor, char character)
{
	struct
	{
		int16_t  x, y;
		rgb24    color;
		char     chr;
	} PACKED p = // parameters
	{
		x, y, charColor, character
	};

	rpc.send(rpcType::Drawing, rpcDrawing::DrawChar, &p, sizeof(p));
}

void LEDMatrix::drawString(int16_t x, int16_t y, const rgb24& charColor, const rgb24& backColor, const char text[])
{
	// TODO: avoid transfering duplicate strings

	if(!rpc.transfer(DRAWSTRING_SLOT, (uint8_t *)text, strlen(text)))
		return;

	struct
	{
		int16_t  x, y;
		rgb24    colorFG, colorBG;
		uint8_t  buffer;
	} PACKED p = // parameters
	{
		x, y, charColor, backColor, DRAWSTRING_SLOT
	};

	rpc.send(rpcType::Drawing, rpcDrawing::DrawString, &p, sizeof(p));
}


//-----------------------------------------------------------------------------
// Font functions
//-----------------------------------------------------------------------------

// cheating for now, but it works
static const Point2I fonttable[FONT_MAXINDEX +1] =
{
	// width, height
	{ 4,  6 },	// apple3x5 / font3x5
	{ 5,  7 },	// apple5x7 / font5x7
	{ 6, 10 },	// apple6x10 / font6x10
	{ 8, 13 },	// apple8x13 / font8x13
	{ 6, 11 },	// gohufont6x11 / gohufont11
	{ 6, 11 }	// gohufont6x11b / gohufont11b
};


void LEDMatrix::setFont(fontChoices newFont)
{
	uint8_t i = (uint8_t)newFont;
	rpc.send(rpcType::Drawing, rpcDrawing::SetFont, &i, sizeof(i));
}

Point2I LEDMatrix::getFontStringDims(fontChoices font, const char *text)
{
	Point2I	result; // defaults to 0,0
	size_t	fontnum	= (size_t)font;
	size_t	length;
	size_t	lines	= 1 + strchrcount(text, '\n', &length);


	if(length && (fontnum <= FONT_MAXINDEX))
	{
		result    = fonttable[fontnum];
		result.x *= (int16_t)length;
		result.y *= (int16_t)lines;
	}

	return result;
}

Point2I LEDMatrix::getFontCharsInRect(fontChoices font, const Point2I &dims)
{
	Point2I result; // defaults to 0,0
	size_t	fontnum	= (size_t)font;

	
	if(fontnum <= FONT_MAXINDEX)
	{
		result.x = dims.x / fonttable[fontnum].x;
		result.y = dims.y / fonttable[fontnum].y;

		if(dims.x % fonttable[fontnum].x)	result.x++;
		if(dims.y % fonttable[fontnum].y)	result.y++;
	}
	
	return result;
}


//-----------------------------------------------------------------------------
// GIF player control
//-----------------------------------------------------------------------------

void LEDMatrix::gifLoad(const char *filepath)
{
	if(!rpc.transfer(DRAWSTRING_SLOT, (uint8_t *)filepath, strlen(filepath)))
		return;

	struct
	{
		uint8_t  control;
		uint8_t  buffnum;
	} PACKED p = // parameters
	{
		(uint8_t)(RPCGIF_FLAG_LOAD | ((uint8_t)mGIF.state & RPCGIF_MASK_STATE)),
		DRAWSTRING_SLOT
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::GIFAnimation, &p, sizeof(p));
}

void LEDMatrix::gifPlay(uint16_t interval)
{
	mGIF.state		= rpcGIFState::Play;
	mGIF.interval	= interval;

	struct
	{
		uint8_t   control;
		uint16_t  x, y;
		uint16_t  interval;
	} PACKED p = // parameters
	{
		(uint8_t)(RPCGIF_FLAG_POS | RPCGIF_FLAG_INTERVAL | ((uint8_t)mGIF.state & RPCGIF_MASK_STATE)),
		mGIF.x, mGIF.y, mGIF.interval
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::GIFAnimation, &p, sizeof(p));
}

void LEDMatrix::gifStop()
{
	mGIF.state = rpcGIFState::Stop;
	
	struct
	{
		uint8_t   control;
	} PACKED p = // parameters
	{
		(uint8_t)((uint8_t)mGIF.state & RPCGIF_MASK_STATE)
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::GIFAnimation, &p, sizeof(p));
}

void LEDMatrix::gifPosition(int16_t x, int16_t y)
{
	mGIF.x = x;
	mGIF.y = y;

	// don't send command when GIF isn't visible
	if(mGIF.state == rpcGIFState::Stop)
		return;

	struct
	{
		uint8_t   control;
		uint16_t  x, y;
	} PACKED p = // parameters
	{
		(uint8_t)(RPCGIF_FLAG_POS | ((uint8_t)mGIF.state & RPCGIF_MASK_STATE)),
		x, y
	};
	
	rpc.send(rpcType::Drawing, rpcDrawing::GIFAnimation, &p, sizeof(p));
}



//=============================================================================
// Text Scrollers class
//=============================================================================
int TextScroller::lastScrollerIndex = -1;

TextScroller::TextScroller()
 :	index(0),
	scrollCounter(0),
	textColor({0xff, 0xff, 0xff}),
	scrollMode(bounceForward),
	scrollFont(font5x7),
	bounds({-1, -1, -1, -1}),
	lineCount(0),
	charCount(0)
{
	ring.pos	 = 0;
	ring.room	 = 0;
	ring.enabled = false;
}
TextScroller::~TextScroller()
{
}

void TextScroller::prepare()
{
	bounds.x0 = 0;
	bounds.x1 = matrix.width -1;
	bounds.y0 = 0;
	bounds.y1 = matrix.height -1;
}

void TextScroller::sync()
{
	if(index == lastScrollerIndex)
		return;

	lastScrollerIndex = index;

	uint8_t i = (uint8_t)index;
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollSelect, &i, sizeof(i));
}

void TextScroller::statusUpdate(eScrollerEvent event, uint32_t value)
{
	switch(event)
	{
		case eScrollerEvent::Stopped:
		{
			scrollCounter = 0;
			break;
		}
		
		case eScrollerEvent::FIFOAvailable:
		case eScrollerEvent::FIFOEmpty:
		{
			ring.room = value;
			
			if(ring.enabled)
				syncRing(true);
			break;
		}

		default:
			return;
	}
}

static size_t copyAndStripToken(std::string &dest, const char *src, size_t size, char token, size_t &found)
{
	size_t	removes = 0;
	int		i, last=0;

	
	// copy passed string buffer
	dest.assign(src, size);

	// remove any matching tokens
	for(i = 0; i<(int)size; i++)
	{
		// check for delimiter
		if(dest[i] == token)
		{
			// remove character
			dest.erase(dest.begin() + i);
			size -= 1;
			i	 -= 1;			
			last  = i;
			
			removes++;
		}
	}

	found = removes;

	// return last seen position of a token
	return last + removes;
}

void TextScroller::syncRing(bool isCB)
{
	if(ring.text.empty())
		return;
	
	if(!ring.enabled)
	{
		const Point2I	dims(bounds.x1 - bounds.x0 +1, bounds.y1 - bounds.y0 +1);
		
		ring.pos		= 0;
		ring.room		= IOBUFFERS_LSIZE;
		ring.civ		= LEDMatrix::getFontCharsInRect(scrollFont, dims).x;
		ring.enabled	= true;

		// prevent having to refill too often to avoid wasting USB through put
		if(ring.civ < (ring.room / 4))
			ring.civ = ring.room / 4;

		// prevent overly complicating the buffer xfers by keeping a chunk size
		// at or below the xfer buffer we utilize. -2 offset for reciving side
		// string NULL terminator storage and our insertion of item delimiter.
		if(ring.civ > (IOBUFFERS_SSIZE -2))
			ring.civ = (IOBUFFERS_SSIZE -2);
	}

	std::string str;

	for(;;)
	{
		size_t size = ring.text.size() - ring.pos;
		
		if(!size || (!isCB && (size < ring.civ) && (ring.room != IOBUFFERS_LSIZE)) || (ring.room < ring.civ))
			break;

		if(size > ring.civ)
			size = ring.civ;

		size_t found;
		/*size_t pos = */copyAndStripToken(str, ring.text.c_str() + ring.pos, size, '\n', found);
		str += "\n";
		appendRing(str.c_str(), str.size());

		ring.pos += size;

		if(found)
		{
			ring.text.erase(0, ring.pos);
			ring.pos   = 0;
			
			lineCount -= found;
			charCount  = ring.text.size();
		}
		
		ring.room -= str.size();
	}

}



//-----------------------------------------------------------------------------
// Scroll text functions
//-----------------------------------------------------------------------------

void TextScroller::scrollText(const char *text, int numScrolls)
{
	sync();

	size_t size = strlen(text);
	if(!rpc.transfer(IOBUFFERS_LSTART + this->index, (uint8_t *)text, size))
		return;

	uint8_t data[2] = { (uint8_t)ScrollState_Start, (uint8_t)numScrolls };
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollState, data, sizeof(data));

	scrollCounter	= numScrolls;
	ring.enabled	= false;
	lineCount		= 1;
	charCount		= size;
	
	ring.text.clear();
}

void TextScroller::setScrollMode(ScrollMode mode)
{
	uint8_t i = (uint8_t)mode;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollMode, &i, sizeof(i));

	scrollMode = mode;
}

void TextScroller::setScrollSpeed(unsigned char pixels_per_second)
{
	uint8_t i = (uint8_t)pixels_per_second;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollSpeed, &i, sizeof(i));
}

void TextScroller::setScrollFont(fontChoices newFont)
{
	uint8_t i = (uint8_t)newFont;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollFont, &i, sizeof(i));

	scrollFont = newFont;
}

void TextScroller::setScrollColor(const rgb24 &color)
{
	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollColor, (uint8_t *)&color, sizeof(color));

	textColor = color;
}

void TextScroller::setScrollOffsetFromTop(int offset)
{
	int16_t i = (int16_t)offset;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollOffsetVertical, (uint8_t *)&i, sizeof(i));
}

void TextScroller::setScrollStartOffsetFromLeft(int offset)
{
	int16_t i = (int16_t)offset;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollOffsetHorizontal, (uint8_t *)&i, sizeof(i));
}

void TextScroller::stopScrollText(void)
{
	uint8_t i = (uint8_t)ScrollState_Stop;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollState, &i, sizeof(i));

	scrollCounter	= 0;
	ring.enabled	= false;
	lineCount		= 0;
	charCount		= 0;

	ring.text.clear();
}

void TextScroller::setScrollBoundary(int x0, int y0, int x1, int y1)
{
	int16_t params[4] =
	{
		(int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1
	};
	
	bounds.x0 = x0;
	bounds.x1 = x1;
	bounds.y0 = y0;
	bounds.y1 = y1;

	sync();
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollBoundary, params, sizeof(params));
}

void TextScroller::appendRing(const char *text, size_t size)
{
	sync();

	uint8_t buffnum = IOBUFFERS_SSTART + this->index;
	if(!rpc.transfer(buffnum, (uint8_t *)text, size))
		return;

	uint8_t data[2] = { (uint8_t)ScrollState_Append, buffnum };
	rpc.send(rpcType::Drawing, rpcDrawing::ScrollState, data, sizeof(data));

	scrollCounter = 1;
}

void TextScroller::appendText(const char *text)
{
	size_t size, count;


	count = strchrcount(text, '\n', &size);
	if(size)
	{
		ring.text += text;
		ring.text += "\n";

		lineCount += count + 1;
		charCount  = ring.text.size();
	}
	
	syncRing(false);
}


//=============================================================================
// Other misc. graphics related functions
//=============================================================================

void colorWheel(rgb24 &color, uint8_t WheelPos)
{
  if(WheelPos < 85)
  {
    color.red   = WheelPos * 3;
    color.green = 255 - WheelPos * 3;
    color.blue  = 0;
    return;
  } else if(WheelPos < 170)
  {
    WheelPos   -=  85;
    color.red   = 255 - WheelPos * 3;
    color.green = 0;
    color.blue  = WheelPos * 3;
    return;
  } else
  {
    WheelPos   -= 170;
    color.red   = 0;
    color.green = WheelPos * 3;
    color.blue  = 255 - WheelPos * 3;
  }
}
