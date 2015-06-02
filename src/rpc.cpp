#include <xpmcommon.h>
#include "rpc.h"
#include "matrix.h"
#include "usbInterface.h"


XPMUSBInterface		mDevice;



//=============================================================================
// LED Matrix RPC wrapper class
//=============================================================================

XpmRPC::XpmRPC()
:	mBatch(false),
	mOK(true)
{
	memset(rpcDataTX, 0, sizeof(rpcDataTX));
	memset(rpcDataRX, 0, sizeof(rpcDataRX));
}
XpmRPC::~XpmRPC()
{
}


bool XpmRPC::prepare()
{
	// scan for all matching USB devices and open the first one found
	if(!mDevice.probe(0x1d50, 0x607a) || !mDevice.open(0))
		return false;

	// request version
	if(!send(rpcType::System, rpcSystem::Version, NULL, 0))
		return false;

	// succcess
	return true;
}


//-----------------------------------------------------------------------------
// I/O functions
//-----------------------------------------------------------------------------

bool XpmRPC::send(rpcType type, uint8_t cmd, const uint8_t *data, size_t size, bool clean, const uint8_t *data2, size_t size2)
{
	// reject over max payload size
	if((size + size2) > RPCPL_SIZE)
		return false;

	// set command bytes
	rpcDataTX[0] = (uint8_t)type;
	rpcDataTX[1] = (uint8_t)cmd;

	// store passed data to transmit buffer and
	// zero-out remaining of packet if requested
	if(size)
		memcpy(&rpcDataTX[RPCC_SIZE], data, size);
	if(size2)
		memcpy(&rpcDataTX[RPCC_SIZE + size], data2, size2);
	if(clean && ((size + size2) < RPCPL_SIZE))
		memset(&rpcDataTX[RPCC_SIZE + (size + size2)], 0, RPCPL_SIZE - (size + size2));

	if(!mDevice.write(rpcDataTX, sizeof(rpcDataTX)))
	{
//		printf("device I/O error: (%d) %s\n", errno, strerror(errno));
		mOK = false;
		return false;
	}

	// success
	return true;
}

bool XpmRPC::sendTypeFrame(uint8_t flags, const uint8_t *data, size_t size)
{
	// reject over max payload size
	if(size > (RPCDATA_SIZE -1))
		return false;

	// set command byte
	rpcDataTX[0] = (uint8_t)rpcType::Framebuffer | flags;

	// store passed data to transmit buffer
	if(size)
		memcpy(&rpcDataTX[1], data, size);

	if(!mDevice.write(rpcDataTX, sizeof(rpcDataTX)))
	{
//		printf("device I/O error: (%d) %s\n", errno, strerror(errno));
		mOK = false;
		return false;
	}

	// success
	return true;
}

bool XpmRPC::transfer(uint8_t slot, const uint8_t *src, size_t size)
{
	tRPCXfer xfer;

	if(slot >= IOBUFFERS_COUNT)
		return false;

	xfer.index = slot & RPCXFER_MASK_SLOT;
	
	// lookup rules for specified slot
	tIOBuffer *buffer = &gm_IOBuffers[slot];

	// clamp too large xfer size request
	if(size > (size_t)buffer->size)
		size = (size_t)buffer->size;

	// breakup data into packets
	while(size)
	{
		if(size > (size_t)(RPCPL_SIZE - sizeof(xfer)))
			xfer.size = (uint8_t)(RPCPL_SIZE - sizeof(xfer));
		else
			xfer.size = (uint8_t)size;

		if(!send(rpcType::IO, rpcIO::XferRecv, &xfer, sizeof(xfer), false, src, xfer.size))
			return false;

		src  += xfer.size;
		size -= xfer.size;
		xfer.index |= RPCXFER_APPEND;
	}
	
	return true;
}

