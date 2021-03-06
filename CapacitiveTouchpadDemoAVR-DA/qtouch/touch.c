/*============================================================================
Filename : touch.c
Project : QTouch Modular Library
Purpose : Provides Initialization, Processing and ISR handler of touch library,
          Simple API functions to get/set the key touch parameters from/to the
          touch library data structures

This file is part of QTouch Modular Library Release 7.2 application.

Important Note: Do not edit this file manually.
                Use QTouch Configurator within Atmel Start to apply any
                modifications to this file.

Usage License: Refer license.h file for license information
Support: Visit http://www.microchip.com/support/hottopics.aspx
               to create MySupport case.

------------------------------------------------------------------------------
Copyright (c) 2020 Microchip. All rights reserved.
------------------------------------------------------------------------------
============================================================================*/
#ifndef TOUCH_C
#define TOUCH_C
/*----------------------------------------------------------------------------
 *     include files
 *----------------------------------------------------------------------------*/

#include "touch.h"
#include "license.h"

#include "port.h"
#include "struct.h"

#if KRONOCOMM_UART == 1u
#include "KronoCommuart.h"
#endif

/*----------------------------------------------------------------------------
 *   prototypes
 *----------------------------------------------------------------------------*/

/*! \brief configure the ptc port pins to Input
 */
static void touch_ptc_pin_config(void);

/*! \brief configure keys, wheels and sliders.
 */
static touch_ret_t touch_sensors_config(void);

/*! \brief Touch measure complete callback function example prototype.
 */
static void qtm_measure_complete_callback(void);

/*! \brief Touch Error callback function prototype.
 */
static void qtm_error_callback(uint8_t error);


#if (DEF_TOUCH_LOWPOWER_ENABLE == 1u)
/* low power processing function */
static void touch_process_lowpower();

/* low power touch detection callback */
static void touch_measure_wcomp_match(void);
/* Cancel low-power measurement */
static void touch_cancel_autoscan(void);

static void touch_enable_lowpower_measurement(void);
static void touch_disable_lowpower_measurement(void);

#endif



/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

extern volatile struct SysFlags Flags;

/* Flag to indicate time for touch measurement */
volatile uint8_t time_to_measure_touch_flag = 0;

/* postporcess request flag */
volatile uint8_t touch_postprocess_request = 0;

/* Measurement Done Touch Flag  */
volatile uint8_t measurement_done_touch = 0;

/* Error Handling */
uint8_t module_error_code = 0;

/* Low-power measurement variables */
uint16_t time_since_touch = 0;
/* store the drift period for comparison */
uint16_t measurement_period_store = 0;

/* Acquisition module internal data - Size to largest acquisition set */
uint16_t touch_acq_signals_raw[DEF_NUM_CHANNELS];

/* Acquisition set 1 - General settings */
qtm_acq_node_group_config_t ptc_qtlib_acq_gen1
    = {DEF_NUM_CHANNELS, DEF_SENSOR_TYPE, DEF_PTC_CAL_AUTO_TUNE, DEF_SEL_FREQ_INIT};

/* Node status, signal, calibration values */
qtm_acq_node_data_t ptc_qtlib_node_stat1[DEF_NUM_CHANNELS];

/* Node configurations */
qtm_acq_avr_da_node_config_t ptc_seq_node_cfg1[DEF_NUM_CHANNELS] = {NODE_0_PARAMS,
                                                                    NODE_1_PARAMS,
                                                                    NODE_2_PARAMS,
                                                                    NODE_3_PARAMS,
                                                                    NODE_4_PARAMS,
                                                                    NODE_5_PARAMS,
                                                                    NODE_6_PARAMS,
                                                                    NODE_7_PARAMS,
																	NODE_8_PARAMS};

/* Container */
qtm_acquisition_control_t qtlib_acq_set1 = {&ptc_qtlib_acq_gen1, &ptc_seq_node_cfg1[0], &ptc_qtlib_node_stat1[0]};
	
/* Low-power autoscan related parameters */
qtm_auto_scan_config_t auto_scan_setup
	= {&qtlib_acq_set1, QTM_AUTOSCAN_NODE, QTM_AUTOSCAN_THRESHOLD, QTM_AUTOSCAN_TRIGGER_PERIOD};

/**********************************************************/
/*********************** Keys Module **********************/
/**********************************************************/

