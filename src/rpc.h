#ifndef XPMRPC_H_
#define XPMRPC_H_


class rgb24;


#pragma pack(1)
struct tRPCPacked
{
  uint8_t    cmd;      // command
  uint8_t   size;      // payload size, else 0xFF/255
};


#define RPCXFER_MASK_SLOT    0x0F
#define RPCXFER_MASK_FLAGS   0xF0
#define RPCXFER_DIR          0x10
#define RPCXFER_APPEND       0x20

struct tRPCXfer
{
  uint8_t   index;     // [bits 7 - 6]: reserved, [bit 5]: append flag, [bit 4]: direction(1=Recv,0=Send), [bits 3 - 0]: buffer slot
  uint8_t   size;      // size in bytes of payload for this command
};

struct tSerialConfig
{
  uint8_t   index;      // [bit 7]: enable bit, [bits 6 - 2]: reserved, [bits 1 & 0]: serial port identifier
  uint32_t  baud;       // baud rate
  uint32_t  format;     // Arduino/Teensy 3.1 defined configuration format
};

struct tSerialXfer
{
  uint8_t   index;      // [bits 7 - 2]: data length, [bits 1 & 0]: serial port identifier
  // rest data..
};

struct tRemoteEvent
{
  uint8_t   command;        // new and current command
  uint8_t   commandOld;     // previous command
  uint32_t  commandTicks;   // previous command lifetime (in units of 200ms)
};

#pragma pack()




// Category of the command
enum class rpcType
{
  System = 0,                    // [ 0] System related
  IO,                            // [ 1] I/O related
  Event,                         // [ 2] Event related
  Display,                       // [ 3] Display related
  Drawing,                       // [ 4] Drawing related
                                 // [ 5 - 14] Unused
  Framebuffer = 15               // [>= 15] Framebuffer related
};

// System commands
enum class rpcSystem
{
  Reset = 0,                     // Force software reset / load bootloader
  Version,                       // Query firmware version
  Ping,                          // Ping? Pong!
  Timestamp,                     // Set current date and time via unix timestamp
};

// Input/Output commands
enum class rpcIO
{
  // Data buffer transfers
  XferRecv,                      // data transfer to LED matrix panel controller (Teensy)
  XferSend,                      // data transfer to LED controller's master (Linux/Host)
  
  // Serial port(s) operations
  SerialPortConfig,              // 
  SerialPortDataSend,            // 
  SerialPortDataRecv,            // 
};

// Event commands
enum class rpcEvent
{
  Packed = 0,                    // Packed event commands
  
  IRRemote,                      // Infrared Remote event
};
static const tRPCPacked m_RPCP_Event[] =
{
  { (uint8_t)rpcEvent::IRRemote,   sizeof(tRemoteEvent) },
  {0, 0} // end of list
};

// Display commands
enum class rpcDisplay
{
  Packed = 0,                    // Packed display commands
  
  Resolution,                    // Query display resolution
  Brightness,                    // Set display brightness
  SwapBuffers,                   // Make drawn framebuffer active
  Mode,                          // Display operation mode
};
static const tRPCPacked m_RPCP_Display[] =
{
  { (uint8_t)rpcDisplay::Resolution,   2 * sizeof(uint16_t) },
  { (uint8_t)rpcDisplay::Brightness,   2 },
  { (uint8_t)rpcDisplay::SwapBuffers,  1 },
  { (uint8_t)rpcDisplay::Mode,         1},
  {0, 0} // end of list
};

enum class rpcDrawing
{
  Packed = 0,                    // Packed drawing commands
  
  // Scrollers
  ScrollSelect,                  // Select a text scroller
  ScrollDefaultSet,              // Save or restore current scroller's default settings
  ScrollEvent,                   // Scroller status callback
  ScrollText,                    // Set text that is to be scrolled
  ScrollState,                   // Scroller current state; ex. start, stop, pause, etc..
  ScrollMode,                    // Scroller mode
  ScrollSpeed,                   // Scroller speed
  ScrollFont,                    // Scroller text font
  ScrollColor,                   // Scroller text color
  ScrollOffsetHorizontal,        // Scroller horizontal offset position
  ScrollOffsetVertical,          // Scroller vertical offset position
  ScrollBoundary,                // Scroller rectangle bounds
  
