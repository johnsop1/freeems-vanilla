/*	commsCore.c

	Copyright 2008 Fred Cooke

	This file is part of the FreeEMS project.

	FreeEMS software is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	FreeEMS software is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with any FreeEMS software.  If not, see <http://www.gnu.org/licenses/>.

	We ask that if you make any changes to this file you send them upstream to us at admin@diyefi.org

	Thank you for choosing FreeEMS to run your engine! */

#define COMMSCORE_C
#include "inc/freeEMS.h"
#include "inc/flashWrite.h"
#include "inc/interrupts.h"
#include "inc/utils.h"
#include "inc/tableLookup.h"
#include "inc/blockDetailsLookup.h"
#include "inc/commsCore.h"
#include <string.h>


/* Internal use without check on buffer */
void sendErrorInternal(unsigned short) FPAGE_FE;
void sendDebugInternal(unsigned char*) FPAGE_FE;


/* If changing this, update the max constant */
void populateBasicDatalog(){
	/* Save the current position */
	unsigned char* position = TXBufferCurrentPositionHandler;

	/* Get core vars */
	memcpy(TXBufferCurrentPositionHandler, CoreVars, sizeof(CoreVar));
	TXBufferCurrentPositionHandler += sizeof(CoreVar);
	/* Get derived vars */
	memcpy(TXBufferCurrentPositionHandler, DerivedVars, sizeof(DerivedVar));
	TXBufferCurrentPositionHandler += sizeof(DerivedVar);
	/* Get raw adc counts */
	memcpy(TXBufferCurrentPositionHandler, ADCArrays, sizeof(ADCArray));
	TXBufferCurrentPositionHandler += sizeof(ADCArray);

	/* Set/Truncate the log to the specified length */
	TXBufferCurrentPositionHandler = position + configuredBasicDatalogLength;
}


// TODO function to setup a packet and send it fn(populateBodyFunctionPointer(), header, other, fields, here, and, use, or, not, within){}

// TODO rip dictionary out to own file

// TODO factor many things into functions and move the receive delegator to its own file


//void populateLogicAnalyser(){
//	// get portT rpm input and inj main
//	// get portB ign
//	// get portA ign
//	// get portK inj staged
//}
//
//
//// All of these require some range checking, eg only some registers, and all ram, not flash, not other regs
//// TODO pointer for one byte
//// TODO pointer for one short
//// TODO function to log generic memory region by location and size ? requires length!
//// Ranges are :
//// ram window
//// bss/data region
//// IO registers etc that can't be altered simply by reading from.
//// NOT :
//// flash makes no sense
//// some regs are sensitive
//// some ram is unused
//// serial buffers make no sense
//// eeprom makes no sense
////
//// 2k of regs max - user beware for now
//// 12k of ram max
//
//init :
//logaddr = fixed.addr
//loglen = fixed.len
//
//len = loglen OR 1 OR 2
//
//check :
//if((addr < 0x0800) && (length < (0x0800 - addr))){
//	// reg space is OK
//}else if(((0x1000 < addr) && (addr < 0x4000)) && (length < (0x4000 - addr))){
//	// ram space is OK
//}else{
//	// send an error instead
//}
//
//run check at init and set time, not run time or just not check?? maybe its silly to check at all
//
///* Just dump the ADC channels as fast as possible */
//void populateScopeLogADCAll(){
//	sampleBlockADC(TXBufferCurrentPositionHandler);
//	TXBufferCurrentPositionHandler += sizeof(ADCArray);
//}


// TODO Look at the time stamps and where to write them, also whether to function
// TODO call these simple blocks or write one function that handles all the logic.


