/******************************************************************************
SparkFun_Alphanumeric_Display.cpp
SparkFun Alphanumeric Display Library Source File
Priyanka Makin @ SparkFun Electronics
Original Creation Date: February 25, 2020
https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library

Updated May 2, 2020 by Gaston Williams to add defineChar function

Pickup a board here: https://sparkle.sparkfun.com/sparkle/storefront_products/16391

This file implements all functions of the HT16K33 class. Functions here range
from printing to one or more Alphanumeric Displays, changing the display settings, and writing/
reading the RAM of the HT16K33.

The Holtek HT16K33 seems to be susceptible to address changes intra-sketch. The ADR pins
are muxed with the ROW and COM drivers so as semgents are turned on/off that affect
the ADR1/ADR0 pins the address has been seen to change. The best way around this is
to do a isConnected check before updateRAM() is sent to the driver IC.

Development environment specifics:
	IDE: Arduino 1.8.9
	Hardware Platform: Arduino Uno
	Alphanumeric Display Breakout Version: 1.0.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <SparkFun_Alphanumeric_Display.h>

/*--------------------------- Character Map ----------------------------------*/
#define SFE_ALPHANUM_UNKNOWN_CHAR 95

//This is the lookup table of segments for various characters
//For AVR architecture, use PROGMEM
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
static const uint16_t PROGMEM alphanumeric_segs[96]{
#else
static const uint16_t alphanumeric_segs[96]{
#endif
	//nmlkjihgfedcba
	0b00000000000000, //' ' (space)
	0b00001000001000, //'!'  - added to map
	0b00001000000010, //'"' - added to map
 	0b1001101001110,  //'#'
	0b1001101101101,  //'$'
	0b10010000100100, //'%'
	0b110011011001,   //'&'
	0b1000000000,	  //'''
	0b111001,         //'('
	0b1111,           //')'
	0b11111010000000, //'*'
	0b1001101000000,  //'+'
	0b10000000000000, //','
	0b101000000,	  //'-'
	0b00000000000000, //'.' - changed to blank
	0b10010000000000, //'/'
	0b111111,         //'0'
	0b10000000110,	  //'1'
	0b101011011,	  //'2'
	0b101001111,	  //'3'
	0b101100110,	  //'4'
	0b101101101,	  //'5'
	0b101111101,	  //'6'
	0b1010000000001,  //'7'
	0b101111111,	  //'8'
	0b101100111,	  //'9'
	0b00000000000000, //':' - changed to blank
	0b10001000000000, //';'
	0b110000000000,   //'<'
	0b101001000,	  //'='
	0b01000010000000, //'>'
    0b01000100000011, //'?' - Added to map
	0b00001100111011, //'@' - Added to map
	0b101110111,	  //'A'
	0b1001100001111,  //'B'
	0b111001,         //'C'
	0b1001000001111,  //'D'
	0b101111001,	  //'E'
	0b101110001,	  //'F'
	0b100111101,	  //'G'
	0b101110110,	  //'H'
	0b1001000001001,  //'I'
	0b11110,          //'J'
	0b110001110000,   //'K'
	0b111000,         //'L'
	0b10010110110,	  //'M'
	0b100010110110,   //'N'
	0b111111,         //'O'
	0b101110011,	  //'P'
	0b100000111111,   //'Q'
	0b100101110011,   //'R'
	0b110001101,	  //'S'
	0b1001000000001,  //'T'
	0b111110,         //'U'
	0b10010000110000, //'V'
	0b10100000110110, //'W'
	0b10110010000000, //'X'
	0b1010010000000,  //'Y'
	0b10010000001001, //'Z'
	0b111001,         //'['
	0b100010000000,   //'\'
	0b1111,           //']'
    0b10100000000000, //'^' - Added to map
	0b1000,			  //'_'
	0b10000000,		  //'`'
	0b101011111,	  //'a'
	0b100001111000,   //'b'
	0b101011000,	  //'c'
	0b10000100001110, //'d'
	0b1111001,        //'e'
	0b1110001,        //'f'
	0b110001111,	  //'g'
	0b101110100,	  //'h'
	0b1000000000000,  //'i'
	0b1110,           //'j'
	0b1111000000000,  //'k'
	0b1001000000000,  //'l'
	0b1000101010100,  //'m'
	0b100001010000,   //'n'
	0b101011100,	  //'o'
	0b10001110001,	  //'p'
	0b100101100011,   //'q'
	0b1010000,        //'r'
	0b110001101,	  //'s'
	0b1111000,        //'t'
	0b11100,          //'u'
	0b10000000010000, //'v'
	0b10100000010100, //'w'
	0b10110010000000, //'x'
	0b1100001110,	  //'y'
	0b10010000001001, //'z'
	0b10000011001001, //'{'
	0b1001000000000,  //'|'
	0b110100001001,   //'}'
	0b00000101010010, //'~' - Added to map
	0b11111111111111, //Unknown character (DEL or RUBOUT)
};

/*--------------------------- Device Status----------------------------------*/

bool HT16K33::begin(uint8_t addressLeft, uint8_t addressLeftCenter, uint8_t addressRightCenter, uint8_t addressRight, TwoWire &wirePort)
{

	_deviceAddressLeft = addressLeft;				//grab the address of the alphanumeric
	_deviceAddressLeftCenter = addressLeftCenter;   //grab the address of the alphanumeric
	_deviceAddressRightCenter = addressRightCenter; //grab the address of the alphanumeric
	_deviceAddressRight = addressRight;				//grab the address of the alphanumeric

	if (_deviceAddressRight != DEFAULT_NOTHING_ATTACHED)
		numberOfDisplays = 4;
	else if (_deviceAddressRightCenter != DEFAULT_NOTHING_ATTACHED)
		numberOfDisplays = 3;
	else if (_deviceAddressLeftCenter != DEFAULT_NOTHING_ATTACHED)
		numberOfDisplays = 2;
	else
		numberOfDisplays = 1;

	//TODO: malloc more displayRAM

	_i2cPort = &wirePort; //Remember the user's setting

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (isConnected(i) == false)
		{
			//Serial.println("Failed isConnected()");
			return false;
		}
		// if (checkDeviceID(i) == false)
		// {
		// 	Serial.println(i);
		// 	Serial.println("Hello, I've failed checkDeviceID()");
		// 	return false;
		// }
	}

	if (initialize() == false)
	{
		//Serial.println("Failed initialize()");
		return false;
	}

	if (clear() == false) //Clear all displays
	{
		//Serial.println("Failed clear()");
		return false;
	}

	displayContent[4 * 4] = '\0'; //Terminate the array because we are doing direct prints

	return true;
}

