using System;
using System.Xml.Linq;
using System.Globalization;


namespace libpsinc
{
	/// <summary>
	/// Control register of the Camera
	/// </summary>
	class Register
	{
		/// <summary>
		/// Hardware address of the register
		/// </summary>
		byte [] address;

		/// <summary>
		/// Address offset
		/// </summary>
		uint offset;

		/// <summary>
		/// Cached value of the register
		/// </summary>
		uint value;

		/// <summary>
		/// Transport layer connecting this register
		/// </summary>
		Transport transport;

		/// <summary>
		/// Gets or sets the hardware page this Register resides on
		/// </summary>
		public byte Page 			{ get; protected set; }

		/// <summary>
		/// Creates a new instance of the <see cref="libpsinc.Register"/> class.
		/// </summary>
		/// <param name="transport">Transport layer connecting this register.</param>
		/// <param name="xml">Description of this Register</param>
		public Register(Transport transport, XElement xml)
		{
			uint address	= uint.Parse(((string)xml.Attribute("address")).Substring(2), NumberStyles.AllowHexSpecifier);
			this.transport	= transport;
			this.address	= new byte [] { (byte)(address & 0xff), (byte)((address >> 8) & 0xff) };
			this.offset		= 2 * (address & 0xff);
			this.Page		= (byte)((address >> 8) & 0xff);
		}

		/// <summary>
		/// Gets the hardware address of this Register
		/// </summary>
		public uint Address
		{
			get
			{
				return  (uint)(this.address[1] << 8) + this.address[0];
			}
		}


		/// <summary>
		/// Gets or sets the value of this register.
		/// Sets are written to the physical hardware.
		/// </summary>
		public uint Value		
		{ 
			get
			{
				return this.value;
			}
			
			set
			{
				this.value = value;
				this.transport.Command(new byte [] { (byte)Commands.WriteRegister, this.address[0], this.address[1], (byte)(value & 0xff), (byte)((value >> 8) & 0xff) }, null);
			}
		}
		
		/// <summary>
		/// Sets a bit in this register.
		/// </summary>
		/// <param name="offset">Offset of the bit to set.</param>
		/// <param name="value">Value to set</param>
		public void SetBit(byte offset, bool value)
		{
			uint mask = (uint)(1 << offset);
			
			if (value) 	this.value |= mask;
			else 		this.value &= ~mask;
			
			this.transport.Command(new byte [] { (byte)Commands.WriteBit, this.address[0], this.address[1], offset, (byte)(value ? 1 : 0) }, null);
		}
		
		/// <summary>
		/// Refresh the local cache of the Register with data from the
		/// physical hardware.
		/// </summary>
		public bool Refresh()
		{
			var receive = new byte[5];

			if (this.transport.CommandFlush(new byte [] { (byte)Commands.QueueRegister, this.address[0], this.address[1] }, receive))
			{
				if (receive.Length == 5 && receive[0] == 0x01)
				{
					this.value = (uint)(receive[4] << 8) + receive[3];
					
					return true;
				}
			}
			
			return false;
		}
		
		/// <summary>
		/// Refresh the cache using data from a page read.
		/// </summary>
		/// <param name="data">Result of a page read from the camera</param>
		public void Refresh(byte [] data)
		{
			if (this.offset < data.Length-1) this.value = (uint)(data[this.offset] << 8) + data[this.offset + 1];
		}
	}
	
}
