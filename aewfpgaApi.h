// Update History:
//	No    DATE			Version		Reason
//	--  ---------   ------  ---------------------------------------------------------------------
//  1. 2012/09/25		3.1.2		Add CryptoMemory AT88SCXXXX.
//  2. 2013/01/01 	3.1.9 	Add pci interface. (Implement WinDriver) 
//	3. 2013/04/10		3.3.0		Add "ExtSRAM" Function. 
// 		
// ********************************************************************************************************
//
#ifndef __AEWFPGA_API_H__
#define __AEWFPGA_API_H__

//FPGA Information Structure
typedef struct
{
	char szManufactureID[5+1];		// 00000~00004	Manufacture ID
	char szCustomerID[8+1];			// 00005~0000C	Customer ID
	char szModuleName[7+1];			// 0000D~00013	Module Name
	unsigned int szVersionNumber[3];	// 00014~00016	Version Number
	unsigned char szBuildtime[8+1];		// 00017~0001E	Buildtime
	int szGPIOdir[5];			// 0001F~00023	GPIO output define
	unsigned int szAPIVersion[3];		// API Version
	char szFPGADriverVerion[12];		// FPGA Driver
}	FPGAInfo;

// ============================================================================
// "General Output" Function (GPO)
// ============================================================================
bool _GetFPGAOCount(unsigned char *ucCount);
bool _SetFPGAOIdx(unsigned long uIdx, bool bFlag);
bool _SetFPGAOAll(bool bFlag);
bool _SetFPGAOutput(unsigned long ulData1,unsigned long ulData2);
bool _GetFPGAOIdx(unsigned long uIdx, bool *bFlag);
bool _GetFPGAOutput(unsigned long* ulData1,unsigned long* ulData2);

// ============================================================================
// "General Input" Function (GPI)
// ============================================================================
bool _GetFPGAICount(unsigned char *ucCount);
bool _GetFPGAIIdx(unsigned long uIdx, bool *bFlag);
bool _GetFPGAInput(unsigned long* ulData1,unsigned long* ulData2);
// Set debounce time
bool _SetFPGAIHWDebTime(unsigned long uIdxGroup, unsigned long time);

// ============================================================================
// "INFORMATION" Function
// ============================================================================
bool _GetFPGAInfor(FPGAInfo * pInfo);

/*
// ****************************************************************************
// "DOOR" FUNCTION
// ****************************************************************************
bool _GetFPGADoorCount(unsigned char *ucCount);
bool _GetFPGADoorIdx(unsigned long uIdx, bool *bFlag);					
bool _GetFPGADoor(unsigned short* usData);			
bool _SetFPGADoorHWDebTime(unsigned long time);
*/

// ============================================================================
// "FLASH/RAM ACCESS" Function
// ============================================================================
bool _RamWriteByte(unsigned long dwOffset, unsigned char ucData);
bool _RamReadByte(unsigned long dwOffset, unsigned char* ucData);
bool _GetRamSize(unsigned long* ulSize);

// NVRam mirror function.
bool _GetRamMirrorEn(bool *bFlag);
bool _SetRamMirrorRange(unsigned char ucMirrorMode, unsigned long ulAddrS, unsigned long ulAddrE);
bool _SetRamMirrorStart();
bool _GetRamMirrorIsFinished(bool *bFlag);

// GA5010
bool _RamNoEnable(bool *bEnable);
bool _GetRamNo(unsigned char *ucRamNo, unsigned char *ucPerRamSize);
bool _SetRamNo(unsigned char ucRamNo, unsigned char ucPerRamSize);
bool _GetRamSizeInfo(unsigned char *ucRamCount, unsigned char *ucPerRamSize);

bool _FlashEraseAll();
bool _FlashEraseSector(unsigned long ulSector);
bool _FlashWriteByte(unsigned long dwOffset, unsigned char ucData);
bool _FlashReadByte(unsigned long dwOffset, unsigned char* ucData);
bool _GetFlashSize(unsigned long* ulSize);