/* Keys set 1 - General settings */
qtm_touch_key_group_config_t qtlib_key_grp_config_set1 = {DEF_NUM_SENSORS,
                                                          DEF_TOUCH_DET_INT,
                                                          DEF_MAX_ON_DURATION,
                                                          DEF_ANTI_TCH_DET_INT,
                                                          DEF_ANTI_TCH_RECAL_THRSHLD,
                                                          DEF_TCH_DRIFT_RATE,
                                                          DEF_ANTI_TCH_DRIFT_RATE,
                                                          DEF_DRIFT_HOLD_TIME,
                                                          DEF_REBURST_MODE};

qtm_touch_key_group_data_t qtlib_key_grp_data_set1;

/* Key data */
qtm_touch_key_data_t qtlib_key_data_set1[DEF_NUM_SENSORS];

/* Key Configurations */
qtm_touch_key_config_t qtlib_key_configs_set1[DEF_NUM_SENSORS]
    = {KEY_0_PARAMS, KEY_1_PARAMS, KEY_2_PARAMS, KEY_3_PARAMS, KEY_4_PARAMS, KEY_5_PARAMS, KEY_6_PARAMS, KEY_7_PARAMS, KEY_8_PARAMS};

/* Container */
qtm_touch_key_control_t qtlib_key_set1
    = {&qtlib_key_grp_data_set1, &qtlib_key_grp_config_set1, &qtlib_key_data_set1[0], &qtlib_key_configs_set1[0]};

/**********************************************************/
/***************** Surface 1t Module ********************/
/**********************************************************/

qtm_surface_cs_config_t qtm_surface_cs_config1 = {
    /* Config: */
    SURFACE_CS_START_KEY_H,
    SURFACE_CS_NUM_KEYS_H,
    SURFACE_CS_START_KEY_V,
    SURFACE_CS_NUM_KEYS_V,
    SURFACE_CS_RESOL_DB,
    SURFACE_CS_POS_HYST,
    SURFACE_CS_FILT_CFG,
    SURFACE_CS_MIN_CONTACT,
    &qtlib_key_data_set1[0]};

/* Surface Data */
qtm_surface_contact_data_t qtm_surface_cs_data1;

/* Container */
qtm_surface_cs_control_t qtm_surface_cs_control1 = {&qtm_surface_cs_data1, &qtm_surface_cs_config1};

/**********************************************************/
/***************** Gesture Module ********************/
/**********************************************************/

/* Gesture Configurations */
qtm_gestures_2d_config_t qtm_gestures_2d_config = {&qtm_surface_cs_data1.h_position,
                                                   &qtm_surface_cs_data1.v_position,
                                                   &qtm_surface_cs_data1.qt_surface_status,
                                                   0,
                                                   0,
                                                   0,
                                                   SCR_RESOLUTION(SURFACE_CS_RESOL_DB),
                                                   TAP_RELEASE_TIMEOUT,
                                                   TAP_HOLD_TIMEOUT,
                                                   SWIPE_TIMEOUT,
                                                   HORIZONTAL_SWIPE_DISTANCE_THRESHOLD,
                                                   VERTICAL_SWIPE_DISTANCE_THRESHOLD,
                                                   0,
                                                   TAP_AREA,
                                                   SEQ_TAP_DIST_THRESHOLD,
                                                   EDGE_BOUNDARY,
                                                   WHEEL_POSTSCALER,
                                                   WHEEL_START_QUADRANT_COUNT,
                                                   WHEEL_REVERSE_QUADRANT_COUNT,

                                                   0

};

qtm_gestures_2d_data_t qtm_gestures_2d_data;

qtm_gestures_2d_control_t qtm_gestures_2d_control1 = {&qtm_gestures_2d_data, &qtm_gestures_2d_config};