  DrawPixel = 20,
  DrawLine,
  DrawFastVLine,
  DrawFastHLine,
  DrawCircle,
  FillCircle,
  DrawEllipse,
  DrawTriangle,
  FillTriangle,
  DrawRectangle,
  FillRectangle,
  DrawRoundRectangle,
  FillRoundRectangle,
  FillScreen,
  SetFont,
  DrawChar,
  DrawString,
  DrawMonoBitmap,
  GIFAnimation,

};
static const tRPCPacked m_RPCP_Drawing[] =
{
  { (uint8_t)rpcDrawing::ScrollSelect,        1 },
  { (uint8_t)rpcDrawing::ScrollDefaultSet,    1 },
  { (uint8_t)rpcDrawing::ScrollEvent,         2 + sizeof(uint32_t) },
  { (uint8_t)rpcDrawing::ScrollText,          1 },
  { (uint8_t)rpcDrawing::ScrollState,         2 },
  { (uint8_t)rpcDrawing::ScrollMode,          1 },
  { (uint8_t)rpcDrawing::ScrollSpeed,         1 },
  { (uint8_t)rpcDrawing::ScrollFont,          1 },
  { (uint8_t)rpcDrawing::ScrollColor,         sizeof(rgb24) },
  { (uint8_t)rpcDrawing::ScrollOffsetHorizontal, 2 },
  { (uint8_t)rpcDrawing::ScrollOffsetVertical,   2 },
  { (uint8_t)rpcDrawing::ScrollBoundary,      sizeof(int16_t)*4 },

  { (uint8_t)rpcDrawing::DrawPixel,           sizeof(int16_t)*2 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawLine,            sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawFastVLine,       sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawFastHLine,       sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawCircle,          sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillCircle,          sizeof(int16_t)*3 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawEllipse,         sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawTriangle,        sizeof(int16_t)*6 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillTriangle,        sizeof(int16_t)*6 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawRectangle,       sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillRectangle,       sizeof(int16_t)*4 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawRoundRectangle,  sizeof(int16_t)*5 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillRoundRectangle,  sizeof(int16_t)*5 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::FillScreen,          sizeof(rgb24) },
  { (uint8_t)rpcDrawing::SetFont,             1 },
  { (uint8_t)rpcDrawing::DrawChar,            sizeof(int16_t)*2 + sizeof(rgb24) + 1 },
  { (uint8_t)rpcDrawing::DrawString,          sizeof(int16_t)*2 + sizeof(rgb24)*2 + 1 },
  { (uint8_t)rpcDrawing::DrawMonoBitmap,      sizeof(int16_t)*2 + 1 },
  { (uint8_t)rpcDrawing::GIFAnimation,        2 + sizeof(int16_t)*2 },

  {0, 0} // end of list
};

enum rpcScrollState
{
  ScrollState_Stop = 0,
  ScrollState_Start,
  ScrollState_Pause,
  ScrollState_Append
};

// Drawing -> GIFAnimation
#define RPCGIF_FLAG_LOAD      0x80    // Load GIF from filesystem flag
#define RPCGIF_FLAG_POS       0x40    // Set GIFPlayer position flag
#define RPCGIF_FLAG_INTERVAL  0x20    // Set GIFPlayer animation interval flag
#define RPCGIF_MASK_FLAGS     0xF0    // Bitmask for flags
#define RPCGIF_MASK_STATE     0x03    // Bitmask for GIFPlayer state

enum class rpcGIFState
{
  Stop = 0,                       // No GIF image displayed
  Play,                           // Animate GIF
  Pause,                          // Freeze frame of GIF animation
};



#define RPCDATA_SIZE  64                          // storage buffer size in bytes
#define RPCC_SIZE     2                           // command size in bytes
#define RPCPL_SIZE    (RPCDATA_SIZE - RPCC_SIZE)  // payload size in bytes



// Remote Procedure Call class
class XpmRPC
{
private:
	uint8_t		rpcDataTX[RPCDATA_SIZE];
	uint8_t		rpcDataRX[RPCDATA_SIZE];
	bool		mBatch;
	bool		mOK;


	void onSystem	(rpcSystem	cmd, uint8_t *data, size_t size);
	void onIO		(rpcIO		cmd, uint8_t *data, size_t size);
	void onEvent	(rpcEvent	cmd, uint8_t *data, size_t size);
	void onDisplay	(rpcDisplay	cmd, uint8_t *data, size_t size);
//	void onDrawing	(rpcDrawing	cmd, uint8_t *data, size_t size);  matrix handles it self now
	int  batchFlush	();

	
public:
	XpmRPC();
	~XpmRPC();


	bool prepare();
	
	bool ok()		{ return mOK; }

	bool send(rpcType type, uint8_t cmd, const uint8_t *data, size_t size, bool clean = false, const uint8_t *data2 = NULL, size_t size2 = 0);
	template<typename T1>
	  inline bool send(rpcType type, T1 cmd, const uint8_t *data, size_t size, bool clean = false, const uint8_t *data2 = NULL, size_t size2 = 0)
	  { return send(type, (uint8_t)cmd, (const uint8_t *)data, size, clean, (const uint8_t *)data2, size2); }
	template<typename T1, typename T2>
	  inline bool send(rpcType type, T1 cmd, const T2 *data, size_t size, bool clean = false, const uint8_t *data2 = NULL, size_t size2 = 0)
	  { return send(type, (uint8_t)cmd, (const uint8_t *)data, size, clean, (const uint8_t *)data2, size2); }
	template<typename T1, typename T2, typename T3>
	  inline bool send(rpcType type, T1 cmd, const T2 *data, size_t size, bool clean = false, const T3 *data2 = NULL, size_t size2 = 0)
	  { return send(type, (uint8_t)cmd, (const uint8_t *)data, size, clean, (const uint8_t *)data2, size2); }

	bool sendTypeFrame(uint8_t flags, const uint8_t *data, size_t size);

	bool transfer(uint8_t slot, const uint8_t *src, size_t size);

	int  poll(unsigned int timeout = 50);

	// command batching
	void batchBegin();
	int  batchEnd();

	// system procedures
	void resetClient();
	void setTime(time_t timestamp);
};

extern XpmRPC rpc;


#endif // XPMRPC_H_
