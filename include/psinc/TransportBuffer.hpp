#pragma once


#include <emergent/Emergent.hpp>
#include <libusb-1.0/libusb.h>
// #include <iostream>

namespace psinc
{
	// A buffer for receiving data at the transport layer. It attempts to
	// allocate DMA memory first to optimise transfer, but falls back to
	// allocating normal memory. Not used at this time.

	class TransportBuffer
	{
		public:

			~TransportBuffer()
			{
				this->Dispose();
			}


			void Dispose()
			{
				if (this->data)
				{
					if (this->deviceAllocated)
					{
						libusb_dev_mem_free(this->handle, this->data, this->max);
					}
					else
					{
						delete [] this->data;
					}
					this->data				= nullptr;
					this->handle			= nullptr;
					this->deviceAllocated	= false;
					this->size = this->max 	= 0;
				}
			}


			// If the device handle has changed then the memory needs
			// re-allocating. If the size is larger than max then the
			// memory needs re-allocating.
			void Resize(libusb_device_handle *handle, const size_t size)
			{
				if (this->handle != handle || size > this->max)
				{
					this->Dispose();

					this->handle	= handle;
					this->max		= size;
					this->data		= libusb_dev_mem_alloc(handle, size);

					if (data)
					{
						// std::cout << "+" << std::flush;
						this->deviceAllocated = true;
					}
					else
					{
						// std::cout << "-" << std::flush;
						// Log::Warning("Failed to allocate device memory");
						this->data = new uint8_t[size];
					}
				}

				this->size = size;
			}

			/// Return the current size of the buffer (not the storage capacity)
			size_t Size() const { return this->size; }

			/// Return the buffer data
			uint8_t *Data() const { return this->data; }


		private:

			// The device associated with the memory allocation
			libusb_device_handle *handle = nullptr;

			// Whether or not memory was successfully allocated for the device
			// otherwise it has been allocated with `new`,
			bool deviceAllocated = false;

			/// The actual buffer
			uint8_t *data = nullptr;

			/// Current size of the buffer (not the storage capacity)
			size_t size = 0;

			/// Storage capacity of the buffer
			size_t max = 0;
	};
}
