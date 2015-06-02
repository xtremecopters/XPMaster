#include <xpmcommon.h>
#include <signal.h>
#include <getopt.h>
#include "rpc.h"
#include "matrix.h"
#include "scripting/scripting.h"


// for reference to Teensy side's avaialble xfer buffers
tIOBuffer gm_IOBuffers[IOBUFFERS_COUNT] =
{
  // small buffers
  { IOBUFFERS_SSIZE, 0, NULL },
  { IOBUFFERS_SSIZE, 0, NULL },
  { IOBUFFERS_SSIZE, 0, NULL },
  { IOBUFFERS_SSIZE, 0, NULL },

  // large buffers
  { IOBUFFERS_LSIZE, 0, NULL },
  { IOBUFFERS_LSIZE, 0, NULL },
  { IOBUFFERS_LSIZE, 0, NULL },
  { IOBUFFERS_LSIZE, 0, NULL },
};


LEDMatrix	matrix;		// LED matrix instance
XpmRPC		rpc;		// Remote Procedure Call instance
ScriptCore	scripting;	// Scripting instance, currently for Python support

volatile bool gm_Exit = false;

static void signalHandler(int sig);
static double getFramerate();

// command line options
static struct option long_options[] =
{
	// flags
//	{ "verbose",	no_argument,	&verbose_flag, 1 },
//	{ "brief",		no_argument,	&verbose_flag, 0 },

	// operations
	{ "help",		no_argument,		0, 'h' },	// print help information
	{ "file",		required_argument,	0, 'f' },	// script file to run instead of default

	// end of options
	{ 0, 0, 0, 0 }
};


static void example_DirectFBWrite();