//Check that all displays are responding
//The Holtek IC sometimes fails to respond. This attempts multiple times before giving up.
bool HT16K33::isConnected(uint8_t displayNumber)
{
	uint8_t triesBeforeGiveup = 20;

	for (uint8_t x = 0; x < triesBeforeGiveup; x++)
	{
		_i2cPort->beginTransmission(lookUpDisplayAddress(displayNumber));
		if (_i2cPort->endTransmission() == 0)
		{
			// if (x > 0)
			// {
			// 	Serial.print("isConnect successful");
			// 	Serial.print(" after ");
			// 	Serial.print(x);
			// 	Serial.print(" tries");
			// 	Serial.println();
			// }
			return true;
		}

		delay(1);
	}
	return false;
}

bool HT16K33::initialize()
{
	//Turn on system clock of all displays
	if (enableSystemClock() == false)
	{
		//Serial.println("Init: Failed enableSystemClock()");
		return false;
	}

	//Set brightness of all displays to full brightness
	if (setBrightness(16) == false)
	{
		//Serial.println("Init: Failed setBrightness()");
		return false;
	}

	//Blinking set - blinking off
	if (setBlinkRate(ALPHA_BLINK_RATE_NOBLINK) == false)
	{
		//Serial.println("Init: Failed setBlinkRate()");
		return false;
	}

	//Turn on all displays
	if (displayOn() == false)
	{
		//Serial.println("Init: Failed display on");
		return false;
	}

	return true;
}

// //Verify that all objects on I2C bus are alphanumeric displays
// bool HT16K33::checkDeviceID(uint8_t displayNumber)
// {
// 	uint8_t address = lookUpDisplayAddress(displayNumber);
// 	uint8_t test = 0xAA;
// 	uint8_t temp;
// 	//Turn off display - DEBUG: should it be ALL or ONE?
// 	singleDisplayOff(displayNumber);
// 	//Write 0xAA to register 0
// 	writeRAM(address, 0, (uint8_t *)&test, 1);
// 	//Read it back, it should be 0xAA
// 	readRAM(address, 0, (uint8_t *)&temp, sizeof((uint8_t)temp));
// 	if (temp != test)
// 		return false;
// 	//DEBUGGING: taking this out for now... can't call this write to ALL displays when we haven't initalized all displays yet!
// 	//Clear the write we just did
// 	// clear();
// 	//Turn display back on
// 	singleDisplayOn(displayNumber);
// 	return true;
// }