static void touch_ptc_pin_config(void)
{

	PORTD_set_pin_pull_mode(5, PORT_PULL_OFF);
	PORTD_pin_set_isc(5, PORT_ISC_INPUT_DISABLE_gc);

	PORTD_set_pin_pull_mode(0, PORT_PULL_OFF);
	PORTD_pin_set_isc(0, PORT_ISC_INPUT_DISABLE_gc);

	PORTA_set_pin_pull_mode(6, PORT_PULL_OFF);
	PORTA_pin_set_isc(6, PORT_ISC_INPUT_DISABLE_gc);

	PORTD_set_pin_pull_mode(1, PORT_PULL_OFF);
	PORTD_pin_set_isc(1, PORT_ISC_INPUT_DISABLE_gc);

	PORTD_set_pin_pull_mode(6, PORT_PULL_OFF);
	PORTD_pin_set_isc(6, PORT_ISC_INPUT_DISABLE_gc);

	PORTA_set_pin_pull_mode(5, PORT_PULL_OFF);
	PORTA_pin_set_isc(5, PORT_ISC_INPUT_DISABLE_gc);

	PORTD_set_pin_pull_mode(7, PORT_PULL_OFF);
	PORTD_pin_set_isc(7, PORT_ISC_INPUT_DISABLE_gc);

	PORTA_set_pin_pull_mode(4, PORT_PULL_OFF);
	PORTA_pin_set_isc(4, PORT_ISC_INPUT_DISABLE_gc);
	
	//Disable the Extra Pins for Expander 2 header
	PORTE_set_pin_pull_mode(2, PORT_PULL_OFF);
	PORTE_pin_set_isc(2, PORT_ISC_INPUT_DISABLE_gc);
	
	PORTE_set_pin_pull_mode(3, PORT_PULL_OFF);
	PORTE_pin_set_isc(3, PORT_ISC_INPUT_DISABLE_gc);
	
	PORTF_set_pin_pull_mode(4, PORT_PULL_OFF);
	PORTF_pin_set_isc(4, PORT_ISC_INPUT_DISABLE_gc);
	
	PORTF_set_pin_pull_mode(5, PORT_PULL_OFF);
	PORTF_pin_set_isc(5, PORT_ISC_INPUT_DISABLE_gc);
}

/*============================================================================
static touch_ret_t touch_sensors_config(void)
------------------------------------------------------------------------------
Purpose: Initialization of touch key sensors
Input  : none
Output : none
Notes  :
============================================================================*/
/* Touch sensors config - assign nodes to buttons / wheels / sliders / surfaces / water level / etc */
static touch_ret_t touch_sensors_config(void)
{
	uint16_t    sensor_nodes;
	touch_ret_t touch_ret = TOUCH_SUCCESS;

	/* Init acquisition module */
	qtm_ptc_init_acquisition_module(&qtlib_acq_set1);

	/* Init pointers to DMA sequence memory */
	qtm_ptc_qtlib_assign_signal_memory(&touch_acq_signals_raw[0]);

	/* Initialize sensor nodes */

	for (sensor_nodes = 0u; sensor_nodes < DEF_NUM_CHANNELS; sensor_nodes++) {
		/* Enable each node for measurement and mark for calibration */
		qtm_enable_sensor_node(&qtlib_acq_set1, sensor_nodes);
		qtm_calibrate_sensor_node(&qtlib_acq_set1, sensor_nodes);
	}

	/* Enable sensor keys and assign nodes */
	for (sensor_nodes = 0u; sensor_nodes < DEF_NUM_CHANNELS; sensor_nodes++) {
		qtm_init_sensor_key(&qtlib_key_set1, sensor_nodes, &ptc_qtlib_node_stat1[sensor_nodes]);
	}

	touch_ret |= qtm_init_surface_cs(&qtm_surface_cs_control1);

	touch_ret |= qtm_init_gestures_2d();

	return (touch_ret);
}

/*============================================================================
static void qtm_measure_complete_callback( void )
------------------------------------------------------------------------------
Purpose: this function is called after the completion of
         measurement cycle. This function sets the post processing request
         flag to trigger the post processing.
Input  : none
Output : none
Notes  :
============================================================================*/
static void qtm_measure_complete_callback(void)
{
	touch_postprocess_request = 1u;
}

/*============================================================================
static void qtm_error_callback(uint8_t error)
------------------------------------------------------------------------------
Purpose: this function is used to report error in the modules.
Input  : error code
Output : decoded module error code
Notes  :
Derived Module_error_codes:
    Acquisition module error =1
    post processing module1 error = 2
    post processing module2 error = 3
    ... and so on
============================================================================*/
static void qtm_error_callback(uint8_t error)
{
	module_error_code = error + 1u;

}