// ============================================================================
// "WDT" Functions
// ============================================================================
bool _SetWDTEnable(bool bFlag);
bool _SetWDTReset(bool bFlag);
bool _GetWDTCountValue(unsigned short *usCount);
bool _SetWDTCountValue(unsigned short usCount);

// ============================================================================
// "Power Off Test" Functions
// ============================================================================
bool _GetPWOffIrqEn(bool *bFlag);
bool _GetPWOffIrqValue(bool *bFlag);

// ============================================================================
// "EEPROM" Function
// ============================================================================
bool _GetEEPRomSize(unsigned long * ulSize);
bool _EEPRomWrite( unsigned char StartAddr, unsigned char * ucBuffer, unsigned long ulLen);
bool _EEPRomRead( unsigned char StartAddr, unsigned char * ucbuffer, unsigned long ullen);

// ============================================================================
// "RANDOM" Function
// ============================================================================
bool _GetRandomCount(unsigned char *ucCount);	// Get random counts which hardware support.
bool _GetRandom(unsigned long uIdx, unsigned long* ulData);	// Get hardware random number from uIdx.

// ============================================================================
// "IRQ" Function
// ============================================================================
// PI Enable Register
bool _GetIrqPICount(unsigned char *ucCount);
bool _SetIrqPIEnableIdx(unsigned long uIdx, bool bFlag);		
bool _SetIrqPIEnableAll(bool bFlag); 				
bool _SetIrqPIEnable(unsigned long ulData1,unsigned long ulData2);   
bool _GetIrqPIEnableIdx(unsigned long uIdx, bool *bFlag);				
bool _GetIrqPIEnable(unsigned long* ulData1,unsigned long* ulData2);

// PI Enable Register
bool _SetIrqPISettingIdx(unsigned long uIdx, bool bFlag);			
bool _SetIrqPISettingAll(bool bFlag);							
bool _SetIrqPISetting(unsigned long ulData1,unsigned long ulData2);	
bool _GetIrqPISettingIdx(unsigned long uIdx, bool *bFlag);				
bool _GetIrqPISetting(unsigned long* ulData1,unsigned long* ulData2);

// Counter
bool _GetIrqPICounter(unsigned long uIdx, unsigned long *ulCount);
bool _GetIrqPICounterPack(unsigned long * ulCounterBuff, unsigned long ulLen);
//GA5010
//bool _GetIrqDRCounter(unsigned long uIdx, unsigned long *ulCount);
//bool _GetIrqDRCounterPack(unsigned long * ulCounterBuff, unsigned long ulLen);
bool _GetTimerCounter(unsigned long uIdx, unsigned long *ulCount);
bool _GetTimerCounterPack(unsigned long * ulCounterBuff, unsigned long ulLen);

// ============================================================================
// "Timer" Function
// ============================================================================
bool _GetTimerCount(unsigned char *ucCount);
bool _GetAutoTimerCount(unsigned char *ucCount);
bool _SetAutoTimerEnableIdx(unsigned long uIdx, bool bFlag);
bool _GetAutoTimerEnableIdx(unsigned long uIdx, bool *bFlag);
bool _SetTimerEnableIdx(unsigned long uIdx, bool bFlag);
bool _GetTimerEnableIdx(unsigned long uIdx, bool *bFlag);
bool _SetTimerIdx(unsigned long uIdx, bool bReload, unsigned long ulCount, unsigned long ulInterval);
bool _GetTimerIdx(unsigned long uIdx, bool *bReload, unsigned long *ulCount, unsigned long *ulInterval);

// ****************************************************************************
// "INIT" FUNCTION
// ****************************************************************************

bool _FPGAInit(unsigned int*, void*);
bool _FPGAUnInit(void);

