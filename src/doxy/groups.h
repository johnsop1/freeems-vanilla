/**	@file groups.h
 * @ingroup doxygenDocFiles
 * @brief Doxygen module groups definition
 *
 * @note The order of group definitions within this file determines the order
 * of them on the modules page so place new groups accordingly.
 */

/** @defgroup interruptHandlers Interrupt Service Routines
 *
 * These files contain special functions that take no arguments and have void
 * return type. They can not take arguments or return anything as they are
 * called asynchronously by the hardware through a special jump table that
 * contains the start address of each one. Typically these functions take care
 * of handling asyncronous external events that are triggered through some sort
 * of peripheral module. Examples of types of events that require interrupts
 * and associated handlers are listed below.
 *
 * @b Examples:
 * - change of pin voltage state (high to low or vice versa)
 * - data received by a communication module
 * - buffer for data to be sent by a communication module requires refilling
 * - real time clock ticks
 * - timer module ticks
 */

/** @defgroup enginePositionRPMDecoders Engine Position/RPM Decoders
 *
 * All of the code that reads the high speed engine
 * position signals can be found linked to from here.
 *
 * Currently only NipponDenso is semi functional.
 */

/** @defgroup communicationsFiles Communications
 *
 * All files that are involved in communicating with something outside the MCU
 * can be found here.
 *
 * @b Examples:
 * - UART/SCI
 * - CAN
 * - SPI
 * - I2C
 */

/** @defgroup globalHeaders Global Headers
 *
 * These key files are included in most source files by default.
 *
 * @b Examples:
 * - Global defines
 * - Global constants
 * - Global variables
 * - Error codes
 * - Device header
 * - Struct typedefs
 */

/** @defgroup dataInitialisers Data Initialisers
 *
 * These files provide initial data definitions for items that live in flash.
 *
 * @b Examples:
 * - Tables
 * - Tunables
 * - Fixed config
 * - Lookup tables
 * - Global constants
 */

/** @defgroup measurementsAndCalculations Measurements And Calculations
 *
 * These files all operate on inputs in order to eventually generate outputs.
 *
 * @b Examples:
 * - ADC readings
 * - Core variable generation
 * - Core variable averaging
 * - Derived variable generation
 * - Fuel calculations
 * - Ignition calculations
 */

/** @defgroup allHeaders All Header Files
 *
 * Most of these are fairly uninteresting, hence being clost to the bottom of
 * the modules list. Typically they just contain function declarations for
 * inclusion in other source files.
 */

/** @defgroup testing Test Files
 *
 * A very small group at this stage, this is intended to contain various files
 * that manipulate the functional code by providing it with artificial inputs
 * instead of real values read from sensors.
 */

/** @defgroup doxygenDocFiles Doxygen Documentation Files
 *
 * These files exist purely to contain documentation for extraction
 * by Doxygen. Using this grouping keeps them off the file list.
 */