bool HT16K33::enableSystemClock()
{
	bool status = true;
	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (enableSystemClockSingle(i) == false)
			status = false;
	}
	return status;
}

bool HT16K33::disableSystemClock()
{
	bool status = true;
	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (enableSystemClockSingle(i) == false)
			status = false;
	}
	return status;
}

bool HT16K33::enableSystemClockSingle(uint8_t displayNumber)
{
	uint8_t dataToWrite = ALPHA_CMD_SYSTEM_SETUP | 1; //Enable system clock

	bool status = writeRAM(lookUpDisplayAddress(displayNumber), dataToWrite);
	delay(1); //Allow display to start
	return (status);
}

bool HT16K33::disableSystemClockSingle(uint8_t displayNumber)
{
	uint8_t dataToWrite = ALPHA_CMD_SYSTEM_SETUP | 0; //Standby mode

	return (writeRAM(lookUpDisplayAddress(displayNumber), dataToWrite));
}

uint8_t HT16K33::lookUpDisplayAddress(uint8_t displayNumber)
{
	switch (displayNumber)
	{
	case 0:
		return _deviceAddressLeft;
		break;
	case 1:
		return _deviceAddressLeftCenter;
		break;
	case 2:
		return _deviceAddressRightCenter;
		break;
	case 3:
		return _deviceAddressRight;
		break;
	}

	return 0; //We shouldn't get here
}

/*-------------------------- Display configuration functions ---------------------------*/

bool HT16K33::clear()
{
	//Clear the displayRAM array
	for (uint8_t i = 0; i < 16 * numberOfDisplays; i++)
		displayRAM[i] = 0;

	digitPosition = 0;

	return (updateDisplay());
}

//Duty valid between 1 and 16
bool HT16K33::setBrightness(uint8_t duty)
{
	bool status = true;
	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (setBrightnessSingle(i, duty) == false)
			status = false;
	}
	return status;
}

bool HT16K33::setBrightnessSingle(uint8_t displayNumber, uint8_t duty)
{
	if (duty > 15)
		duty = 15; //Error check

	uint8_t dataToWrite = ALPHA_CMD_DIMMING_SETUP | duty;
	return (writeRAM(lookUpDisplayAddress(displayNumber), dataToWrite));
}

//Parameter "rate" in Hz
//Valid options for "rate" are defined by datasheet: 2, 1, or 0.5 Hz
//Any other input to this function will result in steady alphanumeric display
bool HT16K33::setBlinkRate(float rate)
{
	bool status = true;
	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (setBlinkRateSingle(i, rate) == false)
			status = false;
	}
	return status;
}

bool HT16K33::setBlinkRateSingle(uint8_t displayNumber, float rate)
{
	if (rate == 2)
	{
		blinkRate = ALPHA_BLINK_RATE_2HZ;
	}
	else if (rate == 1)
	{
		blinkRate = ALPHA_BLINK_RATE_1HZ;
	}
	else if (rate == 0.5)
	{
		blinkRate = ALPHA_BLINK_RATE_0_5HZ;
	}
	//default to no blink
	else
	{
		blinkRate = ALPHA_BLINK_RATE_NOBLINK;
	}

	uint8_t dataToWrite = ALPHA_CMD_DISPLAY_SETUP | (blinkRate << 1) | displayOnOff;
	return (writeRAM(lookUpDisplayAddress(displayNumber), dataToWrite));
}

bool HT16K33::displayOnSingle(uint8_t displayNumber)
{
	return setDisplayOnOff(displayNumber, true);
}
bool HT16K33::displayOffSingle(uint8_t displayNumber)
{
	return setDisplayOnOff(displayNumber, false);
}

//Set or clear the display on/off bit of a given display number
bool HT16K33::setDisplayOnOff(uint8_t displayNumber, bool turnOnDisplay)
{
	if (turnOnDisplay == true)
		displayOnOff = ALPHA_DISPLAY_ON;
	else
		displayOnOff = ALPHA_DISPLAY_OFF;

	uint8_t dataToWrite = ALPHA_CMD_DISPLAY_SETUP | (blinkRate << 1) | displayOnOff;
	return (writeRAM(lookUpDisplayAddress(displayNumber), dataToWrite));
}

