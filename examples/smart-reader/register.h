//- ----------------------------------------------------------------------------------------------------------------------
//- load libraries -------------------------------------------------------------------------------------------------------
#include <AS.h>                                                         // the asksin framework
#include "hardware.h"                                                   // hardware definition
#include <cmSwitch.h>

//- stage modules --------------------------------------------------------------------------------------------------------
AS hm;                                                                  // asksin framework

extern void initSwitch(uint8_t channel);                                 // declare function to jump in
extern void switchSwitch(uint8_t channel, uint8_t status);               // declare function to jump in

//- ----------------------------------------------------------------------------------------------------------------------
//- eeprom defaults table ------------------------------------------------------------------------------------------------
uint16_t EEMEM eMagicByte;
uint8_t  EEMEM eHMID[3]  = {0x9a,0x45,0x9f,};
uint8_t  EEMEM eHMSR[10] = {'M','Y','S','M','A','R','T','T','T','Y',};
uint8_t  EEMEM eHMKEY[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,};

// if HMID and Serial are not set, then eeprom ones will be used
uint8_t HMID[3] = {0x9a,0x45,0x9f,};
uint8_t HMSR[10] = {'M','Y','S','M','A','R','T','T','T','Y',};          // XMS2345679
uint8_t HMKEY[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,}; 

//- ----------------------------------------------------------------------------------------------------------------------
//- settings of HM device for AS class -----------------------------------------------------------------------------------
const uint8_t devIdnt[] PROGMEM = {
    /* Firmware version  1 byte */  0x01,                               // don't know for what it is good for
    /* Model ID          2 byte */  0x8F,0xFE,                          // model ID, describes HM hardware. Own devices should use high values due to HM starts from 0
    /* Sub Type ID       1 byte */  0x30,                               // not needed for FHEM, it's something like a group ID
    /* Device Info       3 byte */  0x00,0x00,0x00,                     // describes device, not completely clear yet. includes amount of channels
};  // 7 byte

//- ----------------------------------------------------------------------------------------------------------------------
//- channel slice address definition -------------------------------------------------------------------------------------
const uint8_t cnlAddr[] PROGMEM = {
    0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
};
// 0x0a - 0x0c is HMID of master
// 0x0d - 0x10 is delay between smart meter readouts in ms

//- channel device list table --------------------------------------------------------------------------------------------
EE::s_cnlTbl cnlTbl[] = {
    // cnl, lst, sIdx, sLen, pAddr, hidden
    { 0, 0, 0x00, 7, 0x001f, 0, },
    //{ 1, 1, 0x06,  4, 0x0025, 0, },
};  // 21 byte

//- peer device list table -----------------------------------------------------------------------------------------------
EE::s_peerTbl peerTbl[] = {
    // cnl, pMax, pAddr;
    { 0, 0, 0x010d, },
};  // 4 byte

//- handover to AskSin lib -----------------------------------------------------------------------------------------------
EE::s_devDef devDef = {
    1, 1, devIdnt, cnlAddr,
};  // 6 byte

//- module registrar -----------------------------------------------------------------------------------------------------
//RG::s_modTable modTbl[1];

//- ----------------------------------------------------------------------------------------------------------------------
//- first time and regular start functions -------------------------------------------------------------------------------

void everyTimeStart(void) {
    // place here everything which should be done on each start or reset of the device
    // typical usecase are loading default values or user class configurations

    // init the homematic framework
    //hm.confButton.config(2, CONFIG_KEY_PCIE, CONFIG_KEY_INT);           // configure the config button, mode, pci byte and pci bit
    hm.ld.init(2, &hm);                                                 // set the led
    hm.ld.set(welcome);                                                 // show something
    //hm.bt.set(30, 3600000);                                             // set battery check, internal, 2.7 reference, measurement each hour
    hm.pw.setMode(0);                                                   // set power management mode

    // register user modules
    //cmSwitch[0].config(&initSwitch, &switchSwitch);                          // configure user module
}

void firstTimeStart(void) {
    // place here everything which should be done on the first start or after a complete reset of the sketch
    // typical usecase are default values which should be written into the register or peer database

	Serial << F("first time start\n");
	uint8_t list0defaults[] = {
		0x00,0x00,0x00, //  0x0a, Master-ID, leave at 000000 to allow for pairing
		0,0,0xea,0x60             //  0x0d, update energy consupmtion every 60 * 5 seconds
	};
	hm.ee.setList(0,0,0,list0defaults);
}