// =============================================================================================
// SPR : "SPECIALL IO" Function - Attribute value  '0'(false): output, '1'(true): input (default)
// =============================================================================================
bool _GetFPGASPRCount(unsigned char *ucCount);				// Valid SPR port counts. 

// SPR I/O attribute setting functions. 
bool _GetFPGASPRIdxAtt(unsigned long uIdx, bool *bFlag);	// Get bFlag(I/O) from uIdx position of Attribute.
bool _GetFPGASPRAtt(unsigned short *usData);				// Get usData from 0~31 position of Attribute.
bool _SetFPGASPRIdxAtt(unsigned long uIdx, bool bFlag);		// Set bFlag(I/O) to uIdx position of Attribute.
bool _SetFPGASPRAllIdxAtt(bool bFlag);						// Set bFlag(I/O) to all position of Attribute. 
bool _SetFPGASPRAtt(unsigned short usData);					// Set usData to 0~31 position of Attribute.

// SPR value functions.
bool _GetFPGASPRIdxData(unsigned long uIdx, bool *bFlag);	// Get bFlag from uIdx position of Data.
bool _GetFPGASPRData(unsigned short *usData);				// Get usData from 0~31 position of Data.
bool _SetFPGASPRIdxData(unsigned long ulIdx, bool bFlag);	// Set bFlag to uIdx position of Data. 
bool _SetFPGASPRAllIdxData(bool bFlag);						// Set bFlag to all position of Data.
bool _SetFPGASPRData(unsigned short usData);					// Set usData to 0~31 position of Data.

// ==========================================================
//       Intrusion functions.
// ==========================================================

typedef struct _SYSTEMTIME {
  unsigned short wYear;
  unsigned short wMonth;
  unsigned short wDayOfWeek;
  unsigned short wDay;
  unsigned short wHour;
  unsigned short wMinute;
  unsigned short wSecond;
  unsigned short wMilliseconds;
}SYSTEMTIME, *PSYSTEMTIME;

struct _INTRUSION_LOG
{
	int iEventID;
	SYSTEMTIME st;
};

struct _INTRUSION_INFOR
{
//	char szSWVersionNo[10];
	char szFWVersionNo[10];
	char szBuildTime[11];
};

// Define Intrusion event code
#define INTR_ENABLE 0x0
#define INTR_RESET 0x1
#define INTR_CLEAR_BUF 0x2
#define INTR_LOAD_BUF 0x3
#define INTR_GET_VER 0x4

// Define Intrusion Log Event Table.
#define INTR_LOG_VER_NO 0x22 //< Never Use in 2011 for Intru. Door Status> 

#define INTR_BLOCK_BYTES 0x20
#define INTR_1stLOG_INDEX 2
#define INTR_VERLOG_INDEX 1


bool _IntrGetLogCount(unsigned char * ucCount);
bool _IntrInit();
bool _IntrUnInit();
bool _IntrResetTime(SYSTEMTIME st);
bool _IntrReadTime(SYSTEMTIME * pst);
bool _IntrLoadLog();
bool _IntrClearLog();
bool _IntrCheckLog(unsigned long uIdx, struct _INTRUSION_LOG * pLog);
bool _IntrGetInfor(struct _INTRUSION_INFOR * pInfor);

bool _IntrGetIrqEn(bool *bFlag);
bool _IntrGetIrqOut(unsigned long *ulIrqOut);

bool _IntrGetDoorCount(unsigned char * ucCount);
bool _IntrGetDoorStatus(unsigned long ulIdex,  bool *bFlag);

// ===================================================================
// "RTC" FUNCTION
// ===================================================================
typedef struct
{
	unsigned short Year;			// 00000	Year (00~99)
	unsigned short Month;			// 00002	Month (1~12)
	unsigned short day;				// 00004	Day (1~7)
	unsigned short date;			// 00006	Date (1~31)
	unsigned short hour;			// 00008	Hour (00~23)
	unsigned short Min;				// 00010	Minute (00~59)
	unsigned short Sec;				// 00012	Second (00~59)
	unsigned short Century;			// 00014	century(0~1) 
}	FPGA_RTC, *pFPGA_RTC;

