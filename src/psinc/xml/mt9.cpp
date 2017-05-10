namespace psinc { namespace chip { const char *mt9 = R"xml(
<?xml version="1.0" encoding="utf-8"?>
<camera chip="mt9" bits="16" manufacturer="PSI" author="dan" contexts="2" hdr="true" bayer="2" sizeByRange="true">
	<alias context="0" name="ColumnStart" feature="x_addr_start" />
	<alias context="0" name="RowStart" feature="y_addr_start" />
	<alias context="0" name="ColumnEnd" feature="x_addr_end" />
	<alias context="0" name="RowEnd" feature="y_addr_end" />
	<alias context="0" name="Exposure" feature="coarse_integration_time" />
	<alias context="0" name="Gain" feature="col_gain" />

	<alias context="1" name="ColumnStart" feature="x_addr_start_cb" />
	<alias context="1" name="RowStart" feature="y_addr_start_cb" />
	<alias context="1" name="ColumnEnd" feature="x_addr_end_cb" />
	<alias context="1" name="RowEnd" feature="y_addr_end_cb" />
	<alias context="1" name="Exposure" feature="coarse_integration_time_cb" />
	<alias context="1" name="Gain" feature="col_gain_cb" />

	<alias name="Context" feature="context_b" />
	<alias name="AutoExposure" feature="ae_enable" />
	<alias name="AutoGain" feature="auto_ag_en" />


	<register address="0x3000"><feature name="chip_version_reg" bits="16" min="0" max="0" default="0" readonly="yes" /></register>
	<register address="0x3002"><feature name="y_addr_start" bits="16" min="4" max="963" default="4" /></register>
	<register address="0x3004"><feature name="x_addr_start" bits="16" min="2" max="1281" default="2" /></register>
	<register address="0x3006"><feature name="y_addr_end" bits="16" min="4" max="963" default="963" /></register>
	<register address="0x3008"><feature name="x_addr_end" bits="16" min="4" max="1281" default="1281" /></register>

	<register address="0x300a"><feature name="frame_length_lines" bits="16" min="0" max="65535" default="990" /></register>
	<register address="0x300c"><feature name="line_length_pck" bits="16" min="1650" max="" default="1650" /></register>
	<register address="0x300e"><feature name="revision_number" bits="8" min="0" max="255" default="34" /></register>
	<register address="0x3010"><feature name="lock_control" bits="16" min="48815" max="48879" default="48879" invalid="48816-48878" /></register>
	<register address="0x3012"><feature name="coarse_integration_time" bits="16" min="3" max="65535" default="16" /></register>
	<register address="0x3014"><feature name="fine_integration_time" bits="16" min="0" max="65535" default="0" /></register>
	<register address="0x3016"><feature name="coarse_integration_time_cb" bits="16" min="3" max="65535" default="16" /></register>
	<register address="0x3018"><feature name="fine_integration_time_cb" bits="16" min="0" max="65535" default="0" /></register>

	<register address="0x301a">
		<feature name="grouped_parameter_hold" bits="1" offset="15" min="0" max="0" default="0" />
		<feature name="smia_serialiser_dis" bits="1" offset="12" min="0" max="1" default="0" />
		<feature name="forced_pll_on" bits="1" offset="11" min="0" max="1" default="0" />
		<feature name="restart_bad" bits="1" offset="10" min="0" max="1" default="0" />
		<feature name="mask_bad" bits="1" offset="9" min="0" max="1" default="0" />
		<feature name="gpi_en" bits="1" offset="8" min="0" max="1" default="0" />
		<feature name="parallel_en" bits="1" offset="7" min="0" max="1" default="1" />
		<feature name="drive_pins" bits="1" offset="6" min="0" max="1" default="1" />
		<feature name="stdby_eof" bits="1" offset="4" min="0" max="1" default="1" />
		<feature name="lock_reg" bits="1" offset="3" min="0" max="1" default="1" />
		<feature name="stream" bits="1" offset="2" min="0" max="1" default="0" />
		<feature name="restart" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="reset" bits="1" offset="0" min="0" max="1" default="0" />
	</register>

	<register address="0x301e"><feature name="data_pedestal" bits="16" min="0" max="65535" default="300" /></register>

	<register address="0x3026">
		<feature name="standby" bits="1" offset="3" min="0" max="1" default="0" readonly="yes" />
		<feature name="trigger" bits="1" offset="2" min="0" max="1" default="0" readonly="yes" />
		<feature name="oe_n" bits="1" offset="1" min="0" max="1" default="0" readonly="yes" />
		<feature name="saddr" bits="1" offset="0" min="0" max="1" default="0" readonly="yes" />
	</register>

	<register address="0x3028"><feature name="row_speed" bits="3" offset="4" min="0" max="7" default="1" /></register>
	<register address="0x302a"><feature name="vt_pix_clk_div" bits="8" min="0" max="255" default="6" /></register>
	<register address="0x302c"><feature name="vt_sys_clk_div" bits="5" min="0" max="31" default="1" /></register>
	<register address="0x302e"><feature name="pre_pll_clk_div" bits="6" min="0" max="63" default="2" /></register>
	<register address="0x3030"><feature name="pll_multiplier" bits="8" min="0" max="255" default="44" /></register>

	<register address="0x3032">
		<feature name="digital_binning_cb" bits="2" offset="4" min="0" max="2" default="0" />
		<feature name="digital_binning_ca" bits="2" offset="0" min="0" max="2" default="0" />
	</register>

	<register address="0x303a"><feature name="frame_count" bits="16" min="0" max="65535" default="0" /></register>

	<register address="0x303c">
		<feature name="standby_status" bits="1" offset="1" min="0" max="1" default="0" readonly="yes" />
		<feature name="framesync" bits="1" offset="0" min="0" max="1" default="0" readonly="yes" />
	</register>

	<register address="0x3040">
		<feature name="vert_flip" bits="1" offset="15" min="0" max="1" default="0" />
		<feature name="horiz_mirror" bits="1" offset="14" min="0" max="1" default="0" />
	</register>

	<register address="0x3044">
		<feature name="show_colcorr_rows" bits="1" offset="12" min="0" max="1" default="0" />
		<feature name="show_dark_extra_rows" bits="1" offset="11" min="0" max="1" default="0" />
		<feature name="row_noise_correction_en" bits="1" offset="10" min="0" max="1" default="1" />
		<feature name="show_dark_cols" bits="1" offset="9" min="0" max="1" default="0" />
	</register>

	<register address="0x3046">
		<feature name="strobe" bits="1" offset="15" min="0" max="1" default="0" readonly="yes" />
		<feature name="triggered" bits="1" offset="14" min="0" max="1" default="0" readonly="yes" />
		<feature name="en_flash" bits="1" offset="8" min="0" max="1" default="0" />
		<feature name="invert_flash" bits="1" offset="7" min="0" max="1" default="0" />
	</register>

	<register address="0x3056">
		<feature name="green1_gain_int" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="green1_gain_frac" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x3058">
		<feature name="blue_gain_int" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="blue_gain_frac" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x305a">
		<feature name="red_gain_int" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="red_gain_frac" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x305c">
		<feature name="green2_gain_int" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="green2_gain_frac" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x305e">
		<feature name="global_gain_int" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="global_gain_frac" bits="5" offset="0" min="0" max="31" default="0" />
	</register>

	<register address="0x3064">
		<feature name="embedded_data" bits="1" offset="8" min="0" max="1" default="1" />
		<feature name="embedded_stats_en" bits="1" offset="7" min="0" max="1" default="1" />
	</register>

	<register address="0x306e">
		<feature name="slew_rate_ctrl_parallel" bits="3" offset="13" min="0" max="7" default="4" />
		<feature name="slew_rate_ctrl_pixclk" bits="3" offset="10" min="0" max="7" default="4" />
		<feature name="postscaler_data_sel" bits="1" offset="8" min="0" max="1" default="0" />
		<feature name="true_bayer" bits="1" offset="4" min="0" max="1" default="0" />
		<feature name="special_line_valid" bits="2" offset="0" min="0" max="2" default="0" />
	</register>

	<register address="0x3070"><feature name="test_pattern_mode" bits="9" min="0" max="256" default="0" invalid="4-255" /></register>
	<register address="0x3072"><feature name="test_data_red" bits="12" min="0" max="4095" default="0" /></register>
	<register address="0x3074"><feature name="test_data_greenr" bits="12" min="0" max="4095" default="0" /></register>
	<register address="0x3076"><feature name="test_data_blue" bits="12" min="0" max="4095" default="0" /></register>
	<register address="0x3078"><feature name="test_data_greenb" bits="12" min="0" max="4095" default="0" /></register>
	<register address="0x307a"><feature name="test_raw_mode" bits="2" min="0" max="3" default="0" /></register>

	<register address="0x3086"><feature name="seq_data_port" bits="16" min="0" max="65535" default="0" /></register>
	<register address="0x3088">
		<feature name="sequencer_stopped" bits="1" offset="15" min="0" max="0" default="0" readonly="yes" />
		<feature name="auto_inc_on_read" bits="1" offset="14" min="0" max="1" default="1" />
		<feature name="access_address" bits="9" offset="0" min="0" max="511" default="0" />
	</register>

	<register address="0x308a"><feature name="x_addr_start_cb" bits="16" min="2" max="1281" default="2" /></register>
	<register address="0x308c"><feature name="y_addr_start_cb" bits="16" min="4" max="963" default="4" /></register>
	<register address="0x308e"><feature name="x_addr_end_cb" bits="16" min="4" max="1281" default="1281" /></register>
	<register address="0x3090"><feature name="y_addr_end_cb" bits="16" min="4" max="963" default="963" /></register>

	<register address="0x30a0"><feature name="x_even_inc" bits="1" min="0" max="0" default="1" readonly="yes" /></register>
	<register address="0x30a2"><feature name="x_odd_inc" bits="1" min="0" max="1" default="1" /></register>
	<register address="0x30a4"><feature name="y_even_inc" bits="1" min="0" max="0" default="1" readonly="yes" /></register>
	<register address="0x30a6"><feature name="y_odd_inc" bits="7" min="1" max="127" default="1" invalid="2,4-6,8-14,16-30,32-62,64-126" /></register>
	<register address="0x30a8"><feature name="y_odd_inc_cb" bits="7" min="1" max="127" default="63" invalid="2,4-6,8-14,16-30,32-62,64-126" /></register>
	<register address="0x30aa"><feature name="frame_length_lines_cb" bits="16" min="0" max="65535" default="90" /></register>
	<register address="0x30ac"><feature name="frame_exposure" bits="16" min="0" max="0" default="16" readonly="yes" /></register>

	<register address="0x30b0">
		<feature name="pll_complete_bypass" bits="1" offset="14" min="0" max="1" default="0" />
		<feature name="context_b" bits="1" offset="13" min="0" max="1" default="0" />
		<feature name="col_gain_cb" bits="2" offset="8" min="0" max="3" default="0" />
		<feature name="mono_chrome" bits="1" offset="7" min="0" max="1" default="1" />
		<feature name="col_gain" bits="2" offset="4" min="0" max="3" default="0" />
	</register>

	<register address="0x30b2"><feature name="tempsens_data" bits="10" min="0" max="1023" default="0" /></register>
	<register address="0x30b4">
		<feature name="temp_clear_value" bits="1" offset="5" min="0" max="1" default="0" />
		<feature name="temp_start_conversion" bits="1" offset="4" min="0" max="1" default="0" />
		<feature name="tempsens_test_ctrl" bits="3" offset="1" min="0" max="7" default="0" />
		<feature name="tempsens_power_on" bits="1" offset="0" min="0" max="1" default="0" />
	</register>
)xml" R"xml(
	<register address="0x30bc">
		<feature name="green1_gain_int_cb" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="green1_gain_frac_cb" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x30be">
		<feature name="blue_gain_int_cb" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="blue_gain_frac_cb" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x30c0">
		<feature name="red_gain_int_cb" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="red_gain_frac_cb" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x30c2">
		<feature name="green2_gain_int_cb" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="green2_gain_frac_cb" bits="5" offset="0" min="0" max="31" default="0" />
	</register>
	<register address="0x30c4">
		<feature name="global_gain_int_cb" bits="3" offset="5" min="0" max="7" default="1" />
		<feature name="global_gain_frac_cb" bits="5" offset="0" min="0" max="31" default="0" />
	</register>

	<register address="0x30c6"><feature name="tempsens_calib1" bits="16" min="0" max="65535" default="291" /></register>
	<register address="0x30c8"><feature name="tempsens_calib2" bits="16" min="0" max="65535" default="17767" /></register>
	<register address="0x30ca"><feature name="tempsens_calib3" bits="16" min="0" max="65535" default="35243" /></register>
	<register address="0x30cc"><feature name="tempsens_calib4" bits="16" min="0" max="65535" default="52719" /></register>

	<register address="0x30d4">
		<feature name="enable_column_correction" bits="1" offset="15" min="0" max="1" default="1" />
		<feature name="double_range" bits="1" offset="14" min="0" max="1" default="1" />
		<feature name="double_samples" bits="1" offset="13" min="0" max="1" default="1" />
		<feature name="colcorr_rows" bits="4" offset="0" min="0" max="15" default="7" />
	</register>

	<register address="0x3100">
		<feature name="min_ana_gain" bits="2" offset="5" min="0" max="3" default="0" />
		<feature name="auto_dg_en" bits="1" offset="4" min="0" max="1" default="0" />
		<feature name="auto_ag_en" bits="1" offset="1" min="0" max="1" default="0" />
		<feature name="ae_enable" bits="1" offset="0" min="0" max="1" default="0" />
	</register>

	<register address="0x3102"><feature name="ae_luma_target_reg" bits="16" min="0" max="65535" default="1280" /></register>
	<register address="0x3108"><feature name="ae_min_ev_step_reg" bits="8" min="0" max="255" default="112" /></register>
	<register address="0x310a"><feature name="ae_max_ev_step_reg" bits="16" min="0" max="65535" default="8" /></register>
	<register address="0x310c"><feature name="ae_damp_offset_reg" bits="16" min="0" max="65535" default="512" /></register>
	<register address="0x310e"><feature name="ae_damp_gain_reg" bits="16" min="0" max="65535" default="8192" /></register>
	<register address="0x3110"><feature name="ae_damp_max_reg" bits="16" min="0" max="65535" default="320" /></register>
	<register address="0x311c"><feature name="ae_max_exposure_reg" bits="16" min="0" max="65535" default="672" /></register>
	<register address="0x311e"><feature name="ae_min_exposure_reg" bits="16" min="0" max="65535" default="1" /></register>
	<register address="0x3124"><feature name="ae_dark_cur_thresh_reg" bits="16" min="0" max="65535" default="32767" /></register>

	<register address="0x312a">
		<feature name="ae_ana_gain" bits="2" offset="8" min="0" max="0" default="0" readonly="yes" />
		<feature name="ae_dig_gain" bits="8" offset="0" min="0" max="0" default="0" readonly="yes" />
	</register>

	<register address="0x3140"><feature name="ae_roi_x_start_offset" bits="11" min="0" max="2047" default="0" /></register>
	<register address="0x3142"><feature name="ae_roi_y_start_offset" bits="10" min="0" max="1023" default="0" /></register>
	<register address="0x3144"><feature name="ae_roi_x_size" bits="11" min="0" max="2047" default="1280" /></register>
	<register address="0x3146"><feature name="ae_roi_y_size" bits="10" min="0" max="1023" default="960" /></register>
	<register address="0x3152"><feature name="ae_mean_l" bits="16" min="0" max="0" default="0" readonly="yes" /></register>
	<register address="0x3164"><feature name="ae_coarse_integration_time" bits="16" min="0" max="0" default="0" readonly="yes" /></register>
	<register address="0x3166"><feature name="ae_ag_exposure_hi" bits="16" min="0" max="65535" default="986" /></register>
	<register address="0x3168"><feature name="ae_ag_exposure_lo" bits="16" min="0" max="65535" default="419" /></register>
	<register address="0x3188"><feature name="delta_dk_level" bits="16" min="0" max="0" default="0" readonly="yes" /></register>

	<register address="0x31c0">
		<feature name="hispi_timing_laneclk" bits="3" offset="12" min="0" max="7" default="0" />
		<feature name="hispi_timing_lane3" bits="3" offset="9" min="0" max="7" default="0" />
		<feature name="hispi_timing_lane2" bits="3" offset="6" min="0" max="7" default="0" />
		<feature name="hispi_timing_lane1" bits="3" offset="3" min="0" max="7" default="0" />
		<feature name="hispi_timing_lane0" bits="3" offset="0" min="0" max="7" default="0" />
	</register>

	<register address="0x31c6">
		<feature name="hispi_status" bits="2" offset="14" min="0" max="0" default="0" readonly="yes" />
		<feature name="hispi_control" bits="8" offset="2" min="0" max="255" default="0" readonly="yes" />
	</register>

	<register address="0x31c8"><feature name="hispi_crc_0" bits="16" min="0" max="0" default="65535" readonly="yes" /></register>
	<register address="0x31ca"><feature name="hispi_crc_1" bits="16" min="0" max="0" default="65535" readonly="yes" /></register>
	<register address="0x31cc"><feature name="hispi_crc_2" bits="16" min="0" max="0" default="65535" readonly="yes" /></register>
	<register address="0x31ce"><feature name="hispi_crc_3" bits="16" min="0" max="0" default="65535" readonly="yes" /></register>

	<register address="0x31d2"><feature name="stat_frame_id" bits="16" min="0" max="0" default="0" /></register>
	<register address="0x31d6"><feature name="i2c_wrt_checksum" bits="16" min="0" max="0" default="65535" /></register>

	<register address="0x31e8"><feature name="horizontal_cursor_position" bits="10" min="0" max="1023" default="0" /></register>
	<register address="0x31ea"><feature name="vertical_cursor_position" bits="11" min="0" max="2047" default="0" /></register>
	<register address="0x31ec"><feature name="horizontal_cursor_width" bits="10" min="0" max="1023" default="0" /></register>
	<register address="0x31ee"><feature name="vertical_cursor_width" bits="11" min="0" max="2047" default="0" /></register>
	<register address="0x31fc"><feature name="i2c_ids" bits="16" min="0" max="65535" default="12320" /></register>
</camera>
)xml"; }}



