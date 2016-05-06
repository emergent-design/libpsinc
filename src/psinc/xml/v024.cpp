namespace psinc { namespace chip { const char *v024 = R"xml(
<?xml version="1.0" encoding="utf-8"?>
<camera chip="v024" bits="16" manufacturer="PSI" author="dan" contexts="2" addressSize="2" bayer="3">
	<alias context="0" name="Width" feature="A: Window Width" />
	<alias context="0" name="Height" feature="A: Window Height" />
	<alias context="0" name="Gain" feature="A: Global Analog Gain" />
	<alias context="0" name="Exposure" feature="A: Coarse Shutter Width Total" />
	<alias context="0" name="AutoGain" feature="A: AGC Enable" />
	<alias context="0" name="AutoExposure" feature="A: AEC Enable" />
	<alias context="0" name="Companding" feature="A: ADC Mode" />
	<alias context="0" name="NoiseCorrection" feature="A: Enable Noise Correction" />
	<alias context="0" name="ColumnStart" feature="A: Column Start" />
	<alias context="0" name="RowStart" feature="A: Row Start" />

	<alias context="1" name="Width" feature="B: Window Width" />
	<alias context="1" name="Height" feature="B: Window Height" />
	<alias context="1" name="Gain" feature="B: Global Analog Gain" />
	<alias context="1" name="Exposure" feature="B: Coarse Shutter Width Total" />
	<alias context="1" name="AutoGain" feature="B: AGC Enable" />
	<alias context="1" name="AutoExposure" feature="B: AEC Enable" />
	<alias context="1" name="Companding" feature="B: ADC Mode" />
	<alias context="1" name="NoiseCorrection" feature="B: Enable Noise Correction" />
	<alias context="1" name="ColumnStart" feature="B: Column Start" />
	<alias context="1" name="RowStart" feature="B: Row Start" />

	<alias name="AGCSensitivity" feature="AEC/AGC Pixel Count" />
	<alias name="ADCReference" feature="VREF_ADC Voltage Level" />
	<alias name="Context" feature="Context A/B Select" />

	<register address="0x01">
		<feature name="A: Column Start" bits="10" min="1" max="752" default="1" />
	</register>
	<register address="0x02">
		<feature name="A: Row Start" bits="9" min="4" max="482" default="4" />
	</register>
	<register address="0x03">
		<feature name="A: Window Height" bits="9" min="1" max="480" default="480" />
	</register>
	<register address="0x04">
		<feature name="A: Window Width" bits="10" min="1" max="752" default="752" />
	</register>
	<register address="0x05">
		<feature name="A: Horizontal Blanking" bits="10" min="61" max="1023" default="94" />
	</register>
	<register address="0x06">
		<feature name="A: Vertical Blanking" bits="15" min="2" max="32288" default="45" />
	</register>
	<register address="0x07">
		<feature name="Scan Mode" bits="3" min="0" max="3" default="0" invalid="1" />
		<feature name="Sensor Operating Mode" bits="2" offset="3" min="0" max="3" default="1" invalid="2" />
		<feature name="Stereoscopy Mode" bits="1" offset="5" min="0" max="1" default="0" />
		<feature name="Stereoscopic Master/Slave Mode" bits="1" offset="6" min="0" max="1" default="0" />
		<feature name="Parallel Output Enable" bits="1" offset="7" min="0" max="1" default="1" />
		<feature name="Simultaneous/Sequential Mode" bits="1" offset="8" min="0" max="1" default="1" />
		<feature name="Defective Pixel Correction Enable" bits="1" offset="9" min="0" max="1" default="1" />
		<feature name="Context A/B Select" bits="1" offset="15" min="0" max="1" default="0" />
	</register>
	<register address="0x08">
		<feature name="A: Coarse Shutter Width 1" bits="15" min="0" max="32765" default="443" />
	</register>
	<register address="0x09">
		<feature name="A: Coarse Shutter Width 2" bits="15" min="0" max="32765" default="473" />
	</register>
	<register address="0x0A">
		<feature name="A: T2 Ratio" bits="4" min="0" max="15" default="4" />
		<feature name="A: T3 Ratio" bits="4" offset="4" min="0" max="15" default="6" />
		<feature name="A: Exposure Knee Point Auto Adjust Enable" bits="1" offset="8" min="0" max="1" default="1" />
		<feature name="A: Single Knee Enable" bits="1" offset="9" min="0" max="1" default="0" />
	</register>
	<register address="0x0B">
		<!--<feature name="A: Coarse Shutter Width Total" bits="15" min="2" max="32765" default="480" />-->
		<!-- Limited to 1024 since high is generally problematic -->
		<feature name="A: Coarse Shutter Width Total" bits="15" min="2" max="1024" default="480" />
	</register>
	<register address="0x0C">
		<feature name="Soft Reset" bits="1" min="0" max="1" default="0" />
		<feature name="Auto Block Soft Reset" bits="1" offset="1" min="0" max="1" default="0" />
	</register>
	<register address="0x0D">
		<feature name="A: Row Bin" bits="2" min="0" max="2" default="0" />
		<feature name="Column Bin" bits="2" offset="2" min="0" max="2" default="0" />
		<feature name="Row Flip" bits="1" offset="4" min="0" max="1" default="0" />
		<feature name="Column Flip" bits="1" offset="5" min="0" max="1" default="0" />
		<feature name="Show Dark Rows" bits="1" offset="6" min="0" max="1" default="0" />
		<feature name="Show Dark Columns" bits="1" offset="7" min="0" max="1" default="0" />
	</register>
	<register address="0x0E">
		<feature name="B: Row Bin" bits="2" min="0" max="2" default="0" />
	</register>
	<register address="0x0F">
		<feature name="A: High Dynamic Range" bits="1" min="0" max="1" default="0" />
		<feature name="Colour/Mono" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="B: High Dynamic Range" bits="1" offset="8" min="0" max="1" default="1" />
	</register>
	<register address="0x1B">
		<feature name="Disable LED_OUT" bits="1" min="0" max="1" default="0" />
		<feature name="Invert LED_OUT" bits="1" offset="1" min="0" max="1" default="0" />
	</register>
	<register address="0x1C">
		<feature name="A: ADC Mode" bits="2" min="2" max="3" default="2" />
		<feature name="B: ADC Mode" bits="2" offset="8" min="2" max="3" default="3" />
	</register>
	<register address="0x2C">
		<feature name="VREF_ADC Voltage Level" bits="3" min="0" max="7" default="4" />
	</register>
	<register address="0x31">
		<feature name="A: V1 Voltage Level" bits="6" min="0" max="63" default="39" />
	</register>
	<register address="0x32">
		<feature name="A: V2 Voltage Level" bits="6" min="0" max="63" default="26" />
	</register>
	<register address="0x33">
		<feature name="A: V3 Voltage Level" bits="6" min="0" max="63" default="5" />
	</register>
	<register address="0x34">
		<feature name="A: V4 Voltage Level" bits="6" min="0" max="63" default="3" />
	</register>
	<register address="0x35">
		<feature name="A: Global Analog Gain" bits="7" min="16" max="64" default="16" />
		<feature name="A: Global Analog Gain Attenuation" bits="1" offset="15" min="0" max="1" default="0" />
	</register>
	<register address="0x36">
		<feature name="B: Global Analog Gain" bits="7" min="16" max="64" default="16" />
		<feature name="B: Global Analog Gain Attenuation" bits="1" offset="15" min="0" max="1" default="1" />
	</register>
	<register address="0x39">
		<feature name="B: V1 Voltage Level" bits="6" min="0" max="63" default="39" />
	</register>
	<register address="0x3A">
		<feature name="B: V2 Voltage Level" bits="6" min="0" max="63" default="38" />
	</register>
	<register address="0x3B">
		<feature name="B: V3 Voltage Level" bits="6" min="0" max="63" default="5" />
	</register>
	<register address="0x3C">
		<feature name="B: V4 Voltage Level" bits="6" min="0" max="63" default="3" />
	</register>
	<register address="0x42">
		<feature name="Frame Dark Average" bits="8" min="0" max="0" default="0" readonly="yes" />
	</register>
	<register address="0x46">
		<feature name="Lower Dark Average Threshold" bits="8" min="0" max="255" default="29" />
		<feature name="Upper Dark Average Threshold" bits="8" offset="8" min="0" max="255" default="35" />
	</register>
	<register address="0x47">
		<feature name="Black Level Calibration Manual Override" bits="1" min="0" max="1" default="0" />
		<feature name="Black Level Averaging Frame Span" bits="3" offset="5" min="0" max="7" default="4" />
	</register>
	<register address="0x48">
		<!--<feature name="Black Level Calibration Value" bits="8" min="-127" max="127" default="0" />-->
		<feature name="Black Level Calibration Value" bits="8" min="0" max="255" default="0" />
	</register>
	<register address="0x4C">
		<feature name="Black Level Calibration Step Size" bits="5" min="0" max="31" default="2" />
	</register>
	<register address="0x70">
		<feature name="A: Enable Noise Correction" bits="1" min="0" max="1" default="0" />
		<feature name="A: Use Black Level Average" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="B: Enable Noise Correction" bits="1" offset="8" min="0" max="1" default="0" />
		<feature name="B: Use Black Level Average" bits="1" offset="9" min="0" max="1" default="0" />
	</register>
	<register address="0x71">
		<feature name="Row Noise Constant" bits="10" min="0" max="1023" default="42" />
	</register>
	<register address="0x72">
		<feature name="Invert Line Valid" bits="1" min="0" max="1" default="0" />
		<feature name="Invert Frame Valid" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="XOR Line Valid" bits="1" offset="2" min="0" max="1" default="0" />
		<feature name="Continuous Line Valid" bits="1" offset="3" min="0" max="1" default="0" />
		<feature name="Invert Pixel Clock" bits="1" offset="4" min="0" max="1" default="0" />
	</register>
	<register address="0x7F">
		<feature name="Two-Wire Serial Interface Test Data" bits="10" min="0" max="1023" default="0" />
		<feature name="Use Two-Wire Serial Interface Test Data" bits="1" offset="10" min="0" max="1" default="0" />
		<feature name="Grey Shade Test Pattern" bits="2" offset="11" min="0" max="3" default="0" />
		<feature name="Test Enable" bits="1" offset="13" min="0" max="1" default="0" />
		<feature name="Flip Two-Wire Serial Interface Test Data" bits="1" offset="14" min="0" max="1" default="0" />
	</register>
	<register address="0x80">
		<feature name="A: Tile Gain X0 Y0" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X0 Y0" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X0 Y0" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x81">
		<feature name="A: Tile Gain X1 Y0" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X1 Y0" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X1 Y0" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x82">
		<feature name="A: Tile Gain X2 Y0" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X2 Y0" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X2 Y0" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x83">
		<feature name="A: Tile Gain X3 Y0" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X3 Y0" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X3 Y0" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x84">
		<feature name="A: Tile Gain X4 Y0" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X4 Y0" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X4 Y0" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x85">
		<feature name="A: Tile Gain X0 Y1" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X0 Y1" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X0 Y1" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x86">
		<feature name="A: Tile Gain X1 Y1" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X1 Y1" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X1 Y1" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x87">
		<feature name="A: Tile Gain X2 Y1" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X2 Y1" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X2 Y1" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x88">
		<feature name="A: Tile Gain X3 Y1" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X3 Y1" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X3 Y1" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x89">
		<feature name="A: Tile Gain X4 Y1" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X4 Y1" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X4 Y1" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8A">
		<feature name="A: Tile Gain X0 Y2" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X0 Y2" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X0 Y2" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8B">
		<feature name="A: Tile Gain X1 Y2" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X1 Y2" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X1 Y2" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8C">
		<feature name="A: Tile Gain X2 Y2" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X2 Y2" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X2 Y2" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8D">
		<feature name="A: Tile Gain X3 Y2" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X3 Y2" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X3 Y2" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8E">
		<feature name="A: Tile Gain X4 Y2" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X4 Y2" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X4 Y2" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x8F">
		<feature name="A: Tile Gain X0 Y3" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X0 Y3" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X0 Y3" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x90">
		<feature name="A: Tile Gain X1 Y3" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X1 Y3" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X1 Y3" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x91">
		<feature name="A: Tile Gain X2 Y3" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X2 Y3" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X2 Y3" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x92">
		<feature name="A: Tile Gain X3 Y3" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X3 Y3" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X3 Y3" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x93">
		<feature name="A: Tile Gain X4 Y3" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X4 Y3" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X4 Y3" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x94">
		<feature name="A: Tile Gain X0 Y4" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X0 Y4" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X0 Y4" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x95">
		<feature name="A: Tile Gain X1 Y4" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X1 Y4" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X1 Y4" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x96">
		<feature name="A: Tile Gain X2 Y4" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X2 Y4" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X2 Y4" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	)xml" R"xml(
	<register address="0x97">
		<feature name="A: Tile Gain X3 Y4" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X3 Y4" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X3 Y4" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x98">
		<feature name="A: Tile Gain X4 Y4" bits="4" min="1" max="15" default="4" />
		<feature name="Sample Weight X4 Y4" bits="4" offset="4" min="1" max="15" default="15" />
		<feature name="B: Tile Gain X4 Y4" bits="4" offset="8" min="1" max="15" default="4" />
	</register>
	<register address="0x99">
		<feature name="Tile Coordinate X0" bits="10" min="0" max="752" default="0" />
	</register>
	<register address="0x9A">
		<feature name="Tile Coordinate X1" bits="10" min="0" max="752" default="150" />
	</register>
	<register address="0x9B">
		<feature name="Tile Coordinate X2" bits="10" min="0" max="752" default="300" />
	</register>
	<register address="0x9C">
		<feature name="Tile Coordinate X3" bits="10" min="0" max="752" default="450" />
	</register>
	<register address="0x9D">
		<feature name="Tile Coordinate X4" bits="10" min="0" max="752" default="600" />
	</register>
	<register address="0x9E">
		<feature name="Tile Coordinate X4 End" bits="10" min="0" max="752" default="752" />
	</register>
	<register address="0x9F">
		<feature name="Tile Coordinate Y0" bits="9" min="0" max="480" default="0" />
	</register>
	<register address="0xA0">
		<feature name="Tile Coordinate Y1" bits="9" min="0" max="480" default="96" />
	</register>
	<register address="0xA1">
		<feature name="Tile Coordinate Y2" bits="9" min="0" max="480" default="192" />
	</register>
	<register address="0xA2">
		<feature name="Tile Coordinate Y3" bits="9" min="0" max="480" default="288" />
	</register>
	<register address="0xA3">
		<feature name="Tile Coordinate Y4" bits="9" min="0" max="480" default="384" />
	</register>
	<register address="0xA4">
		<feature name="Tile Coordinate Y4 End" bits="9" min="0" max="480" default="480" />
	</register>
	<register address="0XA5">
		<feature name="AEC/AGC Desired Bin" bits="6" min="1" max="64" default="58" />
	</register>
	<register address="0xA6">
		<feature name="AEC Frame Skip" bits="4" min="0" max="15" default="2" />
	</register>
	<register address="0xA8">
		<feature name="AEC Low Pass Filter" bits="2" min="0" max="2" default="0" />
	</register>
	<register address="0xA9">
		<feature name="AGC Frame Skip" bits="4" min="0" max="15" default="2" />
	</register>
	<register address="0xAA">
		<feature name="AGC Low Pass Filter" bits="2" min="0" max="2" default="2" />
	</register>
	<register address="0xAB">
		<feature name="Maximum Analog Gain" bits="7" min="16" max="64" default="64" />
	</register>
	<register address="0xAC">
		<feature name="Minimum Coarse Shutter Width Total" bits="16" min="1" max="32765" default="1" />
	</register>
	<register address="0xAD">
		<feature name="Maximum Coarse Shutter Width Total" bits="16" min="1" max="32765" default="480" />
	</register>
	<register address="0xAE">
		<feature name="AEC/AGC Bin Difference Threshold" bits="8" min="0" max="63" default="14" />
	</register>
	<register address="0xAF">
		<feature name="A: AEC Enable" bits="1" min="0" max="1" default="1" />
		<feature name="A: AGC Enable" bits="1" offset="1" min="0" max="1" default="1" />
		<feature name="B: AEC Enable" bits="1" offset="8" min="0" max="1" default="0" />
		<feature name="B: AGC Enable" bits="1" offset="9" min="0" max="1" default="0" />
	</register>
	<register address="0xB0">
		<feature name="AEC/AGC Pixel Count" bits="16" min="0" max="65535" default="44000" />
	</register>
	<register address="0xB1">
		<feature name="PLL Bypass" bits="1" min="0" max="1" default="0" />
		<feature name="LVDS Power Down" bits="1" offset="1" min="0" max="1" default="1" />
		<feature name="PLL Test Mode" bits="1" offset="2" min="0" max="1" default="0" />
		<feature name="LVDS Test Mode" bits="1" offset="3" min="0" max="1" default="0" />
	</register>
	<register address="0xB2">
		<feature name="Shift Clock Delay Element Select" bits="3" min="0" max="7" default="0" />
		<feature name="LVDS Clock Output Enable" bits="1" offset="4" min="0" max="1" default="1" />
	</register>
	<register address="0xB3">
		<feature name="LVDS Data Delay Element Select" bits="3" min="0" max="7" default="0" />
		<feature name="LVDS Data Input Enable" bits="1" offset="4" min="0" max="1" default="1" />
	</register>
	<register address="0xB4">
		<feature name="LVDS Stream Latency Select" bits="1" min="0" max="1" default="0" />
	</register>
	<register address="0xB5">
		<feature name="LVDS Internal Sync Enable" bits="1" min="0" max="1" default="0" />
	</register>
	<register address="0xB6">
		<feature name="LVDS Payload Use 10-Bit Pixel Enable" bits="1" min="0" max="1" default="0" />
	</register>
	<register address="0xB7">
		<feature name="Enable Stereo Error Detect" bits="1" min="0" max="1" default="0" />
		<feature name="Enable Stick Stereo Error Flag" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="Clear Stereo Error Flag" bits="1" offset="2" min="0" max="1" default="0" />
	</register>
	<register address="0xB8">
		<feature name="Stereoscopy Error Flag" bits="1" min="0" max="0" default="0" readonly="yes" />
	</register>
	<register address="0xB9">
		<feature name="LVDS Data Output" bits="16" min="0" max="0" default="0" readonly="yes" />
	</register>
	<register address="0xBA">
		<feature name="AGC Gain" bits="7" min="0" max="0" default="10" readonly="yes" />
	</register>
	<register address="0XBB">
		<feature name="AEC Exposure" bits="16" min="0" max="0" default="200" readonly="yes" />
	</register>
	<register address="0xBC">
		<feature name="AEC/AGC Current Bin" bits="6" min="0" max="0" default="0" readonly="yes" />
	</register>
	<register address="0xBF">
		<feature name="Field Vertical Blank" bits="9" min="1" max="255" default="22" />
	</register>
	<register address="0xC0">
		<feature name="Image Capture Count" bits="8" min="1" max="255" default="10" />
	</register>
	<register address="0xC2">
		<feature name="Anti-Eclipse Enable" bits="1" offset="7" min="0" max="1" default="0" />
		<feature name="V_rst_lim Voltage Level" bits="3" offset="11" min="0" max="7" default="1" />
	</register>
	<register address="0xC6">
		<feature name="Extend Frame Valid" bits="1" min="0" max="1" default="0" />
		<feature name="Replace FV/LV With Ped/Sync" bits="1" offset="1" min="0" max="1" default="0" />
	</register>
	<register address="0xC7">
		<feature name="Front Porch Width" bits="8" min="0" max="255" default="22" />
		<feature name="Sync Width" bits="8" offset="8" min="0" max="255" default="68" />
	</register>
	<register address="0xC8">
		<feature name="Equalizing Pulse Width" bits="8" min="0" max="255" default="33" />
		<feature name="Vertical Serration Width" bits="8" offset="8" min="0" max="255" default="68" />
	</register>
	<register address="0xC9">
		<feature name="B: Column Start" bits="10" min="1" max="752" default="1" />
	</register>
	<register address="0xCA">
		<feature name="B: Row Start" bits="9" min="4" max="482" default="4" />
	</register>
	<register address="0xCB">
		<feature name="B: Window Height" bits="9" min="1" max="480" default="480" />
	</register>
	<register address="0xCC">
		<feature name="B: Window Width" bits="10" min="1" max="752" default="752" />
	</register>
	<register address="0xCD">
		<feature name="B: Horizontal Blanking" bits="10" min="61" max="1023" default="94" />
	</register>
	<register address="0xCE">
		<feature name="B: Vertical Blanking" bits="15" min="2" max="32288" default="45" />
	</register>
	<register address="0xCF">
		<feature name="B: Coarse Shutter Width 1" bits="15" min="0" max="32765" default="478" />
	</register>
	<register address="0xD0">
		<feature name="B: Coarse Shutter Width 2" bits="15" min="0" max="32765" default="479" />
	</register>
	<register address="0xD1">
		<feature name="B: T2 Ratio" bits="4" min="0" max="15" default="4" />
		<feature name="B: T3 Ratio" bits="4" offset="4" min="0" max="15" default="6" />
		<feature name="B: Exposure Knee Point Auto Adjust Enable" bits="1" offset="8" min="0" max="1" default="1" />
		<feature name="B: Single Knee Enable" bits="1" offset="9" min="0" max="1" default="0" />
	</register>
	<register address="0xD2">
		<!-- <feature name="B: Coarse Shutter Width Total" bits="15" min="2" max="32765" default="480" /> -->
		<!-- Limited to 1024 since high is generally problematic -->
		<feature name="B: Coarse Shutter Width Total" bits="15" min="2" max="1024" default="480" />
	</register>
	<register address="0xD3">
		<feature name="A: Fine Shutter Width 1" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD4">
		<feature name="A: Fine Shutter Width 2" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD5">
		<feature name="A: Fine Shutter Width Total" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD6">
		<feature name="B: Fine Shutter Width 1" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD7">
		<feature name="B: Fine Shutter Width 2" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD8">
		<feature name="B: Fine Shutter Width Total" bits="11" min="0" max="1774" default="0" />
	</register>
	<register address="0xD9">
		<feature name="Monitor Mode Enable" bits="1" min="0" max="1" default="0" />
	</register>
	<register address="0xFE">
		<feature name="Register Lock Code" bits="16" min="48879" max="57007" default="48879" invalid="48880-57004,57006" />
	</register>
</camera>
)xml"; }}
