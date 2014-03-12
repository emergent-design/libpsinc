using System;
using System.Collections.Generic;


namespace libpsinc
{
	/// <summary>Available commands</summary>
	internal enum Commands : byte
	{
		/// <summary>Perform an image capture.</summary>
		Capture				= 0x00,

		/// <summary>Perform image capture as a slave device on the rising edge of the sync signal</summary>
		SlaveCaptureRising	= 0x01,

		/// <summary>Reset the imaging chip</summary>
		ResetChip			= 0x02,

		/// <summary>Flush the device; return responses to all commands queued in this and previous un-flushed commands.</summary>
		Flush				= 0x03,

		/// <summary>Capture an image and signal all waiting slaves to perform a capture.</summary>
		MasterCapture		= 0x04,

		/// <summary>Perform image capture as a slave device on the falling edge of the sync signal</summary>
		SlaveCaptureFalling	= 0x05,

		/// <summary>Write a register</summary>
		WriteRegister		= 0x10,

		/// <summary>Write a bit</summary>
		WriteBit			= 0x11,

		/// <summary>Queue a register read</summary>
		QueueRegister		= 0x12,

		/// <summary>Read a page of registers</summary>
		ReadRegisterPage	= 0x13,

		/// <summary>Write to a sub-device</summary>
		WriteDevice			= 0x20,

		/// <summary>Queue a sub-device read.</summary>
		QueueDevice			= 0x21,

		/// <summary>Initialise a sub-device.</summary>
		InitialiseDevice	= 0x22,

		/// <summary>Block write to a sub-device.</summary>
		WriteDeviceBlock	= 0x23,
	}


	/// <summary>Available capture modes, enum to raw instruction dictionary</summary>
	internal class CaptureCommands
	{
		public static readonly Dictionary<CaptureMode, byte> Map = new Dictionary<CaptureMode, byte> {
			{ CaptureMode.Normal,		(byte)Commands.Capture },
			{ CaptureMode.Master,		(byte)Commands.MasterCapture },
			{ CaptureMode.SlaveRising,	(byte)Commands.SlaveCaptureRising },
			{ CaptureMode.SlaveFalling,	(byte)Commands.SlaveCaptureFalling }
		};
	}
}
