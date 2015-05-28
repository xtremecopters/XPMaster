#ifndef XPM_MATRIX_H_
#define XPM_MATRIX_H_


#define MATRIX_SCROLLERS	4
#define DRAWSTRING_SLOT		(IOBUFFERS_SSTART + 0)
#define FONT_MAXINDEX		5	// 0 - 5 or 6 total


typedef enum fontChoices
{
    font3x5 = 0,
    font5x7,
    font6x10,
    font8x13,
    gohufont11,
    gohufont11b,
} fontChoices;

typedef enum ScrollMode
{
    wrapForward = 0,
    bounceForward,
    bounceReverse,
    stopped,
    off,
    wrapForwardFromLeft,
} ScrollMode;

typedef enum TextHAlign {
    AlignHLeft = 0,
    AlignHCenter,
    AlignHRight
} TextHAlign;

typedef enum TextVAlign {
    AlignVTop = 0,
    AlignVCenter,
    AlignVBottom
} TextVAlign;

// TextScroller callback events
enum class eScrollerEvent
{
	None = 0,				// No event occurred.
	Stopped,				// Scroller stopped because numScrolls reached zero.
	FIFOAvailable,			// One or more FIFO slots are available
	FIFOEmpty				// All FIFO slots are available and scroller has stopped.
};

// Display mode states
enum eDisplayState
{
  DisplayState_None = 0,
  DisplayState_Settings,
  DisplayState_DateTime,
  DisplayState_Alarm,
  DisplayState_Timer,
  DisplayState_Messages,
  DisplayState_Manual,    // display controlled via serial draw commands

  DisplayState__End
};


// Text Sroller class
class TextScroller
{
private:
	friend class LEDMatrix;
	static int			lastScrollerIndex;
	const int			index;
	int					scrollCounter;

	rgb24				textColor;
	ScrollMode			scrollMode;
	fontChoices			scrollFont;

	struct
	{
		int				x0, x1;
		int				y0, y1;
	}					bounds;
	struct
	{
		std::string		text;	// string buffer making up the text waiting to be displayed
		size_t			pos;	// read position so far into queue that's left to be sent
		size_t			room;	// number of bytes/chars available on Teensy side
		size_t			civ;	// Characters In View, or number of chars that can be visible at once in the scroller
		bool			enabled;
	}					ring;
	

	void prepare();
	void sync();
	void statusUpdate(eScrollerEvent event, uint32_t value);
	
	void syncRing(bool isCB);
	void appendRing(const char *text, size_t);

	
public:
	int					lineCount;
	int					charCount;

	
	TextScroller();
	~TextScroller();


	// scroll text
	void scrollText(const char *text, int numScrolls);
	void setScrollMode(ScrollMode mode);
	void setScrollSpeed(unsigned char pixels_per_second);
	void setScrollFont(fontChoices newFont);
	void setScrollColor(const rgb24 &color);
	void setScrollOffsetFromTop(int offset);
	void setScrollStartOffsetFromLeft(int offset);
	void stopScrollText(void);
	void setScrollBoundary(int x0, int y0, int x1, int y1);
	void appendText(const char *text);

	// returns positive number indicating number of loops left if running
	// returns  0 if stopped
	// returns -1 if continuously scrolling
	int  getScrollStatus(void) const						{ return scrollCounter; }
};


// LED Matrix RPC wrapper class
class LEDMatrix
{
private:
	friend class XpmRPC;
	TextScroller	mScrollers[MATRIX_SCROLLERS];
	struct
	{
		int16_t		x, y;
		uint16_t	interval;
		rpcGIFState state;
	}				mGIF;


	void displaySwapped()
	{
		bufferswaps++;
	}
	void handleRPCDrawing(rpcDrawing cmd, uint8_t *data, size_t size);

	
public:
	int16_t				width;			// LED matrix width in pixels/LEDs
	int16_t				height;			// LED matrix height in pixels/LEDs
	volatile size_t		bufferswaps;	// count of times display swapped framebuffers