//Turn on/off the entire display
bool HT16K33::displayOn()
{
	bool status = true;

	displayOnOff = ALPHA_DISPLAY_ON;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (displayOnSingle(i) == false)
			status = false;
	}

	return status;
}

bool HT16K33::displayOff()
{
	bool status = true;

	displayOnOff = ALPHA_DISPLAY_OFF;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (displayOffSingle(i) == false)
			status = false;
	}

	return status;
}

bool HT16K33::decimalOnSingle(uint8_t displayNumber)
{
	return setDecimalOnOff(displayNumber, true);
}

bool HT16K33::decimalOffSingle(uint8_t displayNumber)
{
	return setDecimalOnOff(displayNumber, false);
}

bool HT16K33::setDecimalOnOff(uint8_t displayNumber, bool turnOnDecimal)
{
	uint8_t adr = 0x03;
	uint8_t dat;

	if (turnOnDecimal == true)
	{
		decimalOnOff = ALPHA_DECIMAL_ON;
		dat = 0x01;
	}
	else
	{
		decimalOnOff = ALPHA_DECIMAL_OFF;
		dat = 0x00;
	}

	displayRAM[adr + displayNumber * 16] = displayRAM[adr + displayNumber * 16] | dat;
	return (updateDisplay());
}

//Turn on/off the entire display
bool HT16K33::decimalOn()
{
	bool status = true;

	decimalOnOff = ALPHA_DECIMAL_ON;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (decimalOnSingle(i) == false)
			status = false;
	}

	//Serial.println(status);
	return status;
}

bool HT16K33::decimalOff()
{
	bool status = true;

	decimalOnOff = ALPHA_DECIMAL_OFF;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (decimalOffSingle(i) == false)
			status = false;
	}
	return status;
}

bool HT16K33::colonOnSingle(uint8_t displayNumber)
{
	return setColonOnOff(displayNumber, true);
}

bool HT16K33::colonOffSingle(uint8_t displayNumber)
{
	return setColonOnOff(displayNumber, false);
}

bool HT16K33::setColonOnOff(uint8_t displayNumber, bool turnOnColon)
{
	uint8_t adr = 0x01;
	uint8_t dat;

	if (turnOnColon == true)
	{
		colonOnOff = ALPHA_COLON_ON;
		dat = 0x01;
	}
	else
	{
		colonOnOff = ALPHA_COLON_OFF;
		dat = 0x00;
	}

	displayRAM[adr + displayNumber * 16] = displayRAM[adr + displayNumber * 16] | dat;
	return (updateDisplay());
}

bool HT16K33::colonOn()
{
	bool status = true;

	colonOnOff = ALPHA_COLON_ON;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (colonOnSingle(i) == false)
			status = false;
	}
	return status;
}

bool HT16K33::colonOff()
{
	bool status = true;

	colonOnOff = ALPHA_COLON_OFF;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (colonOffSingle(i) == false)
			status = false;
	}
	return status;
}

/*---------------------------- Light up functions ---------------------------------*/

//Given a segment and a digit, set the matching bit within the RAM of the Holtek RAM set
void HT16K33::illuminateSegment(uint8_t segment, uint8_t digit)
{
	uint8_t com;
	uint8_t row;

	com = segment - 'A'; //Convert the segment letter back to a number

	if (com > 6)
		com -= 7;
	if (segment == 'I')
		com = 0;
	if (segment == 'H')
		com = 1;

	row = digit % 4; //Convert digit (1 to 16) back to a relative position on a given display
	if (segment > 'G')
		row += 4;

	uint8_t offset = digit / 4 * 16;
	uint8_t adr = com * 2 + offset;

	//Determine the address
	if (row > 7)
		adr++;

	//Determine the data bit
	if (row > 7)
		row -= 8;
	uint8_t dat = 1 << row;

	//Temp DEBUGGING - clear segments that might affect A0/A1
	//dat &= 0b11111100;

	// Serial.print("illSeg Digit: ");
	// Serial.print(digit);
	// Serial.print("\t row: ");
	// Serial.print(row);
	// Serial.print("\t com: ");
	// Serial.print(com);
	// Serial.print("\t adr: ");
	// Serial.print(adr);
	// Serial.println();

	displayRAM[adr] = displayRAM[adr] | dat;
}

