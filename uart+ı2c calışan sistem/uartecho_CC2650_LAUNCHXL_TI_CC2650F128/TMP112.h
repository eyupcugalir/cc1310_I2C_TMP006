/*
 * TMP112.h
 *
 *  Created on: Apr 25, 2016
 *      Author: antsis
 */

#ifndef TMP112_H_
#define TMP112_H_

//brief This define contains the address of the TMP112A IC.
#define IC_ADDRESS_TMP112_ADDRESS 		0x0048

//! \brief TMP112A temperature register
#define TEMPERATURE_REGISTER		0x00

//! \brief TMP112A configuration register
#define CONFIGURATION_REGISTER		0x01

//! \brief TMP112A temperature low register
#define TLOW_REGISTER				0x02

//! \brief TMP112A temperature high register
#define THIGH_REGISTER				0x03




#endif /* TMP112_H_ */
