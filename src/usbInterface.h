#ifndef XPM_USBINTERFACE_H_
#define XPM_USBINTERFACE_H_


//=============================================================================
// libUSB wrapper class
//=============================================================================

#include <libusb-1.0/libusb.h>
#include <vector>

using std::vector;


class XPMUSBInterface
{
public:
	libusb_context			 *mUSB;
	libusb_device			**mList;
	libusb_device			 *mDevice;
	libusb_device_handle	 *mHandle;
	size_t					  mListCount;
	int						  mInterface;
	bool					  mHasInterface;
	bool					  mError;

	vector<libusb_device *>	  mProbed;


	XPMUSBInterface()
	: mUSB(NULL), mList(NULL), mDevice(NULL), mHandle(NULL), mListCount(0), mInterface(0), mHasInterface(false), mError(false)
	{
		int result;

		result = libusb_init(&mUSB);
		if(result)
		{
			printf("%s() error: libusb_init() failed %d\n", __func__, result);
			mError = true;
		}
	}
	~XPMUSBInterface()
	{
		close();
		if(mUSB)
			libusb_exit(mUSB);
	}

	
	size_t probe(uint16_t vendor_id, uint16_t product_id)
	{
		if(!mUSB)
			return 0;

		libusb_device_descriptor	desc;
		ssize_t						count;

		mProbed.clear();
		if(mList)
			libusb_free_device_list(mList, 1);

		count = libusb_get_device_list(mUSB, &mList);
		if(count < 0)
		{
			printf("%s error: libusb_get_device_list() failed %d\n", __METHOD_NAME_C__, count);
			mListCount = 0;
			mError = true;
			return false;
		}
		
		mListCount = (size_t)count;

		for(size_t i=0; i<mListCount; ++i)
		{
			libusb_get_device_descriptor(mList[i], &desc);

			if((desc.idVendor == vendor_id) && (desc.idProduct == product_id))
				mProbed.push_back(mList[i]);
		}

		return mProbed.size();
	}

	bool open(size_t index, int interface = 0)
	{
		if(!mList || (index > mProbed.size()))
			return false;

		int result;
		
		mDevice = mProbed[index];
		result = libusb_open(mDevice, &mHandle);
		if(result)
		{
			printf("%s error: libusb_open() failed %d\n", __METHOD_NAME_C__, result);
			mDevice = NULL;
			return false;
		}

		// shouldn't happen, but make sure a kernel driver hasn't taken a hold of the wanted interface
		if(libusb_kernel_driver_active(mHandle, interface) > 0)
			libusb_detach_kernel_driver(mHandle, interface);

		// claim the bulk interface
		result = libusb_claim_interface(mHandle, interface);
		if(result)
		{
			printf("%s error: libusb_claim_interface() failed %d\n", __METHOD_NAME_C__, result);
			close();
			return false;
		}

		mInterface		= interface;
		mHasInterface	= true;
		mError			= false;
		
		// success
		return true;
	}

	void close()
	{
		if(mHasInterface)	libusb_release_interface(mHandle, mInterface);
		if(mHandle)			libusb_close(mHandle);
		if(mList)			libusb_free_device_list(mList, 1);
		
		mHandle = NULL;
		mDevice = NULL;
		mList	= NULL;	
	}

	size_t read(void *dest, size_t size, unsigned int timeout = 0)
	{
		if(!mHandle)
			return 0;

		int result, tsize;
		result = libusb_bulk_transfer(mHandle, (LIBUSB_ENDPOINT_IN | 1), (uint8_t *)dest, (int)size, &tsize, timeout);
		if(result && result != LIBUSB_ERROR_TIMEOUT)
		{
			printf("%s error: libusb_bulk_transfer() failed %d\n", __METHOD_NAME_C__, result);
			mError = true;
			return 0;
		}

		return (size_t)tsize;
	}

	size_t write(void *src, size_t size)
	{
		if(!mHandle)
			return 0;

		int result, tsize;
		result = libusb_bulk_transfer(mHandle, (LIBUSB_ENDPOINT_OUT | 2), (uint8_t *)src, (int)size, &tsize, 0);
		if(result)
		{
			printf("%s error: libusb_bulk_transfer() failed %d\n", __METHOD_NAME_C__, result);
			mError = true;
			return 0;
		}

		return (size_t)tsize;
	}
};


#endif // XPM_USBINTERFACE_H_