/*============================================================================
void Timer_set_period(const uint16_t val)
------------------------------------------------------------------------------
Purpose: This function sets the time interval on the RTC/Timer peripheral based
         on the user configuration.
Input  : Time interval
Output : none
Notes  :
============================================================================*/
void Timer_set_period(const uint16_t val)
{
	if (val != 0) {
		while (RTC.STATUS & RTC_PERBUSY_bm)
		;          /* wait for RTC synchronization */
		RTC.PER = val; /* Set period register */
		RTC.INTCTRL |= (1 << RTC_CMP_bp);
		} else { /* if the value is zero, disable interrupt */
		RTC.INTCTRL &= ~(1 << RTC_CMP_bp);
	}

	while (RTC.STATUS & RTC_CNTBUSY_bm)
	;         /* wait for RTC synchronization */
	RTC.CNT = 0u; /* Clear count register */
}

/*============================================================================
void touch_init(void)
------------------------------------------------------------------------------
Purpose: Initialization of touch library. PTC, timer, and
         datastreamer modules are initialized in this function.
Input  : none
Output : none
Notes  :
============================================================================*/
void touch_init(void)
{

	/* Set match value for timer */
	Timer_set_period(DEF_TOUCH_MEASUREMENT_PERIOD_MS);

	/* configure the PTC pins for Input*/
	touch_ptc_pin_config();
	
	#if DEF_TOUCH_LOWPOWER_ENABLE == 1u
	touch_disable_lowpower_measurement();
	#endif

	/* Configure touch sensors with Application specific settings */
	touch_sensors_config();
}

/*============================================================================
void touch_process(void)
------------------------------------------------------------------------------
Purpose: Main processing function of touch library. This function initiates the
         acquisition, calls post processing after the acquistion complete and
         sets the flag for next measurement based on the sensor status.
Input  : none
Output : none
Notes  :
============================================================================*/
void touch_process(void)
{
	touch_ret_t touch_ret;

	/* check the time_to_measure_touch_flag flag for Touch Acquisition */
	if (time_to_measure_touch_flag == 1u) {
		/* Do the acquisition */
		touch_ret = qtm_ptc_start_measurement_seq(&qtlib_acq_set1, qtm_measure_complete_callback);

		/* if the Acquistion request was successful then clear the request flag */
		if (TOUCH_SUCCESS == touch_ret) {
			/* Clear the Measure request flag */
			time_to_measure_touch_flag = 0u;
		}
	}

	/* check the flag for node level post processing */
	if (touch_postprocess_request == 1u) {
		/* Reset the flags for node_level_post_processing */
		touch_postprocess_request = 0u;

		/* Run Acquisition module level post processing*/
		touch_ret = qtm_acquisition_process();

		/* Check the return value */
		if (TOUCH_SUCCESS == touch_ret) {
			/* Returned with success: Start module level post processing */
			touch_ret = qtm_key_sensors_process(&qtlib_key_set1);
			if (TOUCH_SUCCESS != touch_ret) {
				qtm_error_callback(1);
			}
			touch_ret = qtm_surface_cs_process(&qtm_surface_cs_control1);
			if (TOUCH_SUCCESS != touch_ret) {
				qtm_error_callback(2);
			}
			touch_ret = qtm_gestures_2d_process(&qtm_gestures_2d_control1);
			if (TOUCH_SUCCESS != touch_ret) {
				qtm_error_callback(3);
			}
		} else {
			/* Acq module Eror Detected: Issue an Acq module common error code 0x80 */
			qtm_error_callback(0);
		}

		if ((0u != (qtlib_key_set1.qtm_touch_key_group_data->qtm_keys_status & 0x80u))) {
			time_to_measure_touch_flag = 1u;
		} else {
			measurement_done_touch = 1u;
			#if (DEF_TOUCH_LOWPOWER_ENABLE == 1u)
			if (0u != (qtlib_key_grp_data_set1.qtm_keys_status & QTM_KEY_DETECT)) {
				/* Something in detect */
				time_since_touch = 0u;
			}
			/* process lowpower touch measurement */
			touch_process_lowpower();
			#endif
		}

#if KRONOCOMM_ENABLE == 1u
		cpu_irq_disable();
		Krono_UpdateBuffer();
		cpu_irq_enable();
#endif
	}

#if KRONOCOMM_ENABLE == 1u
	uart_process();
#endif
}


