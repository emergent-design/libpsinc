using System;
using System.Linq;
using System.Collections.Generic;


namespace libpsinc
{
	public class AliasCollection
	{
		/// <summary>
		/// The features available through this AliasCollection
		/// </summary>
		internal SortedDictionary<string, Feature> features				= null;

		/// <summary>
		/// The contextual alias-to-feature lookup
		/// </summary>
		internal Dictionary<byte, Dictionary<string, string>> aliases 	= new Dictionary<byte, Dictionary<string, string>>();

		/// <summary>
		/// The current camera context
		/// </summary>
		internal byte context											= 0;

		/// <summary>
		/// Gets the generic Gain feature of the camera.
		/// </summary>
		/// <value>The Gain feature.</value>
		public Feature Gain 			{ get { return this["Gain"]; }}

		/// <summary>
		/// Gets the generic Exposure feature of the camera.
		/// </summary>
		/// <value>The Exposure feature.</value>
		public Feature Exposure 		{ get { return this["Exposure"]; }}

		/// <summary>
		/// Gets the generic Automatic Gain Control Sensitivity feature of the camera.
		/// </summary>
		/// <value>The Automatic Gain Control Sensitivity feature.</value>
		public Feature AgcSensitivity	{ get { return this["AGCSensitivity"]; }}

		/// <summary>
		/// Gets the generic Analogue to Digital Converter Reference Voltage feature of the camera.
		/// </summary>
		/// <value>The Analogue to Digital Converter Reference Voltage feature.</value>
		public Feature AdcReference		{ get { return this["ADCReference"]; }}

		/// <summary>
		/// Gets the generic Automatic Gain Enable feature of the camera.
		/// </summary>
		/// <value>The Automatic Gain Enable feature.</value>
		public Feature AutoGain			{ get { return this["AutoGain"]; }}

		/// <summary>
		/// Gets the generic Automatic Exposure Enable feature of the camera.
		/// </summary>
		/// <value>The Automatic Exposure Enable feature.</value>
		public Feature AutoExposure		{ get { return this["AutoExposure"]; }}

		/// <summary>
		/// Gets the generic Companding Enable feature of the camera.
		/// </summary>
		/// <value>The Companding Enable feature.</value>
		public Feature Companding		{ get { return this["Companding"]; }}


		/// <summary>
		/// List of the generic feature alias names
		/// </summary>
		static readonly string [] GENERIC = { "Context", "Gain", "Exposure", "AGCSensitivity", "ADCReference", "AutoGain", "AutoExposure", "Companding" };
		
		/// <summary>
		/// Gets the Feature with the specified alias.
		/// </summary>
		/// <param name="name">Name of the feature to get or null if no such feature exists</param>
		public Feature this[string name]
		{
			get 
			{
				return this.aliases[this.context].ContainsKey(name) ? this.features[this.aliases[this.context][name]] : null;
			}
		}
		
		
		/// <summary>
		/// Determines whether the specified feature has a generic alias.
		/// </summary>
		/// <returns><c>true</c> if the feature has a generic alias; otherwise, <c>false</c>.</returns>
		/// <param name="feature">Feature to check.</param>
		public bool IsGeneric(Feature feature)
		{
			var f = this.aliases[this.context].FirstOrDefault(a => a.Value == feature.Name);
			
			return GENERIC.Contains(f.Key);
		}
	}
}
