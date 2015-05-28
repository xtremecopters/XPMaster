#ifndef XPMCOMMON_H_
#define XPMCOMMON_H_

#define _XOPEN_SOURCE

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>


typedef std::string		string;			// universal string


extern volatile bool gm_Exit;

extern time_t getLocalTimestamp();


//=============================================================================
// IR Remote
//=============================================================================

#define IR_RECV_POLLRATE  200    // milliseconds

enum class InputCommand
{
    None = 0,
    Same, // same code as before
    BrightDec,
    BrightInc,
    PlayPause,
    Setup,
    StopMode,
    Up,
    Down,
    Left,
    Right,
    Select,
    ZeroTen,
    Back,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    
    _Count
//    Brightness,
//    PlayMode,
//    Palette,
//    Clock,
//    Power,
//    Back,
//    BrightnessUp,
//    BrightnessDown,
//    Menu,
};

extern const char* commandToString(InputCommand cmd);


//=============================================================================
// Utility macros
//=============================================================================
#define IOBUFFERS_COUNT    8  // unified I/O buffer count
#define IOBUFFERS_SSIZE   64  // each small buffer size in bytes
#define IOBUFFERS_SCOUNT   4  // number of small buffers
#define IOBUFFERS_SSTART   0
#define IOBUFFERS_LSIZE  256  // each large buffer size in bytes
#define IOBUFFERS_LCOUNT   4  // number of large buffers
#define IOBUFFERS_LSTART   IOBUFFERS_SCOUNT

struct tIOBuffer
{
  const uint16_t  size;   // buffer capacity in bytes
  uint16_t        used;   // used capacity in bytes
  uint8_t * const data;   // pointer to storage buffer
};

extern tIOBuffer gm_IOBuffers[IOBUFFERS_COUNT];


#define PTRLEN(_s, _e)      ((size_t)(_e) - (size_t)(_s))
#define ARRAYSIZE(_a)       (sizeof(_a) / sizeof(_a[0]))

#define MakeUInt16(_l, _h)  (((uint16_t)_l) | (((uint16_t)_h) <<  8))
#define MakeUInt32(_l, _h)  (((uint32_t)_l) | (((uint32_t)_h) << 16))
#define MakeUInt64(_l, _h)  (((uint64_t)_l) | (((uint64_t)_h) << 32LL))

#define UInt8PToUInt16(_p)   MakeUInt16(_p[0], _p[1])
#define UInt8PToUInt32(_p)   MakeUInt32(MakeUInt16(_p[0], _p[1]), MakeUInt16(_p[2], _p[3]))
#define UInt8PToUInt64(_p)   MakeUInt64(MakeUInt32(MakeUInt16(_p[0], _p[1]), MakeUInt16(_p[2], _p[3])), \
                                        MakeUInt32(MakeUInt16(_p[4], _p[5]), MakeUInt16(_p[6], _p[7])))

//#define PACKED __attribute__((packed))
#define PACKED __attribute__ ((aligned(1), packed))


// http://stackoverflow.com/a/15775519/139041
extern std::string methodName(const std::string& prettyFunction);

#define __METHOD_NAME__		methodName(__PRETTY_FUNCTION__)
#define __METHOD_NAME_C__	methodName(__PRETTY_FUNCTION__).c_str()

/// String character count with optional string length out reference parameter.
extern size_t strchrcount(const char *str, char chr, size_t *strlen = NULL);
extern int64_t clock_getnstime(clockid_t clk_id);



//=============================================================================
// Custom data types and classes
//=============================================================================

// color
typedef struct rgb24
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;


	rgb24(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
		: red(red), green(green), blue(blue)
	{}


	// assign
	rgb24& operator=(const rgb24 &rv)
	{
		red		= rv.red;
		green	= rv.green;
		blue	= rv.blue;

		return *this;
	}
	rgb24& operator+(const rgb24 &rv)
	{
		red		+= rv.red;
		green	+= rv.green;
		blue	+= rv.blue;

		return *this;
	}
	rgb24& operator-(const rgb24 &rv)
	{
		red		-= rv.red;
		green	-= rv.green;
		blue	-= rv.blue;

		return *this;
	}

	// compare
	bool operator==(const rgb24 &rv)
	{
		return (red   == rv.red)   &&
			   (green == rv.green) &&
			   (blue  == rv.blue);
	}
	bool operator!=(const rgb24 &rv)
	{
		return !(*this == rv);
	}
} __attribute__ ((aligned(1), packed)) rgb24;


class Point2I
{
public:
	int16_t		x;
	int16_t		y;

	
	Point2I(int16_t _x=0, int16_t _y=0) : x(_x), y(_y) {}

	// assign
	Point2I& operator=(const Point2I &rv)
	{
		x = rv.x;
		y = rv.y;
		return *this;
	}
	Point2I& operator+(const Point2I &rv)
	{
		x += rv.x;
		y += rv.y;
		return *this;
	}
	Point2I& operator-(const Point2I &rv)
	{
		x -= rv.x;
		y -= rv.y;
		return *this;
	}
	
	// compare
	bool operator==(const Point2I &rv)
	{
		return (x == rv.x) &&
			   (y == rv.y);
	}
	bool operator!=(const Point2I &rv)
	{
		return !(*this == rv);
	}
};



#endif // XPMCOMMON_H_