	LEDMatrix();
	~LEDMatrix();

	bool prepare();

	// scroll text (backwards compatibility)
	void scrollText(const char inputtext[], int numScrolls)	{ mScrollers[0].scrollText(inputtext, numScrolls); }
	void setScrollMode(ScrollMode mode)						{ mScrollers[0].setScrollMode(mode); }
	void setScrollSpeed(unsigned char pixels_per_second)	{ mScrollers[0].setScrollSpeed(pixels_per_second); }
	void setScrollFont(fontChoices newFont)					{ mScrollers[0].setScrollFont(newFont); }
	void setScrollColor(const rgb24 & newColor)				{ mScrollers[0].setScrollColor(newColor); }
#define setScrollOffsetFromEdge setScrollOffsetFromTop // backwards compatibility
	void setScrollOffsetFromTop(int offset)					{ mScrollers[0].setScrollOffsetFromTop(offset); }
	void setScrollStartOffsetFromLeft(int offset)			{ mScrollers[0].setScrollStartOffsetFromLeft(offset); }
	void stopScrollText(void)								{ mScrollers[0].stopScrollText(); }
	int  getScrollStatus(void) const						{ return mScrollers[0].getScrollStatus(); }

	TextScroller& getScroller(size_t index) { return mScrollers[index]; }


	// display control
	void setBrightness(uint8_t foreground, uint8_t background);
	inline void setBrightness(uint8_t brightness)
		{ setBrightness(brightness, brightness); }
	void swapBuffers(bool copy = true);
	bool waitForVSync(size_t times = 1, bool copy = true);
	bool safeSleep(size_t msec);
	void setMode(eDisplayState mode);

	// drawing functions
	void drawPixel(int16_t x, int16_t y, const rgb24& color);
	void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
	void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const rgb24& color);
	void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const rgb24& color);
	void drawCircle(int16_t x, int16_t y, uint16_t radius, const rgb24& color);
	void fillCircle(int16_t x, int16_t y, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor);
	inline void fillCircle(int16_t x, int16_t y, uint16_t radius, const rgb24& color)
		{ fillCircle(x, y, radius, color, color); }
	void drawEllipse(int16_t x, int16_t y, uint16_t radiusX, uint16_t radiusY, const rgb24& color);
	void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& color);
	inline void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& fillColor)
		{ fillTriangle(x0, y0, x1, y1, x2, y2, fillColor, fillColor); }
	void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& outlineColor, const rgb24& fillColor);
	void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
	inline void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color)
		{ fillRectangle(x0, y0, x1, y1, color, color); }
	void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& outlineColor, const rgb24& fillColor);
	void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor);
	inline void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& fillColor)
		{ fillRoundRectangle(x0, y0, x1, y1, radius, fillColor, fillColor); }
	void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor);
	void fillScreen(const rgb24& color);
	void drawChar(int16_t x, int16_t y, const rgb24& charColor, char character);
	inline void drawString(int16_t x, int16_t y, const rgb24& charColor, const char text[])
		{ drawString(x, y, charColor, charColor, text); }
	void drawString(int16_t x, int16_t y, const rgb24& charColor, const rgb24& backColor, const char text[]);
//	void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const rgb24& bitmapColor, const uint8_t *bitmap);


	// fonts
	void setFont(fontChoices newFont);
	static Point2I getFontStringDims(fontChoices font, const char *text);
	static Point2I getFontCharsInRect(fontChoices font, const Point2I &dims);

	// GIF player control
	void gifLoad(const char *filepath);
	void gifPlay(uint16_t interval = 100); // 100ms default
	void gifStop();
	void gifPosition(int16_t x, int16_t y);
};

extern LEDMatrix matrix;



extern void colorWheel(rgb24 &color, uint8_t WheelPos);


#endif // XPM_MATRIX_H_