#if (DEF_TOUCH_LOWPOWER_ENABLE == 1u)

/*============================================================================
static void touch_disable_lowpower_measurement(void)
------------------------------------------------------------------------------
Purpose:
Input  : none
Output : none
Notes  :
============================================================================*/
static void touch_disable_lowpower_measurement(void)
{
	/* Disable RTC to PTC Event system */
	EVSYS.USERPTCSTART = EVSYS_USER_OFF_gc;
	if (QTM_AUTOSCAN_TRIGGER_PERIOD > NODE_SCAN_512MS) {
		EVSYS.CHANNEL0     = EVSYS_CHANNEL0_OFF_gc;
	} else {
		EVSYS.CHANNEL1     = EVSYS_CHANNEL1_OFF_gc;
	}
	
	while (RTC.STATUS > 0) { /* Wait for all register to be synchronized */
	}
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	| 0 << RTC_RTCEN_bp     /* Disable */
	| 1 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */

	while (RTC.PITSTATUS > 0) { /* Wait for all register to be synchronized */
	}
	/* Disable PIT */
	RTC.PITCTRLA = (0 << RTC_PITEN_bp);

	measurement_period_store = DEF_TOUCH_MEASUREMENT_PERIOD_MS;

	Timer_set_period(measurement_period_store);

	while (RTC.STATUS > 0) { /* Wait for all register to be synchronized */
	}
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	| 1 << RTC_RTCEN_bp     /* Enable */
	| 1 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */
	
}

/*============================================================================
static void touch_enable_lowpower_measurement(void)
------------------------------------------------------------------------------
Purpose:
Input  : none
Output : none
Notes  :
============================================================================*/
static void touch_enable_lowpower_measurement(void)
{
    /* Enable RTC to PTC Event system and Set drift wakeup period*/
    if (QTM_AUTOSCAN_TRIGGER_PERIOD > NODE_SCAN_512MS) {
	    EVSYS.CHANNEL0     = EVSYS_CHANNEL0_RTC_PIT_DIV8192_gc + (NODE_SCAN_8192MS - QTM_AUTOSCAN_TRIGGER_PERIOD);
	    EVSYS.USERPTCSTART = EVSYS_USER_CHANNEL0_gc;
	    } else {
	    EVSYS.CHANNEL1     = EVSYS_CHANNEL1_RTC_PIT_DIV512_gc + (NODE_SCAN_512MS - QTM_AUTOSCAN_TRIGGER_PERIOD);
	    EVSYS.USERPTCSTART = EVSYS_USER_CHANNEL1_gc;
    }	
	
	while (RTC.STATUS > 0) { /* Wait for all register to be synchronized */
	}
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	| 0 << RTC_RTCEN_bp     /* Disable */
	| 1 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */

	while (RTC.PITSTATUS > 0) { /* Wait for all register to be synchronized */
	}
	/* Enable PIT */
	RTC.PITCTRLA = ((QTM_AUTOSCAN_TRIGGER_PERIOD << 2)|(1 << RTC_PITEN_bp));

	measurement_period_store = DEF_TOUCH_DRIFT_PERIOD_MS;

	Timer_set_period(measurement_period_store);

	while (RTC.STATUS > 0) { /* Wait for all register to be synchronized */
	}
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	| 1 << RTC_RTCEN_bp     /* Enable */
	| 1 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */
		
}

/*============================================================================
static void touch_process_lowpower(void)
------------------------------------------------------------------------------
Purpose: Processing function for low-power touch measurment.
         Monitors the touch activity and if there is not touch for longer
         duration, low-power autoscan is enabled.
Input  : none
Output : none
Notes  :
============================================================================*/
static void touch_process_lowpower(void)
{
    touch_ret_t touch_ret;
	
	if (time_since_touch >= DEF_TOUCH_TIMEOUT) {

		/* Start Autoscan */
		touch_ret = qtm_autoscan_sensor_node(&auto_scan_setup, touch_measure_wcomp_match);
		//Get a Flag for sleep mode to turn off all LEDs
		Flags.Sleep = 1;
		
		if ((measurement_period_store != DEF_TOUCH_DRIFT_PERIOD_MS) && (touch_ret == TOUCH_SUCCESS)) {

			/* Enable Event System */
			touch_enable_lowpower_measurement();
		}
	} else if (measurement_period_store != DEF_TOUCH_MEASUREMENT_PERIOD_MS) {

		/* Cancel node auto scan */
		qtm_autoscan_node_cancel();

		/* disable event system measurement */
		touch_disable_lowpower_measurement();
	}
}