bool _GetFPGARTCStatus(bool *bWriteFlag);
bool _GetFPGARTC(FPGA_RTC *RTCTime);
bool _SetFPGARTC(FPGA_RTC RTCTime);

// ****************************************************************************
// "Enable" FUNCTION
// ****************************************************************************
bool _FlashEnable(bool *bFlag);
bool _EEPROMEnable(bool *bFlag);
bool _IntrusionEnable(bool *bFlag);
bool _PowerOffEnable(bool *bFlag);
bool _DoorEnable(bool *bFlag);
bool _FameGEnable(bool *bFlag);
bool _RTCEnable(bool *bFlag);
bool _WDTEnable(bool *bFlag);
bool _SPREnable(bool *bFlag);
bool _AutoTimerEnable(bool *bFlag);
bool _TimerEnable(bool *bFlag);
bool _RandomEnable(bool *bFlag);
bool _MirrorEnable(bool *bFlag);

// GA5010
// ****************************************************************************
// "SIO" FUNCTION
// ****************************************************************************
//bool _SetSIO(unsigned char ucData);
//bool _GetSIO(unsigned char * ucData);

// ============================================================================
//  "FameG" Function
// ============================================================================
//bool _FameGCommTest(unsigned char SlvAdr);
//bool _FameGInit(unsigned char SlvAdr, unsigned char* KEY);
//bool _FameGAuthen(unsigned long MessageAddress, LPCWSTR filename);

// ============================================================================
// 2012/09/25 Add "CryptoMemory AT88SCXXXX" Function
// ============================================================================
bool _CryptoMemEnable();	
bool _CryptoMemWrite(unsigned char * ucCommandBuf, unsigned char ucCommandLen, unsigned char * ucDataBuf, unsigned char ucDataLen);

//Define pulse mode.
#define	PULSE_MODE_GPO  0x0
#define	PULSE_MODE_HLH  0x1
#define	PULSE_MODE_LHL  0x3

// ============================================================================
// Add "Pulse" Function
// ============================================================================
// Modify Pulse functions.
// Define Pulse type code:
//	01 : Signal generator.(For GA2200)
//	02 : Pulse Counter.(For GA5010)
bool _PulseEnable(bool *bEnable);
bool _GetPulseCount(unsigned char *ucCount);
bool _SetPulseIrqEn(bool bFlag);
bool _SetPulseSwitchCode(unsigned char ucSwitchCode);
bool _AddPulseCounter(unsigned long ulIdx, unsigned char ucCounter);
bool _SetPulseFrequence(unsigned long ulIdx, unsigned char ucFrequence);
bool _GetPulseUnsentCounter(unsigned long ulIdx, unsigned char *ucCounter);
bool _GetPulseTypeCode(unsigned char *ucPulseType);
bool _SetPulseMode(unsigned char ucPulseMode);

// ============================================================================
// Add "SerialOut" Function for ISA interface
// ============================================================================
bool _SerialOutputEnable(bool *bEnable);
bool _SetSerialOutputSwitchCode(unsigned char ucSwitchCode);
bool _SetSerialOutput(unsigned char ucSerialData);
bool _GetSerialOutputIsFinished(bool *bFlag);