int XpmRPC::poll(unsigned int timeout)
{
	int proccount = 0;

	
	for(;;)
	{
		if(!mDevice.read(rpcDataRX, sizeof(rpcDataRX), timeout))
			break;

		proccount++;
		
		// first byte is always command
		rpcType  type  = (rpcType)rpcDataRX[0];
		uint8_t  *data = &rpcDataRX[1];

		// handle base command or type identifier
		switch(type)
		{
			case rpcType::System:	onSystem( (rpcSystem)data[0],  data +1, RPCPL_SIZE); break;
			case rpcType::IO:		onIO(     (rpcIO)data[0],      data +1, RPCPL_SIZE); break;
			case rpcType::Event:	onEvent(  (rpcEvent)data[0],   data +1, RPCPL_SIZE); break;
			case rpcType::Display:	onDisplay((rpcDisplay)data[0], data +1, RPCPL_SIZE); break;
//			case rpcType::Drawing:	onDrawing((rpcDrawing)data[0], data +1, RPCPL_SIZE); break;
			case rpcType::Drawing:	matrix.handleRPCDrawing((rpcDrawing)data[0], data +1, RPCPL_SIZE); break;
			default:
				break;
		}
	}

	if(mDevice.mError)
		mOK = false;

	return proccount;
}

void XpmRPC::onSystem(rpcSystem cmd, uint8_t *data, size_t size)
{
	switch(cmd)
	{
		case rpcSystem::Version:
		{
			printf("Version reply: %s\n", data);
			break;
		}
		case rpcSystem::Ping:
		{
			// do something with response
			break;
		}
		default:
			break;
	}
}

void XpmRPC::onIO(rpcIO cmd, uint8_t *data, size_t size)
{
	switch(cmd)
	{
		case rpcIO::XferSend:
		{
			// data transfer to LED controller's master (Linux/Host)
			
			break;
		}
		case rpcIO::SerialPortDataRecv:
		{
			// received data on a serial port
			break;
		}
		default:
			break;
	}
}

void XpmRPC::onEvent(rpcEvent cmd, uint8_t *data, size_t size)
{
	switch(cmd)
	{
		case rpcEvent::Packed:
		{
			// Packed event commands
			break;
		}
		case rpcEvent::IRRemote:
		{
			tRemoteEvent event;
			memcpy(&event, data, sizeof(event));
			printf("Button pressed: %s, released: %s (held %u ms)\n",
					commandToString((InputCommand)event.command),
					commandToString((InputCommand)event.commandOld),
					(event.commandTicks +1) * IR_RECV_POLLRATE);
			break;
		}
		default:
			break;
	}
}

void XpmRPC::onDisplay(rpcDisplay cmd, uint8_t *data, size_t size)
{
	switch(cmd)
	{
		case rpcDisplay::Packed:
		{
			// Packed event commands
			break;
		}
		case rpcDisplay::Resolution:
		{
			int16_t dims[2]; // dimensions

			memcpy(dims, data, sizeof(dims));
			matrix.width  = dims[0];
			matrix.height = dims[1];
			
			printf("Display resolution %ux%u\n", dims[0], dims[1]);
			break;
		}
		case rpcDisplay::SwapBuffers:
		{
			matrix.displaySwapped();
			break;
		}
		default:
			break;
	}
}



// command batching place holders
void XpmRPC::batchBegin()
{
	if(mBatch)
		return;
	
	mBatch = true;
}

int XpmRPC::batchEnd()
{
	if(!mBatch)
		return 0;

	mBatch = false;
	return 0;
}

int XpmRPC::batchFlush()
{
	return 0;
}



//-----------------------------------------------------------------------------
// System functions
//-----------------------------------------------------------------------------

void XpmRPC::resetClient()
{
	send(rpcType::System, rpcSystem::Reset, NULL, 0);
}

void XpmRPC::setTime(time_t timestamp)
{
	uint64_t ts = timestamp; // 64bit value for future proofing

	send(rpcType::System, rpcSystem::Timestamp, &ts, sizeof(ts));
}