//Given a binary set of segments and a digit, store this data into the RAM array
void HT16K33::illuminateChar(uint16_t segmentsToTurnOn, uint8_t digit)
{
	for (uint8_t i = 0; i < 14; i++) //There are 14 segments on this display
	{
		if ((segmentsToTurnOn >> i) & 0b1)
			illuminateSegment('A' + i, digit); //Convert the segment number to a letter
	}
}

//Show a character on display
void HT16K33::printChar(uint8_t displayChar, uint8_t digit)
{
	//moved alphanumeric_segs array to PROGMEM
	uint16_t characterPosition = 65532;

	//space
	if (displayChar == ' ')
		characterPosition = 0;
	//Printable Symbols
	else if (displayChar >= '!' && displayChar <= '~')
	{
		characterPosition = displayChar - '!' + 1;
	}

	uint8_t dispNum = digitPosition / 4;
	//Take care of special characters
	if (characterPosition == 14) //'.'
		decimalOnSingle(dispNum);
	if (characterPosition == 26) //':'
		colonOnSingle(dispNum);
	if (characterPosition == 65532) //unknown character
		characterPosition = SFE_ALPHANUM_UNKNOWN_CHAR;

	// //Error check
	// if (characterPosition > sizeof(alphanumeric_segs))
	// 	characterPosition = sizeof(alphanumeric_segs) - 1; //Unknown char

	uint16_t segmentsToTurnOn = getSegmentsToTurnOn(characterPosition);

	illuminateChar(segmentsToTurnOn, digit);
}

//Update the list to define a new segments display for a particular character
bool HT16K33::defineChar(uint8_t displayChar, uint16_t segmentsToTurnOn)
{
	bool result = false;

	//Check to see if character is within range of displayable ASCII characters
	if (displayChar >= '!' && displayChar <= '~')
	{
	  //Get the index of character in table and update its 14-bit segment value
	  uint16_t characterPosition = displayChar - '!' + 1;

      //Create a new character definition
      struct CharDef * pNewCharDef = (CharDef *)calloc(1, sizeof(CharDef));

	  //Set the position to the table index
      pNewCharDef -> position = characterPosition;
      //Mask the segment value to 14 bits only
      pNewCharDef -> segments = segmentsToTurnOn & 0x3FFF;
      //New definition always goes at the end of the list
      pNewCharDef -> next = NULL;

      //If list is empty set it to the new item
      if (pCharDefList == NULL)
      {
	    pCharDefList = pNewCharDef;
      }
      else
      {
      //Otherwise go to the end of the list and add it there
      struct CharDef * pTail = pCharDefList;

      while(pTail->next != NULL)
      {
        pTail = pTail->next;
      }

      pTail->next = pNewCharDef;
    }
	//We added the definition so we're all good
	result = true;
  }
  return result;
}

//Get the character map from the definition list or default table
uint16_t HT16K33::getSegmentsToTurnOn(uint8_t charPos)
{
  uint16_t segments = 0;
  //pointer to a defined character in list
  struct CharDef * pDefChar = pCharDefList;

  //Search the chacters list for a match
  while(pDefChar && (pDefChar->position != charPos))
  {
    pDefChar = pDefChar -> next;
  }

  //If we found a match return that value
  if (pDefChar != NULL)
  {
    segments = pDefChar -> segments;
  }
  //Otherwise get the value from the table
  else
  {
    segments = pgm_read_word_near(alphanumeric_segs + charPos);
  }
  return segments;
}

/*
 * Write a byte to the display.
 * Required for Print.
 */
size_t HT16K33::write(uint8_t b)
{
	//If user wants to print '.' or ':', don't increment the digitPosition!
	if (b == '.' | b == ':')
		printChar(b, 0);
	else
	{
		printChar(b, digitPosition++);
		digitPosition %= (numberOfDisplays * 4); //Convert displays to number of digits
	}

	return (updateDisplay()); //Send RAM buffer over I2C bus
}

/*
 * Write a character buffer to the display.
 * Required for Print.
 */
size_t HT16K33::write(const uint8_t *buffer, size_t size)
{
	size_t n = size;
	uint8_t buff;

	//Clear the displayRAM array
	for (uint8_t i = 0; i < 16 * numberOfDisplays; i++)
		displayRAM[i] = 0;

	digitPosition = 0;

	while (size--)
	{
		buff = *buffer++;
		//For special characters like '.' or ':', do not increment the digitPosition
		if (buff == '.')
			printChar('.', 0);
		else if (buff == ':')
			printChar(':', 0);
		else
		{
			printChar(buff, digitPosition);
			displayContent[digitPosition] = buff; //Record to internal array

			digitPosition++;
			digitPosition %= (numberOfDisplays * 4);
		}
	}

	updateDisplay(); //Send RAM buffer over I2C bus

	return n;
}

