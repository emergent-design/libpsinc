using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;


namespace libpsinc
{
	/// Helper singleton for dynamically loading the libusb-1.0 library under Windows. Versions for 32-bit and 64-bit Windows 
	/// are embedded inside this library and the correct one is extracted and loaded automatically.
	internal class Windows
	{
		static readonly Windows instance	= new Windows();
		readonly string name				= string.Format("libusb-1.0-x{0}.dll", IntPtr.Size == 4 ? "86" : "64");
		readonly string path				= Path.Combine(Path.GetTempPath(), "libusb-1.0.dll");
		IntPtr handle						= IntPtr.Zero;
		public bool Loaded 					{ get; private set; }
		public static Windows Instance		{ get { return instance; }}
		
		[DllImport("kernel32")]							internal static extern IntPtr LoadLibrary(string path);
		[DllImport("kernel32", SetLastError = true)]	internal static extern bool FreeLibrary(IntPtr library);
		
		
		Windows()
		{
			using (var library = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
				using (var file = new FileStream(path, FileMode.Create))
			{
				library.CopyTo(file);
			}
			
			this.handle = LoadLibrary(path);
			this.Loaded = this.handle != IntPtr.Zero;
		}
		
		
		~Windows()
		{
			if (this.Loaded)
			{
				FreeLibrary(this.handle);
				FreeLibrary(this.handle);
			}
			
			if (File.Exists(path)) File.Delete(path);
		}
	}
	
	
	/// The imports for libusb v1.0 (only the ones that are needed).
	internal static class Usb
	{
		const string LIB = "libusb-1.0.dll";

		[DllImport(LIB, EntryPoint = "libusb_init")]						internal static extern int Init(ref IntPtr context);		
		[DllImport(LIB, EntryPoint = "libusb_exit")]						internal static extern void Exit(IntPtr context);
		[DllImport(LIB, EntryPoint = "libusb_get_device_list")]				internal static extern int GetDeviceList(IntPtr context, out IntPtr list);
		[DllImport(LIB, EntryPoint = "libusb_free_device_list")]			internal static extern void FreeDeviceList(IntPtr list, int unrefDevices);
		[DllImport(LIB, EntryPoint = "libusb_get_device_descriptor")]		internal static extern int GetDeviceDescriptor(IntPtr device, [Out]Descriptor descriptor);
		[DllImport(LIB, EntryPoint = "libusb_get_bus_number")]				internal static extern byte GetBusNumber(IntPtr device);
		[DllImport(LIB, EntryPoint = "libusb_get_device_address")]			internal static extern byte GetDeviceAddress(IntPtr device);
		[DllImport(LIB, EntryPoint = "libusb_open")]						internal static extern int Open(IntPtr device, ref IntPtr handle);
		[DllImport(LIB, EntryPoint = "libusb_close")]						internal static extern void Close(IntPtr handle);
		[DllImport(LIB, EntryPoint = "libusb_set_configuration")]			internal static extern int SetConfiguration(IntPtr handle, int configuration);
		[DllImport(LIB, EntryPoint = "libusb_claim_interface")]				internal static extern int ClaimInterface(IntPtr handle, int interfaceNumber);
		[DllImport(LIB, EntryPoint = "libusb_release_interface")]			internal static extern int ReleaseInterface(IntPtr handle, int interfaceNumber);
		[DllImport(LIB, EntryPoint = "libusb_reset_device")]				internal static extern int ResetDevice(IntPtr handle);
		[DllImport(LIB, EntryPoint = "libusb_bulk_transfer")]				internal static extern int BulkTransfer(IntPtr handle, byte endpoint, [In][Out]byte [] data, int length, out int transferred, int timeout);
		[DllImport(LIB, EntryPoint = "libusb_get_string_descriptor_ascii")] internal static extern int GetStringDescriptorASCII(IntPtr handle, int index, [Out]byte[] data, int length);
		
		public enum Error
		{
			Success = 0, Io = -1, InvalidParameter = -2, Access = -3, NoDevice = -4, NotFound = -5, Busy = -6,		
			Timeout = -7, Overflow = -8, Pipe = -9, Interrupted = -10, NoMemory = -11, NotSupported = -12, Other = -99
		}
		
		
		[StructLayout(LayoutKind.Sequential, Pack = 0)]
		internal class Descriptor
		{
			public static readonly int Size = Marshal.SizeOf(typeof(Descriptor));
			
			public byte Length;
			public byte DescriptorType;
			public readonly short BcdUsb;
			public readonly byte Class;
			public readonly byte SubClass;
			public readonly byte Protocol;
			public readonly byte MaxPacketSize0;
			public readonly ushort VendorID;
			public readonly ushort ProductID;
			public readonly ushort BcdDevice;
			public readonly byte ManufacturerStringIndex;
			public readonly byte ProductStringIndex;
			public readonly byte SerialStringIndex;
			public readonly byte ConfigurationCount;
		}
	}
}