// ============================================================================
// 2012/11/22 Add "Lamp" Function.
// ============================================================================
bool _LampEnable(bool *bEnable);
// Flash function.
bool _GetLampFlashIdxEn(unsigned long ulIndex, bool *bFlag);
bool _SetLampFlashIdxEn(unsigned long ulIndex, bool bFlag);
bool _GetLampFlashEn(unsigned short *usLampFlashEn);
bool _SetLampFlashEn(unsigned short usLampFlashEn);
bool _GetLampFlashValue(unsigned char *ucLampFlashValue);
bool _SetLampFlashValue(unsigned char ucLampFlashValue);
// Brightness
bool _GetLampBrightnessIdxEn(unsigned long ulIndex, bool *bFlag);
bool _SetLampBrightnessIdxEn(unsigned long ulIndex, bool bFlag);
bool _GetLampBrightnessEn(unsigned short *usLampBrightnessEn);
bool _SetLampBrightnessEn(unsigned short usLampBrightnessEn);
bool _GetLampBrightnessValue(unsigned char *ucLampBrightnessValue);
bool _SetLampBrightnessValue(unsigned char ucLampBrightnessValue);
// To control brightness of PWM0~ PWM9 separately 
bool _GetIndividLampBrightnessIdxEn(unsigned long ulIndex, bool *bFlag);
bool _SetIndividLampBrightnessIdxEn(unsigned long ulIndex, bool bFlag);
bool _GetIndividLampBrightnessEn(unsigned short *ucLampBrightnessEn);
bool _SetIndividLampBrightnessEn(unsigned short ucLampBrightnessEn);
bool _GetIndividLampBrightnessIdxValue(unsigned long ulIndex, unsigned char *ucLampBrightnessValue);
bool _SetIndividLampBrightnessIdxValue(unsigned long ulIndex, unsigned char ucLampBrightnessValue);

bool _GetIndividLampBrightnessCount(unsigned char *ucCount);

// ============================================================================
// 2012/12/13 Add Hopper Function
// ============================================================================
bool _HopperEnable(bool *bEnable);
bool _GetHopperCount(unsigned char *ucCount);
bool _SetHopperPulseMode(unsigned long ulIndex, unsigned char ucPulseMode);
bool _ResetHopper(unsigned long ulIndex, unsigned char ucReset);
bool _GetHopperState(unsigned long ulIndex, unsigned char *ucState);
bool _SetHopperCounter(unsigned long ulIndex, unsigned short usCounter);
bool _GetHopperCounter(unsigned long ulIndex, unsigned short *usCounter);
bool _SetHopperReStart(unsigned long ulIndex);
bool _SetHopperPause(unsigned long ulIndex);
bool _GetIrqHopperEnableIdx(unsigned long uIdx, bool *bFlag);
bool _SetIrqHopperEnableIdx(unsigned long uIdx, bool bFlag);
bool _GetIrqHopperCounter(unsigned long ulIndex, unsigned long *ulCounter);
bool _GetIrqHopperCounterPack(unsigned long * ulCounterBuff, unsigned long ulLen);

// Define Hopper code
// 1. Reset mode
#define	ResetHopperAll			0x1
#define	ResetHopperError		0x2
// 2. State
#define	StartHopper				0x1
#define	RestartHopper			0x1
#define HopperInProcessing		0x1
#define	HopperIsFinished		0x2
#define	HopperIsPause			0x3
#define	HopperCoinInsuff		0x11
#define	HopperCoinJammed		0x21

// ==================================================================
// 2012/12/13 Add HPMotorOut functions.
// ==================================================================
bool _HPMotorOut(bool *bEnable);
bool _GetHPMotorOutCount(unsigned char *ucCount);
bool _GetHPMotorOutIdx(unsigned long ulIndex, bool *bFlag);
bool _SetHPMotorOutIdx(unsigned long ulIndex, bool bFlag);

// ============================================================================
// 2013/04/10 Add "ExtSRAM" Function
// ============================================================================
bool _ExtSRAMEnable(bool *bEnable);	
bool _GetExtSRAMSizeInfo(unsigned char *ucRamCount, unsigned char *ucPerRamSize);
bool _GetExtSRAMSize(unsigned long * ulSize);
bool _ExtSRAMWriteByte(unsigned long dwOffset, unsigned char ucData);
bool _ExtSRAMReadByte(unsigned long dwOffset, unsigned char * ucData);	


#endif					/* __AEWFPGA_API_H__ */