/* Checksum and initiate send process */
void checksumAndSend(){
	/* Get the length from the pointer */
	unsigned short TXPacketLengthToSend = (unsigned short)TXBufferCurrentPositionHandler - (unsigned short)&TXBuffer;

	/* Tag the checksum on the end */
	*TXBufferCurrentPositionHandler = checksum((unsigned char*)&TXBuffer, TXPacketLengthToSend);
	TXPacketLengthToSend++;

	/* Send it out on all the channels required. */

	/* SCI0 - Main serial interface */
	if(TXBufferInUseFlags & COM_SET_SCI0_INTERFACE_ID){
		/* Copy numbers to interface specific vars */
		TXPacketLengthToSendSCI0 = TXPacketLengthToSend;
		TXPacketLengthToSendCAN0 = TXPacketLengthToSend;

		/* Queue preamble by clearing and then setting transmit enable	*/
		/* See section 11.4.5.2 of the xdp512 specification document	*/
		//SCI0CR2 &= SCICR2_TX_DISABLE;
		//SCI0CR2 |= SCICR2_TX_ENABLE;

		/* Initiate transmission */
		SCI0DRL = START_BYTE;
		while(!(SCI0SR1 & 0x80)){/* Wait for ever until able to send then move on */}
		SCI0DRL = START_BYTE; // TODO nasty hack that works... means at least one and most 2 starts are sent so stuff works, but is messy... must be a better way.

		// TODO http://freeems.aaronb.info/tracker/view.php?id=81

		/* Note : Order Is Important! */
		/* TX empty flag is already set, so we must clear it by writing out before enabling the interrupt */
		SCI0CR2 |= SCICR2_TX_ISR_ENABLE;
	}
	/* CAN0 - Main CAN interface */
	if(TXBufferInUseFlags & COM_SET_CAN0_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_CAN0_INTERFACE_ID;
	}
	/* spare2 */
	if(TXBufferInUseFlags & COM_SET_SPARE2_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE2_INTERFACE_ID;
	}
	/* spare3 */
	if(TXBufferInUseFlags & COM_SET_SPARE3_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE3_INTERFACE_ID;
	}
	/* spare4 */
	if(TXBufferInUseFlags & COM_SET_SPARE4_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE4_INTERFACE_ID;
	}
	/* spare5 */
	if(TXBufferInUseFlags & COM_SET_SPARE5_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE5_INTERFACE_ID;
	}
	/* spare6 */
	if(TXBufferInUseFlags & COM_SET_SPARE6_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE6_INTERFACE_ID;
	}
	/* spare7 */
	if(TXBufferInUseFlags & COM_SET_SPARE7_INTERFACE_ID){
		// just clear up front for now
		TXBufferInUseFlags &= COM_CLEAR_SPARE7_INTERFACE_ID;
	}
}


// hHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
// above this to another file


