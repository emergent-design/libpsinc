using System;
using System.Xml.Linq;
using System.Collections.Generic;

namespace libpsinc
{
	/// <summary>
	/// Feature of the Camera.
	/// We do not expose control registers directly through Acquirer as some registers
	/// control multiple camera features.
	/// Features are individual controllable properties of the camera.
	/// </summary>
	public class Feature
	{
		/// <summary>
		/// Control register the Feature resides in
		/// </summary>
		private Register register;

		/// <summary>
		/// Gets the minimum legal value for the feature
		/// </summary>
		public uint Minimum	{ get; private set; }

		/// <summary>
		/// Gets the maximum legal value for the feature
		/// </summary>
		public uint Maximum	{ get; private set; }

		/// <summary>
		/// Gets the name of the feature
		/// </summary>
		public string Name	{ get; private set; }

		static string [] COMMA	= { "," };

		static string [] HYPHEN	= { "-" };

		/// <summary>
		/// Bit-length of the feature
		/// </summary>
		protected int bits;

		/// <summary>
		/// The offset of the feature in the host register
		/// </summary>
		protected byte offset;

		/// <summary>
		/// Default value of the feature
		/// </summary>
		protected uint defaultValue;

		/// <summary>
		/// True if the feature cannot be modified
		/// </summary>
		protected bool readOnly;

		/// <summary>
		/// True if the feature is a flag
		/// </summary>
		protected bool flag;

		/// <summary>
		/// Bitmask of the feature
		/// </summary>
		protected uint mask					= 0;

		/// <summary>
		/// A collection of invalid values for this feature
		/// </summary>
		protected SortedSet<uint> invalid	= new SortedSet<uint>();

		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.Feature"/> class.
		/// </summary>
		/// <param name="xml">Xml description of the feature.</param>
		/// <param name="register">Host register containing the feature.</param>
		internal Feature(XElement xml, Register register)
		{ 
			this.register = register;
			this.offset			= (byte)(int)(xml.Attribute("offset") ?? new XAttribute("offset", "0"));
			this.readOnly		= (string)(xml.Attribute("readonly") ?? new XAttribute("readonly", "no")) == "yes";
			this.defaultValue	= (uint)xml.Attribute("default");
			this.bits			= (int)xml.Attribute("bits");
			this.flag			= this.bits == 1;
			
			this.Name			= (string)xml.Attribute("name");
			this.Minimum		= (uint)xml.Attribute("min");
			this.Maximum		= this.readOnly ? (uint)((1 << this.bits) - 1) : (uint)xml.Attribute("max");
			
			for (int i=0; i<this.bits; i++) this.mask |= (uint)(1 << (this.offset + i));
			
			this.Invalidate((string)(xml.Attribute("invalid") ?? new XAttribute("invalid", "")));
		}


		/// <summary>
		/// Set the invalid values for this Feature
		/// </summary>
		/// <param name="values">Description of the invalid values. Comma delimited list with hyphenated spanning
		/// (for example 4, 6, 10-20, 30)</param>
		void Invalidate(string values)
		{
			foreach (string range in values.Split(COMMA, StringSplitOptions.RemoveEmptyEntries))
			{
				string [] limits = range.Split(HYPHEN, StringSplitOptions.RemoveEmptyEntries);
				
				switch (limits.Length)
				{
				case 1:		this.invalid.Add(uint.Parse(limits[0]));	
					break;
					
				case 2:		uint end = uint.Parse(limits[1]);
					for (uint i=uint.Parse(limits[0]); i<=end; i++) this.invalid.Add(i);
					break;
					
				default:	throw new Exception(string.Format("Error parsing invalid ranges string '{0}'", values));
				}
			}
		}
		
		/// <summary>
		/// Gets the bit length of the feature
		/// </summary>
		public int Size
		{
			get
			{
				return this.bits;
			}
		}
		
		/// <summary>
		/// Reset this instance.
		/// </summary>
		public uint Reset()
		{
			return this.Value = this.defaultValue;
		}


		/// <summary>
		/// Gets or sets the value of this feature.
		/// </summary>
		public uint Value
		{
			get
			{
				return (this.register.Value & this.mask) >> this.offset;
			}
			
			set
			{
				if (!this.readOnly && value >= this.Minimum && value <= this.Maximum && !this.invalid.Contains(value))
				{
					if (this.flag)	this.register.SetBit(this.offset, value > 0);
					else 			this.register.Value = (this.register.Value & ~this.mask) | ((value << this.offset) & this.mask);
				}
			}
		}
		
		/// <summary>
		/// Refresh this feature by re-reading the host register
		/// from the physical camera.
		/// </summary>
		public bool Refresh()
		{
			return this.register.Refresh();
		}
	}
}