//Write a string to the display
size_t HT16K33::write(const char *str)
{
	if (str == NULL)
		return 0;
	return write((const uint8_t *)str, strlen(str));
}

//Push the contents of displayRAM out to the various displays in 16 byte chunks
bool HT16K33::updateDisplay()
{
	//printRAM();

	bool status = true;

	for (uint8_t i = 0; i < numberOfDisplays; i++)
	{
		if (writeRAM(lookUpDisplayAddress(i), 0, (uint8_t *)(displayRAM + (i * 16)), 16) == false)
		{
			//Serial.print("updateDisplay fail at display 0x");
			//Serial.println(lookUpDisplayAddress(i), HEX);
			status = false;
		}
	}

	return status;
}

//Shift the display content to the right one digit
bool HT16K33::shiftRight(uint8_t shiftAmt)
{
	for (uint8_t x = (4 * numberOfDisplays) - shiftAmt; x >= shiftAmt; x--)
	{
		displayContent[x] = displayContent[x - shiftAmt];
	}

	//Clear the leading characters
	for (uint8_t x = 0; x < shiftAmt; x++)
	{
		if (x + shiftAmt > (4 * numberOfDisplays))
			break; //Error check

		displayContent[0 + x] = ' ';
	}

	return (print(displayContent));
}

//Shift the display content to the left one digit
bool HT16K33::shiftLeft(uint8_t shiftAmt)
{
	for (int x = 0; x < 4 * numberOfDisplays; x++)
	{
		if (x + shiftAmt > (4 * numberOfDisplays))
			break; //Error check
		displayContent[x] = displayContent[x + shiftAmt];
	}

	//Clear the trailing characters
	for (int x = 0; x < shiftAmt; x++)
	{
		if (4 * numberOfDisplays - 1 - x < 0)
			break; //Error check

		displayContent[4 * numberOfDisplays - 1 - x] = ' ';
	}

	return (print(displayContent));
}

/*----------------------- Internal I2C Abstraction -----------------------------*/

bool HT16K33::readRAM(uint8_t address, uint8_t reg, uint8_t *buff, uint8_t buffSize)
{
	uint8_t displayNum = 0;
	if (address == _deviceAddressLeftCenter)
		displayNum = 1;
	else if (address == _deviceAddressRightCenter)
		displayNum = 2;
	else if (address == _deviceAddressRight)
		displayNum = 3;
	isConnected(displayNum); //Wait until display is ready

	_i2cPort->beginTransmission(address);
	_i2cPort->write(reg);
	_i2cPort->endTransmission(false);

	if (_i2cPort->requestFrom(address, buffSize) > 0)
	{
		for (uint8_t i = 0; i < buffSize; i++)
			buff[i] = _i2cPort->read();
		return true;
	}

	return false;
}

// //Overloaded function declaration
// //Use when reading just one byte of data
// bool HT16K33::read(uint8_t reg, uint8_t data)
// {
// 	return (read(reg, (uint8_t *)&data, (uint8_t)sizeof(data)));
// }

//Write the contents of the RAM array out to the Holtek IC
//After much testing, it
bool HT16K33::writeRAM(uint8_t address, uint8_t reg, uint8_t *buff, uint8_t buffSize)
{
	uint8_t displayNum = 0;
	if (address == _deviceAddressLeftCenter)
		displayNum = 1;
	else if (address == _deviceAddressRightCenter)
		displayNum = 2;
	else if (address == _deviceAddressRight)
		displayNum = 3;
	//isConnected(displayNum); //Wait until display is ready

	_i2cPort->beginTransmission(address);
	_i2cPort->write(reg);

	for (uint8_t i = 0; i < buffSize; i++)
		_i2cPort->write(buff[i]);

	if (_i2cPort->endTransmission() == 0)
		return true;

	return false;
}

//Write a single byte to the display. This is often a command byte.
//The address of the data to write is contained in the first four bits of dataToWrite
bool HT16K33::writeRAM(uint8_t address, uint8_t dataToWrite)
{
	uint8_t temp = 0;
	return (writeRAM(address, dataToWrite, (uint8_t *)&temp, 0));
}