int main(int argc, char * const argv[])
{
	// register interrupt signal handler
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = signalHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	
	// process command line options
	int optionIndex = 0;
	
	for(;;)
	{
		int chr = getopt_long(argc, argv, "hf:", long_options, &optionIndex);

		// check for end of options reached
		if(chr == -1)
			break;

		// handle the option
		switch(chr)
		{
			case 'h':
			case '?':
			{
				// print help information
				printf("Read the user manual for help.\n");
				return 0;
			}
			
			case 'f':
			{
				// script file to run instead of default
				scripting.file = optarg;
				break;
			}
		}
	}

	
	// initialize RPC protocol and USB connection
	if(!rpc.prepare())
	{
		printf("Display panel is not connected.\n");
		return -1;
	}

	// initialize matrix
	if(!matrix.prepare())
	{
		printf("Display panel communication failure.\n");
		return -2;
	}

	// synchronize local machine time with Teensy
	rpc.setTime(getLocalTimestamp());
//	matrix.setMode(DisplayState_DateTime);


	// API usage example
	matrix.setMode(DisplayState_Manual);
/*	matrix.gifLoad("gifs/sunset.gif");
	matrix.gifPosition(matrix.width / 2 - 16, 0);
	matrix.gifPlay();
*/	matrix.waitForVSync();

#if 0
	// testing and debugging continous text scrolling feature
	TextScroller &scroller = matrix.getScroller(3);
	scroller.stopScrollText();
	scroller.setScrollColor(rgb24(0,255,0));
	scroller.setScrollSpeed(160);
	scroller.setScrollFont(font8x13);
	scroller.setScrollMode(wrapForward /* or wrapForwardFromLeft */); // only two modes are supported for continuous scroll texting.
	scroller.setScrollBoundary(32, 0, matrix.width -1, matrix.height - 1);
	scroller.setScrollOffsetFromTop( matrix.height - matrix.getFontStringDims(font8x13, "I").y );
	scroller.appendText("Hello world! How's it going up there? :) ");
	scroller.appendText("This is a short message. ");
	scroller.appendText("This long message could possibly go on forever and on and on and on, but it won't. ");
	scroller.appendText("Now some random IRC chat messages: ");
	scroller.appendText("<ran> That stuff is all cool, but how do you update the resolution of an issue in JIRA using their REST API?  THAT is the real mystery... ");
	scroller.appendText("<Xi> ran: the api is only there when it isn't observed j/k ;) ");
	scroller.appendText("<ran> Oh, no, you're absolutely right ");
	scroller.appendText("<mac> schrodingers API ");
	scroller.appendText("<Jay> heh ");
	scroller.appendText("<ran> You throw text at it and close your eyes... hopefully it will do what you want... ");
	scroller.appendText("<Ht> thats stuff has been a while for me ");
	scroller.appendText("<ran> Been a while for you?!?  It's been over 30 years for me... ");
	scroller.appendText("<Ht> now I just use software to do quantum calculations ... a real black box lol ");
	scroller.appendText("<Jay> so yeah, 2 bugs to correct, but otherwise zooming for the nodegraph is in ");
	scroller.appendText("<Tn> :P ");

//	scroller.appendText("");

	while(rpc.ok())
		rpc.poll(200);
#endif // 0

#if 1 // scripting

	// initialize scripting subsystem
	if(scripting.prepare())
	{
		// have scripting take over from here
		scripting.start();
	}

#else // scripting
#if 1 // direct framebuffer write C++ example

	example_DirectFBWrite();

#else // other C++ examples
	int16_t x, y;
	uint8_t bn=255, bstep = 256 / matrix.height;
	
	
	matrix.fillScreen({0,0,64});
	for(y=matrix.height -1; y>-1; y--)
	{
		matrix.drawLine(0, y, matrix.width, y, {bn,bn,bn});
		bn -= bstep;
	}
	matrix.setFont(gohufont11b);
	matrix.drawString(0, 0, rgb24(0xff,0x93,0x26), rgb24(0x21,0x4d,0x88), "Hello World!");
	matrix.setScrollColor(rgb24(255,255,0));
	matrix.scrollText("This will only scroll five times.", 5);
	matrix.swapBuffers();


	uint8_t wheelPos=128;
	rgb24 color;

	// continous draw loop, until RPC I/O error occurs
	while(matrix.waitForVSync())
	{
		double framerate = getFramerate();

		wheelPos += 6;
		uint8_t cw = wheelPos;

		x = matrix.width  / 2;
		y = matrix.height / 2;
#if 1
		for(; x<matrix.width; x++)
		{
			cw += 4;
			colorWheel(color, cw);
			matrix.drawFastVLine(x, y, matrix.height -1, color);
		}
#else
		matrix.fillRoundRectangle(x, y, matrix.width -1, matrix.height -1, 3, rgb24(0xae,0x10,0x53), rgb24((255,255,255));
#endif

		// display frame rate
		static char text[32];
		sprintf(text, "FPS %2.2f", framerate);
		matrix.setFont(font5x7);
		matrix.drawString((matrix.width / 2) +4, matrix.height - 12, rgb24(0,0,0), text);
	}
#endif // 0
#endif // other C++ examples
	
	return 0;
}

static void signalHandler(int sig)
{
	static int counter=0;
	
	if(sig == SIGINT)
	{
		gm_Exit = true;
		if(!counter)
			scripting.stop();
		
		if(++counter > 5)
			exit(2);
	}
}


/*
 * Retrieve a timestamp value that's suited for Teensy side to use as a time clock sync value.
 */
time_t getLocalTimestamp()
{
	time_t ts = time(NULL);
	struct tm * tinfo = localtime(&ts);
	tzset();
	// timezone comes from time.h which is set by tzset().
	if(tinfo->tm_isdst > 0)
		timezone -= (60 * 60);
	return ts - timezone;
}

int64_t clock_getnstime(clockid_t clk_id)
{
	struct timespec ts;

	clock_gettime(clk_id, &ts);
	return ((int64_t)ts.tv_sec * 1000000000LL) + (int64_t)ts.tv_nsec;
}

bool getFramerate(double &fps)
{
	static int64_t	lastTime	= clock_getnstime(CLOCK_MONOTONIC);
	static int64_t	calls		= 0;
	static double	rate		= 0.0;
	int64_t			now			= clock_getnstime(CLOCK_MONOTONIC);
	int64_t			diff		= now - lastTime;
	

	++calls;
	
	if(diff >= 1000000000LL)
	{
		rate	 = (double)calls * (diff / 1000000000.0);
		calls	 = 0;
		lastTime = now;
		fps		 = rate;
		
		return true;
	}

	fps = rate;
	return false;
}
double getFramerate()
{
	double rate;
	getFramerate(rate);
	return rate;
}

const char* commandToString(InputCommand cmd)
{
    switch(cmd)
    {
        case InputCommand::None:      return "None";
        case InputCommand::Same:      return "<Same>";
        case InputCommand::BrightDec: return "BrightDec";
        case InputCommand::BrightInc: return "BrightInc";
        case InputCommand::PlayPause: return "PlayPause";
        case InputCommand::Setup:     return "Setup";
        case InputCommand::StopMode:  return "StopMode";
        case InputCommand::Up:        return "Up";
        case InputCommand::Down:      return "Down";
        case InputCommand::Left:      return "Left";
        case InputCommand::Right:     return "Right";
        case InputCommand::Select:    return "Select";
        case InputCommand::ZeroTen:   return "ZeroTen";
        case InputCommand::Back:      return "Back";
        case InputCommand::Num1:      return "Num1";
        case InputCommand::Num2:      return "Num2";
        case InputCommand::Num3:      return "Num3";
        case InputCommand::Num4:      return "Num4";
        case InputCommand::Num5:      return "Num5";
        case InputCommand::Num6:      return "Num6";
        case InputCommand::Num7:      return "Num7";
        case InputCommand::Num8:      return "Num8";
        case InputCommand::Num9:      return "Num9";
        default:                      return "<Unknown>";
    }
}

std::string methodName(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin,end) + "()";
}