/*============================================================================
static void touch_measure_wcomp_match(void)
------------------------------------------------------------------------------
Purpose: callback of autoscan function
Input  : none
Output : none
Notes  :
============================================================================*/
void touch_measure_wcomp_match(void)
{
	if (measurement_period_store != DEF_TOUCH_MEASUREMENT_PERIOD_MS) {
		touch_cancel_autoscan();
		time_to_measure_touch_flag =1u;
		time_since_touch = 0u;
		
		//Get a Flag for Wake UP MCU
		Flags.WakeUp = 1;
	}
}
/*============================================================================
static void touch_cancel_autoscan(void)
------------------------------------------------------------------------------
Purpose: cancelling of autoscan functionality
Input  : none
Output : none
Notes  :
============================================================================*/
static void touch_cancel_autoscan(void)
{

	/* disable event system measurement */
	touch_disable_lowpower_measurement();
}

#endif



/*============================================================================
void touch_timer_handler(void)
------------------------------------------------------------------------------
Purpose: This function updates the time elapsed to the touch key module to
         synchronize the internal time counts used by the module.
Input  : none
Output : none
Notes  :
============================================================================*/
void touch_timer_handler(void)
{
	time_to_measure_touch_flag = 1u;
	
#if (DEF_TOUCH_LOWPOWER_ENABLE == 1u)
	qtm_update_qtlib_timer(measurement_period_store);
	qtm_update_gesture_2d_timer(measurement_period_store/DEF_GESTURE_TIME_BASE_MS);
	if (time_since_touch < (65535u - measurement_period_store)) {
		time_since_touch += measurement_period_store;
		} else {
		time_since_touch = 65535u;
	}
#else
	qtm_update_qtlib_timer(DEF_TOUCH_MEASUREMENT_PERIOD_MS);
	qtm_update_gesture_2d_timer(1);
#endif
}

uint16_t get_sensor_node_signal(uint16_t sensor_node)
{
	return (ptc_qtlib_node_stat1[sensor_node].node_acq_signals);
}

void update_sensor_node_signal(uint16_t sensor_node, uint16_t new_signal)
{
	ptc_qtlib_node_stat1[sensor_node].node_acq_signals = new_signal;
}

uint16_t get_sensor_node_reference(uint16_t sensor_node)
{
	return (qtlib_key_data_set1[sensor_node].channel_reference);
}

void update_sensor_node_reference(uint16_t sensor_node, uint16_t new_reference)
{
	qtlib_key_data_set1[sensor_node].channel_reference = new_reference;
}

uint16_t get_sensor_cc_val(uint16_t sensor_node)
{
	return (ptc_qtlib_node_stat1[sensor_node].node_comp_caps);
}

void update_sensor_cc_val(uint16_t sensor_node, uint16_t new_cc_value)
{
	ptc_qtlib_node_stat1[sensor_node].node_comp_caps = new_cc_value;
}

uint8_t get_sensor_state(uint16_t sensor_node)
{
	return (qtlib_key_set1.qtm_touch_key_data[sensor_node].sensor_state);
}

void update_sensor_state(uint16_t sensor_node, uint8_t new_state)
{
	qtlib_key_set1.qtm_touch_key_data[sensor_node].sensor_state = new_state;
}

void calibrate_node(uint16_t sensor_node)
{
	/* Calibrate Node */
	qtm_calibrate_sensor_node(&qtlib_acq_set1, sensor_node);
	/* Initialize key */
	qtm_init_sensor_key(&qtlib_key_set1, sensor_node, &ptc_qtlib_node_stat1[sensor_node]);
}

/*============================================================================
ISR(QTML_PTC_vect)
------------------------------------------------------------------------------
Purpose:  Interrupt handler for PTC EOC Interrupt
Input    :  none
Output  :  none
Notes    :  none
============================================================================*/
#if defined(__ICCAVR__)
ISR(QTML_PTC_vect_iar)
#else
ISR(QTML_PTC_vect)
#endif
{
	qtm_avr_da_ptc_handler_eoc();
}

#endif /* TOUCH_C */