void decodePacketAndRespond(){
	/* Extract and build up the header fields */
	RXBufferCurrentPosition = (unsigned char*)&RXBuffer;
	TXBufferCurrentPositionHandler = (unsigned char*)&TXBuffer;

	/* Initialised here such that override is possible */
	TXBufferCurrentPositionSCI0 = (unsigned char*)&TXBuffer;
	TXBufferCurrentPositionCAN0 = (unsigned char*)&TXBuffer;

	/* Start this off as full packet length and build down to the actual length */
	RXCalculatedPayloadLength = RXPacketLengthReceived;

	/* Grab the RX header flags out of the RX buffer */
	RXHeaderFlags = *RXBufferCurrentPosition;
	RXBufferCurrentPosition++;
	RXCalculatedPayloadLength--;

	/* Flag that we are transmitting! */
	TXBufferInUseFlags |= COM_SET_SCI0_INTERFACE_ID;
	// SCI0 only for now...

	/* Load a blank header into the TX buffer ready for masking */
	unsigned char* TXHeaderFlags = TXBufferCurrentPositionHandler;
	*TXHeaderFlags = 0;
	TXBufferCurrentPositionHandler++;

	/* Grab the payload ID for processing and load the return ID */
	RXHeaderPayloadID = *((unsigned short*)RXBufferCurrentPosition);
	*((unsigned short*)TXBufferCurrentPositionHandler) = RXHeaderPayloadID + 1;
	RXBufferCurrentPosition += 2;
	TXBufferCurrentPositionHandler += 2;
	RXCalculatedPayloadLength -= 2;

	/* If there is an ack, copy it to the return packet */
	if(RXHeaderFlags & HEADER_HAS_ACK){
		*TXBufferCurrentPositionHandler = *RXBufferCurrentPosition;
		*TXHeaderFlags |= HEADER_HAS_ACK;
		RXBufferCurrentPosition++;
		TXBufferCurrentPositionHandler++;
		RXCalculatedPayloadLength--;
	}

	/* If the header has addresses, check them and if OK copy them */
	if(RXHeaderFlags & HEADER_HAS_ADDRS){
		/* Check the destination address against our address */
		if(*RXBufferCurrentPosition != fixedConfigs2.networkAddress){
			/* Addresses do not match, discard packet without error */
			resetReceiveState(CLEAR_ALL_SOURCE_ID_FLAGS);
			TXBufferInUseFlags = 0;
			return;
		}
		RXBufferCurrentPosition++;

		/* Save and check the source address */
		RXHeaderSourceAddress = *RXBufferCurrentPosition;
		RXBufferCurrentPosition++;
		if(RXHeaderSourceAddress == 0){
			sendErrorInternal(sourceAddressIsBroadcast);
			resetReceiveState(CLEAR_ALL_SOURCE_ID_FLAGS);
			return;
		}
		if(RXHeaderSourceAddress == fixedConfigs2.networkAddress){
			sendErrorInternal(sourceAddressIsDuplicate);
			resetReceiveState(CLEAR_ALL_SOURCE_ID_FLAGS);
			return;
		}

		/* All is well, setup reply addresses */
		*TXHeaderFlags |= HEADER_HAS_ADDRS;
		/* TX destination = RX source */
		*TXBufferCurrentPositionHandler = RXHeaderSourceAddress;
		TXBufferCurrentPositionHandler++;
		/* TX source = our address */
		*TXBufferCurrentPositionHandler = fixedConfigs2.networkAddress;
		TXBufferCurrentPositionHandler++;
		/* Decrement for both at once to save a cycle */
		RXCalculatedPayloadLength -= 2;
	}

	/* Subtract checksum to get final length */
	RXCalculatedPayloadLength--;

	/* Grab the length if available */
	if(RXHeaderFlags & HEADER_HAS_LENGTH){
		RXHeaderPayloadLength = *((unsigned short*)RXBufferCurrentPosition);
		RXBufferCurrentPosition += 2;
		RXCalculatedPayloadLength -= 2;
		/* Already subtracted one for checksum */
		if(RXHeaderPayloadLength != RXCalculatedPayloadLength){
			sendErrorInternal(payloadLengthHeaderMismatch);
			resetReceiveState(CLEAR_ALL_SOURCE_ID_FLAGS);
			return;
		}
	}

	/* Please Note :															*/
	/* length (and it's flag) should be set by each return packet type handler if required or desired.	*/
	/* If an ack has been requested, ensure the negative ack flag is set if the opration failed.		*/

	/* Perform the requested action based on payload ID */
	if (RXHeaderFlags & HEADER_IS_PROTO){ /* Protocol payload types */
		/* Set the return type to be protocol too */
		*TXHeaderFlags |= HEADER_IS_PROTO;

		switch (RXHeaderPayloadID){
		case requestInterfaceVersion:
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* This type must have a length field, set that up */
			*((unsigned short*)TXBufferCurrentPositionHandler) = sizeof(interfaceVersionAndType);
			*TXHeaderFlags |= HEADER_HAS_LENGTH;
			TXBufferCurrentPositionHandler += 2;
			/* Load the body into place */
			memcpy((void*)TXBufferCurrentPositionHandler, (void*)&interfaceVersionAndType, sizeof(interfaceVersionAndType));
			TXBufferCurrentPositionHandler += sizeof(interfaceVersionAndType);
			checksumAndSend();
			break;
		case requestFirmwareVersion:
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}
			/* This type must have a length field, set that up */
			*((unsigned short*)TXBufferCurrentPositionHandler) = sizeof(firmwareVersion);
			*TXHeaderFlags |= HEADER_HAS_LENGTH;
			TXBufferCurrentPositionHandler += 2;
			/* Load the body into place */
			memcpy((void*)TXBufferCurrentPositionHandler, (void*)&firmwareVersion, sizeof(firmwareVersion));
			TXBufferCurrentPositionHandler += sizeof(firmwareVersion);
			checksumAndSend();
			break;
		case requestMaxPacketSize:
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}
			/* Load the size into place */
			*((unsigned short*)TXBufferCurrentPositionHandler) = RX_BUFFER_SIZE;
			TXBufferCurrentPositionHandler += 2;
			checksumAndSend();
			break;
		case requestEchoPacketReturn:
			/* This type must have a length field, set that up */
			*((unsigned short*)TXBufferCurrentPositionHandler) = RXPacketLengthReceived;
			*TXHeaderFlags |= HEADER_HAS_LENGTH;
			TXBufferCurrentPositionHandler += 2;
			/* Load the body into place */
			memcpy((void*)TXBufferCurrentPositionHandler, (void*)&RXBuffer, RXPacketLengthReceived);
			/* Note, there is no overflow check here because the TX buffer is slightly       */
			/* bigger than the RX buffer and there is overflow checking for receives anyway. */
			TXBufferCurrentPositionHandler += RXPacketLengthReceived;
			checksumAndSend();
			break;
		case requestSoftSystemReset:
		{
			// hack to allow datalog on/off from the orange button (thank christ I asked for that button when I did)
			if(asyncDatalogType){
				asyncDatalogType = asyncDatalogOff;
			}else{
				asyncDatalogType = asyncDatalogBasic;
			}
			sendAckIfRequired(); // TODO implement
			break;

//			// hack to use soft reset to request registers
//			/* This type must have a length field, set that up */
//			*((unsigned short*)TXBufferCurrentPositionHandler) = memdumplength;
//			*TXHeaderFlags |= HEADER_HAS_LENGTH;
//			TXBufferCurrentPositionHandler += 2;
//			/* Load the body into place */
//			memcpy((void*)TXBufferCurrentPositionHandler, memdumpaddr, memdumplength);
//			TXBufferCurrentPositionHandler += memdumplength;
//			memdumpaddr += memdumplength;
//			checksumAndSend();
//			break;

//			if(RXCalculatedPayloadLength != 0){
//				sendErrorInternal(payloadLengthTypeMismatch);
//				break;
//			}
//			/* Perform soft system reset */
//			_start();
		}
		case requestHardSystemReset:
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* This is how the serial monitor does it. */
			COPCTL = 0x01; /* Arm with shortest time */
			ARMCOP = 0xFF; /* Write bad value, should cause immediate reset */
			/* Using _start() only resets the app ignoring the monitor switch. It does not work */
			/* properly because the location of _start is not the master reset vector location. */
		default:
			if((RXHeaderPayloadID % 2) == 1){
				sendErrorInternal(invalidProtocolPayloadID);
			}else{
				sendErrorInternal(unrecognisedProtocolPayloadID);
			}
		}
	}else{ /* Otherwise firmware payload types */
		switch (RXHeaderPayloadID) {
		case replaceBlockInRAM:
		{
			/* Extract the ram location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition += 2;

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Subtract two to allow for the locationID */
			if((RXCalculatedPayloadLength - 2) != details.size){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			if((details.RAMPage == 0) || (details.RAMAddress == 0)){
				sendErrorInternal(invalidMemoryActionForID);
				break;
			}

			unsigned short errorID = 0;
			if(locationID < 16){
				mainTable aTable;
				errorID = validateMainTable(&aTable);
			}else if(locationID > 399){
				twoDTableUS aTable;
				errorID = validateTwoDTable(&aTable);
			}// TODO add other table types here
			/* If the validation failed, report it */
			if(errorID != 0){
				sendErrorInternal(errorID);
				break;
			}

			/* Save page values for restore */
			unsigned char oldRamPage = RPAGE;
			/* Set the viewable ram page */
			RPAGE = details.RAMPage;
			/* Copy from the RX buffer to the block of ram */
			memcpy(details.RAMAddress, RXBufferCurrentPosition, details.size);
			/* Restore the original ram and flash pages */
			RPAGE = oldRamPage;

			sendAckIfRequired(); // TODO implement
			break;
		}
		case replaceBlockInFlash:
		{
			/* Extract the ram location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition += 2;

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Subtract two to allow for the locationID */
			if((RXCalculatedPayloadLength - 2) != details.size){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			if((details.FlashPage == 0) || (details.FlashAddress == 0)){
				sendErrorInternal(invalidMemoryActionForID);
				break;
			}

			if(locationID < 16){
				unsigned short errorID = validateMainTable((mainTable*)RXBufferCurrentPosition);
				if(errorID != 0){
					sendErrorInternal(errorID);
					break;
				}
			}/* else{ hope for the best ;-) } */

			/* Copy from the RX buffer to the block of flash */
			unsigned short errorID = writeBlock(details.FlashPage, (unsigned short*)details.FlashAddress, RPAGE, (unsigned short*)RXBufferCurrentPosition, details.size);

			/* If flash write failed for some reason */
			if(errorID != 0){
				sendErrorInternal(errorID);
				break;
			}

			/* If present in ram, update that too */
			if((details.RAMPage != 0) && (details.RAMAddress != 0)){
				/* Save page values for restore */
				unsigned char oldRamPage = RPAGE;
				/* Set the viewable ram page */
				RPAGE = details.RAMPage;
				/* Copy from the RX buffer to the block of ram */
				memcpy(details.RAMAddress, RXBufferCurrentPosition, details.size);
				/* Restore the original ram and flash pages */
				RPAGE = oldRamPage;
			}

			sendAckIfRequired(); // TODO implement
			// TODO document errors can always be returned and add error check in to send as response for ack and async otherwise
			break;
		}
		case retrieveBlockFromRAM:
		{
			if(RXCalculatedPayloadLength != 2){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the ram location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			/* Store it back into the output data */
			*(unsigned short*)TXBufferCurrentPositionHandler = locationID;
			TXBufferCurrentPositionHandler += 2;

			/* If it's a main table we are returning, specify the limits explicitly */
			if(locationID < 16){
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_RPM_LENGTH;
				TXBufferCurrentPositionHandler += 2;
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_LOAD_LENGTH;
				TXBufferCurrentPositionHandler += 2;
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_MAIN_LENGTH;
				TXBufferCurrentPositionHandler += 2;
			}

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			if((details.RAMPage == 0) || (details.RAMAddress == 0)){
				sendErrorInternal(invalidMemoryActionForID);
				break;
			}

			/* Save page value for restore and set the visible page */
			unsigned char oldRamPage = RPAGE;
			RPAGE = details.RAMPage;

			/* Copy the block of ram to the TX buffer */
			memcpy(TXBufferCurrentPositionHandler, details.RAMAddress, details.size);
			TXBufferCurrentPositionHandler += details.size;

			/* Restore the original ram and flash pages */
			RPAGE = oldRamPage;

			checksumAndSend();
			break;
		}
		case retrieveBlockFromFlash:
		{
			if(RXCalculatedPayloadLength != 2){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			/* Store it back into the output data */
			*(unsigned short*)TXBufferCurrentPositionHandler = locationID;
			TXBufferCurrentPositionHandler += 2;

			/* If it's a main table we are returning, specify the limits explicitly */
			if(locationID < 16){
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_RPM_LENGTH;
				TXBufferCurrentPositionHandler += 2;
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_LOAD_LENGTH;
				TXBufferCurrentPositionHandler += 2;
				/* Store it back into the output data */
				*(unsigned short*)TXBufferCurrentPositionHandler = MAINTABLE_MAX_MAIN_LENGTH;
				TXBufferCurrentPositionHandler += 2;
			}

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			if((details.FlashPage == 0) || (details.FlashAddress == 0)){
				sendErrorInternal(invalidMemoryActionForID);
				break;
			}

			/* Save page value for restore and set the visible page */
			unsigned char oldFlashPage = PPAGE;
			PPAGE = details.FlashPage;

			/* Copy the block of flash to the TX buffer */
			memcpy(TXBufferCurrentPositionHandler, details.FlashAddress, details.size);
			TXBufferCurrentPositionHandler += details.size;

			/* Restore the original ram and flash pages */
			PPAGE = oldFlashPage;

			checksumAndSend();
			break;
		}
		case burnBlockFromRamToFlash:
		{
			if(RXCalculatedPayloadLength != 2){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Check that all data we need is present */
			if((details.RAMPage == 0) || (details.RAMAddress == 0) || (details.FlashPage == 0) || (details.FlashAddress == 0)){
				sendErrorInternal(invalidMemoryActionForID);
				break;
			}

			if(details.size < 1024){
				sendErrorInternal(0x0999);
				/* create some sort of function to copy the flash sector up into
				 * the serial rx buffer in the high end and then over write with
				 * the small piece defined either from incoming data, or from its
				 * memory location. Then just call burn in the normal way.
				 *
				 * function could take :
				 * pointer to the buffer region (must be 1024 long or more)
				 * rpage, address, length of data to be persisted
				 * ppage, address of the sector to retrieve the rest of the data from
				 * pointer to the details object we want to use for the following call :
				 */
			}

			unsigned short errorID = writeBlock(details.RAMPage, (unsigned short*)details.RAMAddress, details.FlashPage, (unsigned short*)details.FlashAddress, details.size);

			if(errorID != 0){
				sendErrorInternal(errorID);
				break;
			}

			sendDebugInternal("Copied RAM to Flash!");
			break;
		}
		case eraseAllBlocksFromFlash:
		{
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			// perform function TODO
			unsigned char page = 0xE0;
			unsigned short start = 0x8000;
			unsigned short end = 0xC000;
			unsigned short inc = 0x0400;
			for(;page < 0xF8;page++){
				unsigned short addr;
				for(addr = start;addr < end; addr += inc){
					eraseSector(page, (unsigned short*)addr);
				}
			}
			sendDebugInternal("Erased three 128k Flash blocks!");
			break;
		}
		case burnAllBlocksOfFlash:
		{
			if(RXCalculatedPayloadLength != 0){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			// perform function TODO
			unsigned char page = 0xE0;
			unsigned short start = 0x8000;
			unsigned short end = 0xC000;
			unsigned short inc = 0x0400;
			for(;page < 0xF8;page++){
				unsigned short addr;
				for(addr = start;addr < end; addr += inc){
					writeSector(RPAGE, (unsigned short*)0xc000, page, (unsigned short*)addr);
				}
			}
			sendDebugInternal("Overwrote three 128k Flash blocks!");
			break;
		}
		case adjustMainTableCell:
		{
			if(RXCalculatedPayloadLength != 8){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition += 2;

			/* Check the ID to ensure it is a main table */
			if(15 < locationID){
				sendErrorInternal(invalidIDForMainTableAction);
				break;
			}

			/* Extract the cell value and coordinates */
			unsigned short RPMIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition += 2;
			unsigned short LoadIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition += 2;
			unsigned short cellValue = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Attempt to set the value */
			unsigned short errorID = setPagedMainTableCellValue(details.RAMPage, details.RAMAddress, RPMIndex, LoadIndex, cellValue);
			if(errorID != 0){
				sendErrorInternal(errorID);
			}else{
				sendAckIfRequired();
			}
			break;
		}
		case adjustMainTableRPMAxis:
		{
			if(RXCalculatedPayloadLength != 6){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;

			/* Check the ID to ensure it is a main table */
			if(15 < locationID){
				sendErrorInternal(invalidIDForMainTableAction);
				break;
			}

			/* Extract the cell value and coordinates */
			unsigned short RPMIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;
			unsigned short RPMValue = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Attempt to set the value */
			unsigned short errorID = setPagedMainTableRPMValue(details.RAMPage, details.RAMAddress, RPMIndex, RPMValue);
			if(errorID != 0){
				sendErrorInternal(errorID);
			}else{
				sendAckIfRequired();
			}
			break;
		}
		case adjustMainTableLoadAxis:
		{
			if(RXCalculatedPayloadLength != 6){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;

			/* Check the ID to ensure it is a main table */
			if(15 < locationID){
				sendErrorInternal(invalidIDForMainTableAction);
				break;
			}

			/* Extract the cell value and coordinates */
			unsigned short LoadIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;
			unsigned short LoadValue = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Attempt to set the value */
			unsigned short errorID = setPagedMainTableLoadValue(details.RAMPage, details.RAMAddress, LoadIndex, LoadValue);
			if(errorID != 0){
				sendErrorInternal(errorID);
			}else{
				sendAckIfRequired();
			}
			break;
		}
		case adjust2dTableAxis:
		{
			if(RXCalculatedPayloadLength != 6){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;

			/* Check the ID to ensure it is a 2d table */
			if((locationID > 899) || (locationID < 400)){
				sendErrorInternal(invalidIDForTwoDTableAction);
				break;
			}

			/* Extract the cell value and coordinates */
			unsigned short axisIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;
			unsigned short axisValue = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Attempt to set the value */
			unsigned short errorID = setPagedTwoDTableAxisValue(details.RAMPage, details.RAMAddress, axisIndex, axisValue);
			if(errorID != 0){
				sendErrorInternal(errorID);
			}else{
				sendAckIfRequired();
			}
			break;
		}
		case adjust2dTableCell:
		{
			if(RXCalculatedPayloadLength != 6){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			/* Extract the flash location ID from the received data */
			unsigned short locationID = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;

			/* Check the ID to ensure it is a 2d table */
			if((locationID > 899) || (locationID < 400)){
				sendErrorInternal(invalidIDForTwoDTableAction);
				break;
			}

			/* Extract the cell value and coordinates */
			unsigned short cellIndex = *((unsigned short*)RXBufferCurrentPosition);
			RXBufferCurrentPosition -= 2;
			unsigned short cellValue = *((unsigned short*)RXBufferCurrentPosition);

			/* Look up the memory location details */
			blockDetails details;
			lookupBlockDetails(locationID, &details);

			/* Attempt to set the value */
			unsigned short errorID = setPagedTwoDTableCellValue(details.RAMPage, details.RAMAddress, cellIndex, cellValue);
			if(errorID != 0){
				sendErrorInternal(errorID);
			}else{
				sendAckIfRequired();
			}
			break;
		}
		case requestBasicDatalog:
		{
			if((RXCalculatedPayloadLength > 2) || (RXCalculatedPayloadLength == 1)){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}else if(RXCalculatedPayloadLength == 2){
				unsigned short newConfiguredLength = *((unsigned short*)RXBufferCurrentPosition);
				if(newConfiguredLength > maxBasicDatalogLength){
					sendErrorInternal(datalogLengthExceedsMax);
					break;
				}else{
					configuredBasicDatalogLength = newConfiguredLength;
				}
			}

			/* Set the length field up */
			*TXHeaderFlags |= HEADER_HAS_LENGTH;
			*(unsigned short*)TXBufferCurrentPositionHandler = configuredBasicDatalogLength;

			/* Fill out the log and send */
			populateBasicDatalog();
			checksumAndSend();
			break;
		}
		case requestConfigurableDatalog:
		{
			// perform function TODO
			sendErrorInternal(unimplementedFunction);
			break;
		}
		case forwardPacketOverCAN:
		{
			// perform function TODO
			sendErrorInternal(unimplementedFunction);
			break;
		}
		case forwardPacketOverOtherUART:
		{
			// perform function TODO
			sendErrorInternal(unimplementedFunction);
			break;
		}
		case setAsyncDatalogType:
		{
			if(RXCalculatedPayloadLength != 1){
				sendErrorInternal(payloadLengthTypeMismatch);
				break;
			}

			unsigned char newDatalogType = *((unsigned char*)RXBufferCurrentPosition);
			if(newDatalogType > 0x03){
				sendErrorInternal(noSuchAsyncDatalogType);
				break;
			}else{
				asyncDatalogType = newDatalogType;
			}

			sendAckIfRequired();
			break;
		}
		default:
			if((RXHeaderPayloadID % 2) == 1){
				sendErrorInternal(invalidFirmwarePayloadID);
			}else{
				sendErrorInternal(unrecognisedFirmwarePayloadID);
			}
		}
	}
	/* Switch reception back on now that we are done with the received data */
	resetReceiveState(CLEAR_ALL_SOURCE_ID_FLAGS);
	PORTK |= BIT0;
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//below this to another file for now

/* Wrapper for use outside the com handler */
void sendErrorIfClear(unsigned short errorID){
	if(!TXBufferInUseFlags){
		TXBufferInUseFlags = ONES;
		sendErrorInternal(errorID);
	}else{
		Counters.commsErrorMessagesNotSent++;
	}
}


/* Wrapper for use outside the com handler */
void sendErrorBusyWait(unsigned short errorID){
	while(TXBufferInUseFlags){} /* Wait till clear to send */
	TXBufferInUseFlags = ONES;
	sendErrorInternal(errorID);
}


/* Send an error code out in a frequency limited way */
void sendErrorInternal(unsigned short errorCode){ // TODO build wrapper to check for use inside handler or not and not send if busy and not in handler ditto for debug

//	set buffer in use, consider blocking interrupts to do this cleanly


	//	TXBufferInUseFlags = 0;
	/* No need for atomic block here as one of two conditions will always be */
	/* true when calling this. Either we have been flagged to receive and    */
	/* decode a packet, or we are in an ISR. In either case it is safe to    */
	/* check the flags and initiate the sequence if they are clear.          */
//	if(RXTXSerialStateFlags & TX_IN_PROGRESS){
		/* It's OK to return without resetting as it will be done by */
		/* either of those processes if they are underway. The other */
		/* processes are not overridden because they have priority.  */
//		return;
//	}else{ /* Turn off reception */
		/* It's OK to turn this off if nothing was currently being received */
//		SCI0CR2 &= SCICR2_RX_ISR_DISABLE;
//		SCI0CR2 &= SCICR2_RX_DISABLE;

		/* Build up the packet */
		/* Set the pointer to the start */
		TXBufferCurrentPositionHandler = (unsigned char*)&TXBuffer;
		/* Set the length */
//		TXPacketLengthToSend = 5; /* Flags + Payload ID + Error Code */
		/* Flags = protocol with no extra fields */
		*TXBufferCurrentPositionHandler = 0x01;
		TXBufferCurrentPositionHandler++;
		/* Set the payload ID */
		*((unsigned short*)TXBufferCurrentPositionHandler) = asyncErrorCodePacket;
		TXBufferCurrentPositionHandler += 2;
		/* Set the error code */
		*((unsigned short*)TXBufferCurrentPositionHandler) = errorCode;
		TXBufferCurrentPositionHandler += 2;
		checksumAndSend();
//	}
}


/* Wrapper for use outside the com handler */
void sendDebugIfClear(unsigned char* message){
	if(!TXBufferInUseFlags){
		TXBufferInUseFlags = ONES;
		sendDebugInternal(message);
	}else{
		Counters.commsDebugMessagesNotSent++;
	}
}


/* Wrapper for use outside the com handler */
void sendDebugBusyWait(unsigned char* message){
	while(TXBufferInUseFlags){} /* Wait till clear to send */
	TXBufferInUseFlags = ONES;
	sendDebugInternal(message);
}


/* Send a null terminated message out on the broadcast address */
void sendDebugInternal(unsigned char* message){

//	set buffer in use, consider blocking interrupts to do this cleanly

//	if(TRUE){
//		Counters.serialDebugUnsentCounter++;
//		return;
//	}
	// wrong :
	/* No need for atomic block here as one of two conditions will always be */
	/* true when calling this. Either we have been flagged to receive and    */
	/* decode a packet, or we are in an ISR. In either case it is safe to    */
	/* check the flags and initiate the sequence if they are clear.          */
	//if(RXTXSerialStateFlags & TX_IN_PROGRESS){
		// wrong :
		/* It's OK to return without resetting as it will be done by */
		/* either of those processes if they are underway. The other */
		/* processes are not overridden because they have priority.  */
		//TXBufferInUseFlags = 0;
		//return;
//	}else{ /* Turn off reception */
		/* It's OK to turn this off if nothing was currently being received */
	//	SCI0CR2 &= SCICR2_RX_ISR_DISABLE;
	//	SCI0CR2 &= SCICR2_RX_DISABLE;

		/* Build up the packet */
		/* Set the pointer to the start and init the length */
		TXBufferCurrentPositionHandler = (unsigned char*)&TXBuffer;

		/* Load a protocol with length header into the TX buffer ready for masking */
		*TXBufferCurrentPositionHandler = 0x11;
		TXBufferCurrentPositionHandler++;

		/* Set the payload ID */
		*((unsigned short*)TXBufferCurrentPositionHandler) = asyncDebugInfoPacket;
		TXBufferCurrentPositionHandler += 2;

		/* Store the length location */
		unsigned short* TXLength = (unsigned short*)TXBufferCurrentPositionHandler;
		TXBufferCurrentPositionHandler += 2;

		/* Copy the string into place and record the length copied */
		unsigned short messageLength = stringCopy(TXBufferCurrentPositionHandler, message);
		*TXLength = messageLength;
		TXBufferCurrentPositionHandler += messageLength;

		checksumAndSend();
	//}
}


/* This function should be period limited to about 10 seconds internally (or by scheduler) */
//void checkCountersAndSendErrors(){
	// compare time stamps  with current time stamps and execute if old enough. (if no scheduler)

	// compare counters with counters cache (from last time) sending an error packet when they differ

	// copy counters to counters cache for next time

	// send errors with busy wait on the basis that all errors should be taken care of and not be sent in fairly short order?

	// or send with isr but just busy wait for it to finish before sending the next?

	// debug messages, busy wait or isr or both, perhaps busy wait till able to send, lock sending (need semaphore for this as well as sending one?) and initiate send, then carry on? investigate timeframes for sends of smallish 100byte packets.

	// need to figure out how to queue received packets for processing when we are currently sending stuff out.

	// above notes don't belong here really.
//}


//void prepareDatalog(){
//	// send data log by default otherwise
//	unsigned char chunksExpected = 8; // based on configuration, yet to determine how to calculate this number
//	unsigned char chunksLoaded = 0;
//	if ((!receiving) && (datalogMask & rawVarsMask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	if ((!receiving) && (datalogMask & Mask)) {
//		//
//		chunksLoaded++;
//	}
//	//	set the length
//	//	the pointer should be correct already
//}


/* TODO when implementing, check that ppage is OK!!! */
void sendAckIfRequired(){
	TXBufferInUseFlags = 0;
	// check PPAGE while implementing TODO
}