/**
 * Get number of times chr is seen in str character string.
 * 
 * @param[in]	str		Source character string to search through.
 * @param		chr		Character to compare with.
 * @param[out]	strlen	When non-NULL is dereferenced to store string length into.	
 */
size_t strchrcount(const char *str, char chr, size_t *strlen)
{
	size_t count = 0;
	size_t len   = 0;


	// abort on NULL pointer
	if(!str)
		return 0;

	// search string until NULL terminator
	while(*str)
	{
		if(*str == chr)
			++count;
		
		// next
		++str;
		++len;
	}

	// optional string length output parameter
	if(strlen)
		*strlen = len;

	return count;
}


/**
 * Direct framebuffer write example, instead of using Matrix draw commands.
 * XtremePanel equivalent to fadecandy's.
 */
static void example_DirectFBWrite()
{
	// new framebuffer has been displayed
	rgb24	color;
	int16_t	i;
	static	uint8_t wheelPos=128;
//	colorWheel(color, ++wheelPos);

	const size_t FBsize   = matrix.width * matrix.height;
	const size_t TXStride = RPCDATA_SIZE -1;
		  size_t FBpacks  = (FBsize*3) / TXStride;

	if((FBsize*3) % TXStride)
		++FBpacks;

	// allocate local framebuffer
	rgb24 *framebuffer = new rgb24[FBpacks * (TXStride / 3)];


	// stop GIF playback, stop text scrollers, etc..
	matrix.gifStop();
	matrix.getScroller(0).stopScrollText();
	matrix.getScroller(1).stopScrollText();
	matrix.fillScreen(rgb24(0, 0, 0));
	matrix.waitForVSync();


	// main loop
	while(!gm_Exit && rpc.ok())
	{
		double fps;
		if(getFramerate(fps))
		{
			// report current frame rate every second
			printf("\rFrame update rate %0.3f", fps);
			fflush(stdout);
		}
		
#if 1
		// colorwheel in smooth moving columns
		wheelPos += 8;
		uint8_t cw = wheelPos;
		for(int16_t x=0; x<matrix.width; x++)
		{
			cw += 4;
			colorWheel(color, cw);
			for(int16_t y=0; y<matrix.height; y++)
			{
				framebuffer[(matrix.width * y) + x] = color;
			}
		}
#else
		colorWheel(color, ++wheelPos);
		// fill entire display with colorwheel value
		for(int16_t x=0; i<FBsize; i++)
			framebuffer[x] = color;
#endif

		// make border outline white
		color = {255, 255, 255};
		for(i=0; i<matrix.width; i++)
		{
			// top and bottom rows
			framebuffer[i] = color;
			framebuffer[matrix.width * (matrix.height -1) +i] = color;
		}
		for(i=0; i<matrix.height; i++)
		{
			// left and right columns
			framebuffer[(matrix.width * i)] = color;
			framebuffer[(matrix.width * i) + (matrix.width -1)] = color;
		}

		uint8_t *data = (uint8_t *)framebuffer;

		// transfer framebuffer
		for(size_t i=0; i<FBpacks; i++)
		{
			uint8_t flags = 0;
			if(i > 0)
				flags |= 0x10;	// append flag
			if(i+1 == FBpacks)
				flags |= 0x20;	// swapBuffers flag
//			flags |= 0x80;		// raster debug visual aid enable flag, unique colors per raster segment(21 pixels) draw
			rpc.sendTypeFrame(flags, &data[i * TXStride], TXStride);
		}

		// frame swap detection (vertical synchronization), different from matrix.waitForVSync() cause of direct framebuffer writing done above
		size_t fbswaps = matrix.bufferswaps;
		while(!gm_Exit && rpc.poll(5) && rpc.ok() && (fbswaps == matrix.bufferswaps));
	}

	delete[] framebuffer;
}

