//********************************************************
// History :
//   Date	Ver	Description
// ----------	-----	------------------------
// 2013/01/01 	3.1.9 	Add pci interface. (Implement WinDriver)
// 2013/02/23		3.1.10	Debug - Just show valid counter for gc_intr_input_get_times_all. 
// 2013/02/25 	3.1.11	Debug HPMotorout count. 
// 2013/03/26		3.1.12	Add EEPROM error byte writing.
// 2013/03/28   3.1.13  1.Debug - Reset function list enable flag of SerialOutput and random number.
//											2.Debug - serialoutput interface.  
//											3.Debug - The range of gc_sram_write_chk() has to match with gc_sram_write().
// 2013/04/10 	3.3.0		1. Add test area setting item for all memory.(SRAM, flash and EXT SRAM)
//											2. Add "ExtSRAM" Functions.
//
//********************************************************
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>           /* time() */
#include <sys/stat.h>   /* struct stat */
#include <fcntl.h>

#include "aewfpgaApi.h"

//static unsigned int gc_um;
//static unsigned int gc_max;

static int accSize = -1;
#define MAXLISTITEM     200
#define MAXINTRIRQDATA  16

static struct _INTRUSION_LOG intrLog[MAXLISTITEM+1];

char * event_string(int event_um)
{
    char* eventString = "NONE";
    
    switch (event_um) {
        case 0x01:
            eventString = "PWR_RESET";
            break;
        case 0x02:
            eventString = "HARD_RESET";
            break;
        case 0x03:
            eventString = "BOUT_PREST";
            break;
        case 0x04:
            eventString = "WD_RESET";
            break;
        case 0x10:
            eventString = "DR0_ON";
            break;
        case 0x11:
            eventString = "DR1_ON";
            break;
        case 0x12:
            eventString = "DR2_ON";
            break;
        case 0x13:
            eventString = "DR3_ON";
            break;
        case 0x14:
            eventString = "DR4_ON";
            break;
        case 0x15:
            eventString = "DR5_ON";
            break;
        case 0x16:
            eventString = "DR6_ON";
            break;
        case 0x17:
            eventString = "DR7_ON";
            break;
            
        case 0x18:
            eventString = "DR0_OFF";
            break;
        case 0x19:
            eventString = "DR1_OFF";
            break;
        case 0x1A:
            eventString = "DR2_OFF";
            break;
        case 0x1B:
            eventString = "DR3_OFF";
            break;
        case 0x1C:
            eventString = "DR4_OFF";
            break;
        case 0x1D:
            eventString = "DR5_OFF";
            break;
        case 0x1E:
            eventString = "DR6_OFF";
            break;
        case 0x1F:
            eventString = "DR7_OFF";
            break;
            
        case 0x20:
            eventString = "PWR_GOOD";
            break;
        case 0x21:
            eventString = "PWR_LOW";
            break;
    }
    return eventString;
}

SYSTEMTIME timet_to_systime( time_t t )
{
    struct tm gm = *localtime( &t );
    SYSTEMTIME st = { 1900+gm.tm_year, 1+gm.tm_mon, gm.tm_wday, gm.tm_mday, gm.tm_hour, gm.tm_min, gm.tm_sec, 0 };
    return st;
}

time_t systime_to_timet( SYSTEMTIME st )
{
    struct tm gm = { st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth-1, st.wYear-1900, st.wDayOfWeek, 0, 0 };
    
    return mktime( &gm );
}

void gc_info()
{
    FPGAInfo Infor;
    struct _INTRUSION_INFOR intruInfor;
    unsigned char ucCount;
    
    _GetFPGAInfor(&Infor);
    
    printf("\t      Manufacture ID: %s\n",
           Infor.szManufactureID);
    printf("\t         Customer ID: %s\n",
           Infor.szCustomerID);
    printf("\t         Module Name: %s\n",
           Infor.szModuleName);
    printf("\t        FPGA Version: %d.%d.%d\n",
           Infor.szVersionNumber[0],
           Infor.szVersionNumber[1],
           Infor.szVersionNumber[2]);
    printf("\t     FPGA Build Time: %d%d%d%d-%d%d-%d%d\n",
           Infor.szBuildtime[0],
           Infor.szBuildtime[1],
           Infor.szBuildtime[2],
           Infor.szBuildtime[3],
           Infor.szBuildtime[4],
           Infor.szBuildtime[5],
           Infor.szBuildtime[6],
           Infor.szBuildtime[7]);

    printf("\t         API Version: %d.%d.%d\n",
           Infor.szAPIVersion[0],
           Infor.szAPIVersion[1],
           Infor.szAPIVersion[2]);

    printf("\t      Driver Version: %c%c%c%c%c%c%c%c\n",
           Infor.szFPGADriverVerion[0],
           Infor.szFPGADriverVerion[1],
           Infor.szFPGADriverVerion[2],
           Infor.szFPGADriverVerion[3],
           Infor.szFPGADriverVerion[4],
           Infor.szFPGADriverVerion[5],
           Infor.szFPGADriverVerion[6],
           Infor.szFPGADriverVerion[7]);

    _IntrGetLogCount(&ucCount);
    if (ucCount != 0) {
        _IntrGetInfor(&intruInfor);
        printf("\tIntrusion Build Time: %s\n",
               intruInfor.szBuildTime);
        printf("\tIntrusion FW Version: %s\n",
               intruInfor.szFWVersionNo);
    }
}


void gc_output_count()
{
    unsigned char ucCount;
    
    _GetFPGAOCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("[ %d ]\n",ucCount);
    
    return;
}

void gc_output_set_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiFlag;
    
    _GetFPGAOCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Pin Flag [0=Lo, 1=Hi] : ");scanf("%d",&uiFlag);
    
    _SetFPGAOIdx(uIdx,(bool)uiFlag);
    
    return;
}

void gc_output_set_pins()
{
    unsigned long maxData[2] = {0,0};
    unsigned char ucCount;
    unsigned long ulData1;
    unsigned long ulData2;
    int i;
    
    _GetFPGAOCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    for (i=0;i<ucCount;i++) 
    {
        if (i >= 32 ) 
        {
            maxData[1] |= (1<<(i-32));
        } 
        else 
        {
            maxData[0] |= (1<<(i));
        }
    }
    
    if (ucCount > 32 ) 
    {
        printf("GPO(31:00) Flag, Max[0x%x] : 0x", maxData[0]);scanf("%08lx",&ulData1);
        printf("GPO(%d:32) Flag, Max[0x%x] : 0x",ucCount-1 ,maxData[1]);scanf("%08lx",&ulData2);
    } 
    else 
    {
        printf("GPO(%d:00) Flag, Max[0x%x] : 0x", ucCount-1,maxData[0]);scanf("%08lx",&ulData1);
        ulData2=0;
    }
    _SetFPGAOutput(ulData1,ulData2);
    
    return;
}

void gc_output_set_all()
{
    
    int Flag;
    
    printf("[0] OFF [1] ON\n>> "); scanf("%d", &Flag);
    
    _SetFPGAOAll((bool)Flag);
    
    return;
}

void gc_output_get_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;
    
    _GetFPGAOCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    
    _GetFPGAOIdx(uIdx, &bFlag);    
    if (bFlag == 0x01) 
    {
        printf("[ HIGH ]\n");
    } 
    else 
    {
        printf("[ LOW ]\n");
    }
    
    return;
}
void gc_output_get_pins()
{
    unsigned long ulData1;
    unsigned long ulData2;
    unsigned char ucCount;
    int i;
    
    ulData1 =0;
    ulData2 =0;
    
    _GetFPGAOCount(&ucCount);
    _GetFPGAOutput(&ulData1,&ulData2);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("PIN HIGH:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) != 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) != 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    printf("PIN LOW:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) == 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) == 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    
    return;
}

void gc_input_count()
{
    unsigned char ucCount;
    
    _GetFPGAICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("[ %d ]\n",ucCount);
    
    return;
}

void gc_input_get_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;
    
    _GetFPGAICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("Pin Number: [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    
    _GetFPGAIIdx(uIdx, &bFlag);    
    if (bFlag == 0x01) 
    {
        printf("HIGH\n");
    } 
    else 
    {
        printf("LOW\n");
    }
    
    return;
}

void gc_input_get_pins()
{
    unsigned long ulData1;
    unsigned long ulData2;
    unsigned char ucCount;
    int i;
    
    ulData1 =0;
    ulData2 =0;
    _GetFPGAICount(&ucCount);
    _GetFPGAInput(&ulData1,&ulData2);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("PIN RELEASED:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) != 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) != 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    printf("PIN PRESSED:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) == 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) == 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    
    return;
}

void gc_input_set_deb()
{
    unsigned char ucCount;
    int i,j;
    
    _GetFPGAICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("Input PI Group [0-%d] : ",ucCount/8-1);scanf("%d",&i);
    printf("Input PI Debunce Time [0-7]: ");scanf("%d",&j);
    _SetFPGAIHWDebTime((unsigned long) i,(unsigned long) j);
    
    return;
}

void gc_intr_input_count()
{
    unsigned char ucCount;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("IRQ PI Count: %d\n",ucCount);
    
    return;
}

void gc_intr_input_set_ena_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiFlag;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Set IRQ Status Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Set IRQ Pin Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uiFlag);
    _SetIrqPIEnableIdx(uIdx,(bool) uiFlag);
    
    return;
}

void gc_intr_input_set_ena_pins()
{
    unsigned char ucCount;
    unsigned long maxData[2] = {0,0};
    unsigned long ulData1, ulData2;
    int i;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    for (i=0;i<ucCount;i++) 
    {
        if (i >= 32 ) 
        {
            maxData[1] |= (1<<(i-32));
        } 
        else 
        {
            maxData[0] |= (1<<(i));
        }
    }
    
    if (ucCount > 32 ) 
    {
        printf("Set IRQ Pins Status [31:0]  Max[0x%x] : 0x", maxData[0]);scanf("%08lx",&ulData1);
        printf("Set IRQ Pins Status [%d:32] Max[0x%x] : 0x",ucCount-1 ,maxData[1]);scanf("%08lx",&ulData2);
    } 
    else 
    {
        printf("Set IRQ Pins Status [%d:0] Max[0x%x] : 0x", ucCount-1,maxData[0]);scanf("%08lx",&ulData1);
        ulData2=0;
    }
    _SetIrqPIEnable(ulData1, ulData2);
    
    return;
}
void gc_intr_input_set_ena_all()
{
    unsigned int uiFlag;
    
    printf("Set IRQ All Pin Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uiFlag);
    
    _SetIrqPIEnableAll((bool)uiFlag);
    
    return;
}

void gc_intr_input_get_ena_pin()
{
    unsigned char ucCount;
    bool bFlag;
    unsigned long uIdx;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("Get IRQ Status Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    if (_GetIrqPIEnableIdx(uIdx,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
    
    return;
}

void gc_intr_input_get_ena_pins()
{
    unsigned char ucCount;
    unsigned long ulData1, ulData2;
    int i;
    
    _GetIrqPICount(&ucCount);
    _GetIrqPIEnable(&ulData1,&ulData2);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("PIN ENABLE:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) != 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) != 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    printf("PIN DISABLE:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) == 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) == 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    return;
}

void gc_intr_autmr_count()
{
    unsigned char ucCount;
   
    _GetAutoTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Auto Timer IRQ Count: %d\n",ucCount);
    return;
}

void gc_intr_autmr_set_ena_pin()
{
    unsigned int uiFlag;
    
    printf("Set Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uiFlag);
    _SetAutoTimerEnableIdx(0,(bool)uiFlag);
    return;
}

void gc_intr_autmr_get_ena_pin()
{
    bool bFlag;
    
    printf("Timer -auto - IRQ Status : ");
   
    if (_GetAutoTimerEnableIdx(0,&bFlag) == false) 
    {
        return;
    }
 
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
    return;
}

void gc_intr_timer_count()
{
    unsigned char ucCount;
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Timer IRQ Count: %d\n",ucCount);
    return;
}

void gc_intr_timer_set_ena_pin()
{
    unsigned char ucCount;
    unsigned int uiFlag;
    unsigned long uIdx;
   
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Set Timer IRQ Status Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Set Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uiFlag);
    _SetTimerEnableIdx(uIdx,(bool) uiFlag);
    return;
}

void gc_intr_timer_get_ena_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;
   
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Get Timer IRQ Status Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    if (_GetTimerEnableIdx(uIdx,&bFlag) == false) 
    {
        return;
    }
 
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
    return;
}

void gc_intr_timer_set_setup()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiReload;
    unsigned long ulCount;
    unsigned long ulInterval;
    
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Set Timer Value, Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Set Reload [0:OneTime, 1:Repeat] : ");scanf("%u",&uiReload);
    printf("Set Timer Count Value [1~65535] : ");scanf("%lu",&ulCount);
    printf("Set Timer Interval [0-7] : ");scanf("%lu",&ulInterval);

    _SetTimerIdx(uIdx, (bool) uiReload, ulCount, ulInterval);
    return;
}

void gc_intr_timer_get_setup()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiReload;
    unsigned long ulCount;
    unsigned long ulInterval;
    bool bReload;
    
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Get Timer Value, Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 

    _GetTimerIdx(uIdx, &bReload, &ulCount, &ulInterval);
    printf("Reload Value: %u\n", bReload);
    printf("Timer Count Value : %lu\n", ulCount);
    printf("Timer Interval Value : %lu\n", ulInterval);
    return;
}

void gc_intr_input_get_times()
{
    unsigned char ucCount;
    unsigned long ulCount;
    unsigned long uIdx;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("PI PRESS Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
  
    _GetIrqPICounter(uIdx, &ulCount);
    printf("PI Pin %lu press count = %lu\n", uIdx,ulCount);
    
    return;
}

void gc_intr_input_get_times_all()
{
    unsigned char ucCount;
    int i,j;
    unsigned long *ulCounterBuff;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    ulCounterBuff = (unsigned long*) malloc(sizeof(unsigned long) * ucCount);
    if (ulCounterBuff == NULL) 
    {
        return;
    }

    _GetIrqPICounterPack(ulCounterBuff,sizeof(unsigned long) * ucCount);
    
    printf("PI All Interrupt count:\n");

    for (i=0;i<8;i++) 
    {
    // 2013/02/23 Debug - Just show valid counter  
    //    for (j=0;j<ucCount;j+=8) 
	for (j=0;(i+j)<ucCount;j+=8) 
        {
            printf("[%02d]:%6ld\t",i+j,ulCounterBuff[i+j]);
        }
        printf("\n");
    }

    free(ulCounterBuff);
    
    return;
}

void gc_intr_autmr_get_times()
{
    unsigned long ulCount;
    
    _GetTimerCounter(0, &ulCount);
    printf("Auto Timer 0 time up count = %lu\n", ulCount);
    
    return;
}

void gc_intr_timer_get_times()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned long ulCount;
    
    _GetTimerCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("Select Timer  Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); uIdx++;  

    _GetTimerCounter(uIdx, &ulCount);
    printf("Timer %lu time up count = %lu\n", --uIdx,ulCount);
    
    return;
}

void gc_intr_timer_get_times_all()
{
    unsigned char ucTimerCount;
    unsigned char ucAutoTimerCount;
    unsigned char ucCount;
    unsigned long* ulCounterBuff;
    int i;
    
    _GetTimerCount(&ucTimerCount);
    _GetAutoTimerCount(&ucAutoTimerCount);
    
    ucCount = ucTimerCount+ucAutoTimerCount;
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    if (ucCount == 0) 
    {
        return;
    }
    ulCounterBuff = (unsigned long*) malloc(sizeof(unsigned long) * ucCount);
    if (ulCounterBuff == NULL) 
    {
        return;
    }
    
    _GetTimerCounterPack(ulCounterBuff,sizeof(unsigned long) * ucCount);    
    for (i=0;i<ucCount;i++) 
    {
        if (i == 0) 
        {
            printf("AutoTimer  Interrupt count: %ld\n",*(ulCounterBuff+i));
        } 
        else 
        {
            printf(" Timer [%d], count: %ld\n",i-1,*(ulCounterBuff+i));
        }
    }
    free(ulCounterBuff);
    
    return;
}

void gc_intr_input_set_edge_pin()
{
    unsigned char ucCount;
    unsigned int uiFlag;
    unsigned long uIdx;
    
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Set IRQ Setting Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Set IRQ Pin Trigger edge [0=FALLING, 1=RISING] : ");scanf("%u",&uiFlag);
    
    _SetIrqPISettingIdx(uIdx, (bool)uiFlag);
    
    return;
}

void gc_intr_input_set_edge_all()
{
    unsigned int uiFlag;
    
    printf("Set IRQ All Pin Trigger edge [0=FALLING, 1=RISING] : ");scanf("%u",&uiFlag);
 
    _SetIrqPISettingAll((bool)uiFlag);
    
    return;
}

void gc_intr_input_set_edge_pins()
{
    unsigned char ucCount;
    unsigned long ulData1, ulData2;
    unsigned long maxData[2] = {0,0};
    int i;
 
    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
	   for (i=0;i<ucCount;i++) 
    {
        if (i >= 32 ) 
        {
            maxData[1] |= (1<<(i-32));
        } 
        else 
        {
            maxData[0] |= (1<<(i));
        }
    }

		if (ucCount > 32 ) 
    {
        printf("Set IRQ Pins Trigger edge[31:0]  Max[0x%x] : 0x", maxData[0]);scanf("%08lx",&ulData1);
        printf("Set IRQ Pins Trigger edge[%d:32] Max[0x%x] : 0x",ucCount-1 ,maxData[1]);scanf("%08lx",&ulData2);
    } 
    else 
    {
        printf("Set IRQ Pins Trigger edge[%d:0] Max[0x%x] : 0x", ucCount-1,maxData[0]);scanf("%08lx",&ulData1);
        ulData2=0;
    }

    _SetIrqPISetting(ulData1, ulData2);
    
    return;
}

void gc_intr_input_get_edge_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;

    _GetIrqPICount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Get IRQ Setting Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx);     

    if (_GetIrqPISettingIdx(uIdx,&bFlag) == false) 
    {
        return;
    }
    
    if (bFlag == true) 
    {
        printf("RISING\n");
    } 
    else 
    {
        printf("FALLING\n");
    }
    
    return;
}

void gc_intr_input_get_edge_pins()
{
    unsigned char ucCount;
    unsigned long ulData1, ulData2;
    int i;
    
    _GetIrqPICount(&ucCount);
    _GetIrqPISetting(&ulData1,&ulData2);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    printf("pin RISING Edge:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) != 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) != 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    printf("pin FALLING Edge:");
    for (i=0;i<ucCount;i++) 
    {
        if (i>=32) 
        {
            if ((ulData2 & (1 << (i-32) )) == 0x00) 
            {
                printf(" %d",i);
            }
        } 
        else 
        {
            if ((ulData1 & (1 << i )) == 0x00) 
            {
                printf(" %d",i);
            }
        }
    }
    printf("\n");
    return;
}

static bool isQuit;

void sigroutine(int signum)
{
    switch(signum) 
    {
        case SIGQUIT:
            isQuit = true;
            break;
    }
}

bool interruptFlag = false;
bool gc_intr_ev_loop_Flag = false;

static void InterruptFun(void)
{
    unsigned char ucCount, ucCounter;
    unsigned long ulCount, ulSize, Idx;
    int i,j,k;
    bool bPWOffEn, bPWOffValue, bIntrIrqEn;
    unsigned long ulIntrIrqData[MAXINTRIRQDATA];
    unsigned long ulIndex;
    unsigned long logdex;
    bool tes_flag;
    bool bEnabled;
 
		if(gc_intr_ev_loop_Flag == false)
			return; 
 
    _PowerOffEnable(&bEnabled);	
    if (bEnabled) 
    {
        _GetPWOffIrqEn(&bPWOffEn);
        if(bPWOffEn)
        {
            _GetPWOffIrqValue(&bPWOffValue);
            
            if(bPWOffValue)
            {
                _GetRamSize(&ulSize);
                for(Idx = 0; Idx < ulSize; Idx++)
                {
                    _RamWriteByte(Idx, 0x0);
                }
            }
        }
    }
    	
    _GetIrqPICount(&ucCount);
    for (i=0;i<ucCount;i++) 
    {
        _GetIrqPICounter(i, &ulCount);
        if(ulCount != 0) 
        {
            printf("interrupt PI %d detect, count = %ld \n",i,ulCount);
        }
    }
    
    _GetAutoTimerCount(&ucCount);
    for (i=0;i<ucCount;i++) 
    {
        _GetTimerCounter(i, &ulCount);
        if(ulCount != 0) 
        {
            // printf("CardNum: %d\n",gc_um);
             printf("interrupt AutoTimer %d detect, count = %ld \n",i,ulCount);
            // respone on IO-board's LED. no print. cause too fast.
	/*	
            if (tes_flag)
            {
                _SetFPGAOIdx(0,true);
                tes_flag = false;
            }
            else
            {
                _SetFPGAOIdx(0,false);
                tes_flag = true;
            }
	*/
        }
    }
    
    _GetTimerCount(&ucCount);
    for (i=0;i<ucCount;i++) 
    {
        _GetTimerCounter(i+1, &ulCount);
        if(ulCount != 0) 
        {
            printf("interrupt Timer %d detect, count = %ld \n",i,ulCount);
        }
    }
    
    _IntrGetLogCount(&ucCount);
    if(ucCount > 0)
    {
        _IntrGetIrqEn(&bIntrIrqEn);
        
        if(bIntrIrqEn == true)
        {
            for(i = 0; i < MAXINTRIRQDATA; i++)
            {
                ulIntrIrqData[i] = 0;
            }
            
            _IntrGetIrqOut(ulIntrIrqData); // 取得更改的記錄筆號
            
            j = (ucCount + 2) / 32;
            if(((ucCount + 2) % 32) != 0)
            {
                j++;
            }
            
            if(j > MAXINTRIRQDATA)
                j = MAXINTRIRQDATA;
            
            for(i = 0; i < j; i++) 
            {
                k = 0;
                
                logdex = ulIntrIrqData[i];
                //printf("logdex [%d]  = %08x\n",i,logdex);
                for(k = 0; k < 32; k++)
                {
                    if(logdex & (0x1 << k))
                    {
                        ulIndex = (i * 32) + k - 2;
                        _IntrCheckLog(ulIndex, &intrLog[ulIndex]);
                        printf("Index\t   Event\tYear\tMonth\tDay\tHour\tMinute\tSecond\n");
                        printf("%02lu\t%10s\t%d\t%d\t%d\t%d\t%d\t%d\n",
                               ulIndex,
                               event_string(intrLog[ulIndex].iEventID),
                               intrLog[ulIndex].st.wYear,
                               intrLog[ulIndex].st.wMonth,
                               intrLog[ulIndex].st.wDay,
                               intrLog[ulIndex].st.wHour,
                               intrLog[ulIndex].st.wMinute,
                               intrLog[ulIndex].st.wSecond  );
                    }
                }
            }
        }
    }
    	
		_PulseEnable(&bEnabled);
		if(bEnabled)
		{
				_GetPulseCount(&ucCount);
				for(i = 0; i < ucCount; i++)
				{
						_GetPulseUnsentCounter(i, &ucCounter);
						if(ucCounter > 0)
							printf("interrupt Pulse %d detect, count = %ld \n", i, ucCounter);
				}	
		}
		
		unsigned char ucStateCode;
		_HopperEnable(&bEnabled);
		if(bEnabled)
		{
				_GetHopperCount(&ucCount);
				for(i = 0; i < ucCount; i++)
				{
						_GetIrqHopperCounter(i, &ulCount);
						if(ulCount > 0)
						{
								_GetHopperState(i, &ucStateCode);
								printf("interrupt Hopper %d detect, error code : %c \n", i, ucStateCode);
						}
				}	
		}
}

void gc_intr_ev_loop ()
{
    unsigned char event;
    int fd;
    
    isQuit = false;
    event = 0;
  
 		signal(SIGQUIT, sigroutine);
 	
 		gc_intr_ev_loop_Flag = true;		
    
    printf("----------------- PRESS CTRL-\\ to Quit --------------------\n");
 			
		while(1) 
		{
				if(isQuit)
				{
						gc_intr_ev_loop_Flag = false;						
						break;
				}
	
				usleep(1000); 					
		}
      

}

void gc_spr_get_count()
{
    unsigned char ucCount;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("%d\n", ucCount);
    return;
}

void gc_spr_get_attr_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("SPR Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    if (_GetFPGASPRIdxAtt(uIdx,&bFlag) == false) 
    {
        return;
    }
    
    if (bFlag == true) 
    {
        printf("PIN ATTR is INPUT\n");
    } 
    else 
    {
        printf("PIN ATTR is OUTPUT\n");
    }
    
    return;
}

void gc_spr_get_attr_all()
{
    unsigned char ucCount;
    unsigned short usData;
    int i;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    _GetFPGASPRAtt(&usData);
    printf("PIN ATT INPUT:");
    for (i=0;i<ucCount;i++) 
    {
        if ((usData & (1 << i )) != 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");
    printf("PIN ATT OUTPUT:");
    for (i=0;i<ucCount;i++) 
    {
        if ((usData & (1 << i )) == 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");
    return;
}

void gc_spr_set_attr_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiFlag;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    printf("SPR Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("SPR Pin Attr [0=Output, 1=Input] : ");scanf("%d",&uiFlag);
    _SetFPGASPRIdxAtt(uIdx, (bool)uiFlag);
    
    return;
}

void gc_spr_set_attr_all()
{
    unsigned int uiFlag;
    unsigned char ucCount;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    printf("Set SPR All Pin Att [0=OUTPUT, 1=INPUT] : ");scanf("%u",&uiFlag);
    _SetFPGASPRAllIdxAtt((bool)uiFlag);
    return;
}

void gc_spr_set_attr_pins()
{
    unsigned char ucCount;
    unsigned short usData = 0;
    int i;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    for (i=0;i<ucCount;i++) 
    { 
         usData |= (1<<(i));
    }
    
    printf("SPR PIN (%d:00) Att, Max[0x%x] : 0x", ucCount-1, usData);scanf("%04x",&usData);
    _SetFPGASPRAtt(usData);
    return;
}

void gc_spr_get_data_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    bool bFlag;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("SPR Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    if (_GetFPGASPRIdxData(uIdx,&bFlag) == false) 
    {
        return;
    }
    if (bFlag == true) 
    {
        printf("PIN VALUE is HIGH\n");
    }
    else 
    {
        printf("PIN VALUE is LOW\n");
    }
    return;
}

void gc_spr_get_data_all()
{
    unsigned char ucCount;
    unsigned short usData;
    int i;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    _GetFPGASPRData(&usData);
    printf("PIN VALUE HIGH:");
    for (i=0;i<ucCount;i++) 
    {
        if ((usData & (1 << i )) != 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");
    printf("PIN VALUE LOW:");
    for (i=0;i<ucCount;i++) 
    {
        if ((usData & (1 << i )) == 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");
    return;
}

void gc_spr_set_data_pin()
{
    unsigned char ucCount;
    unsigned long uIdx;
    unsigned int uiFlag;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("SPR Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("SPR Pin Value [0=LOW, 1=HIGH] : ");scanf("%d",&uiFlag);
    _SetFPGASPRIdxData(uIdx, (bool)uiFlag);
    
    return;
}

void gc_spr_set_data_pins()
{
    unsigned char ucCount;
    unsigned long ulData1;
    unsigned short usData;
    int i;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    for (i=0;i<ucCount;i++) 
    {
 				usData |= (1<<(i));	
    }
    
    printf("SPR PIN (%d:00) Value, Max[0x%x] : 0x", ucCount-1,usData);scanf("%08lx",&usData);
    _SetFPGASPRData(usData);
    return;
}

void gc_spr_set_data_all()
{
    unsigned int uiFlag;
    unsigned char ucCount;
    
    _GetFPGASPRCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    
    // issue: if attr is all input,  can to write data?
    printf("Set SPR All Pin VALUE [0=LOW, 1=HIGH] : ");scanf("%u",&uiFlag);
    _SetFPGASPRAllIdxData((bool)uiFlag);
    return;
}

void gc_random_tes()
{
    unsigned char ucCount;
    int i,j;
    unsigned long ulData;
    
    printf("Input Random Times : ");scanf("%d",&j);
    _GetRandomCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    // title
    for (i=0;i<ucCount;i++) 
    {
        printf("Rdm_%d ",i);
    }
    printf("\n");
  
    //	j = 5;  
    // content
    while (j != 0) 
    {
        for (i=0;i<ucCount;i++) 
        {
            _GetRandom(i,&ulData);
            printf("%05lu ",ulData);
        }
        printf("\n");
        
        j--;
    }
    return;
}

void gc_rtc_get_access_status()
{
    bool bWriteFlag;
    
    _GetFPGARTCStatus(&bWriteFlag);
    if (bWriteFlag) 
    {
        printf("WRITEABLE\n");
    } 
    else 
    {
        printf("NOT WRITE\n");
    }
    return;
}
void gc_rtc_get_time()
{
    FPGA_RTC RTCTime;
    
    _GetFPGARTC(&RTCTime);
    printf("RTC:\n");
    printf("Year    = %d\n",RTCTime.Year);
    printf("Month   = %d\n",RTCTime.Month);
    printf("day     = %d\n",RTCTime.day);
    printf("date    = %d\n",RTCTime.date);
    printf("hour    = %d\n",RTCTime.hour);
    printf("Min     = %d\n",RTCTime.Min);
    printf("Sec     = %d\n",RTCTime.Sec);
    printf("Century = %d\n",RTCTime.Century);
    return;
}
void gc_rtc_set_time()
{
    bool bWriteFlag;
    FPGA_RTC RTCTime;
    unsigned int rtcData;
    
    printf("Input RTC:\n");
    printf("Year    = ");scanf("%d",&rtcData); RTCTime.Year= rtcData;
    printf("Month   = ");scanf("%d",&rtcData); RTCTime.Month= rtcData;
    printf("day     = ");scanf("%d",&rtcData); RTCTime.day= rtcData;
    printf("date    = ");scanf("%d",&rtcData); RTCTime.date= rtcData;
    printf("hour    = ");scanf("%d",&rtcData); RTCTime.hour= rtcData;
    printf("Min     = ");scanf("%d",&rtcData); RTCTime.Min= rtcData;
    printf("Sec     = ");scanf("%d",&rtcData); RTCTime.Sec= rtcData;
    printf("Century = ");scanf("%d",&rtcData); RTCTime.Century= rtcData;
    
    bWriteFlag = false;
    while(!bWriteFlag) 
    {
        _GetFPGARTCStatus(&bWriteFlag);
    }
    _SetFPGARTC(RTCTime);
    return;
}

#define MEMORYTYPECNT 8
#define MAX_ERROR_COUNT 100 

void gc_sram_get_sz()
{
    unsigned long ulSize;
    
    _GetRamSize(&ulSize);
    printf("SRAM Size: 0x%x (Bytes)\n",ulSize);
}

// 2013/04/10 Add memory test range setting.
void gc_sram_set_range()
{
    printf("Input RAM Size - [0:32K, 1:64K, 2:128K, 3:256K, 4:512K, 5:1M, 6:2M, 7:4M] : ");
    scanf("%d",&accSize);		
}

void gc_sram_write()
{
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    unsigned char ucData;
    int i;
    
    // 2013/04/10 Add memory test range setting.
    //printf("Input RAM Size - [0:32K, 1:64K, 2:128K, 3:256K, 4:512K, 5:1M, 6:2M, 7:4M] : ");
    //scanf("%d",&accSize);
    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return; 
    }
    
    for(i= 0 ; i<= ulMemRange[accSize]; i++) 
    { 
        ucData = (unsigned char)(i + (i>>8) + (i>>16));       
        _RamWriteByte(i, ucData);
    }
    printf("Write done.\n");
    
    return;
}

void gc_sram_write_chk()
{
    int errCount;
    // 2013/03/28 Debug - The range of SRAM Read check has to match with SRAM write.
    //int accSize = 0;
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    unsigned char ucData;
    int i;

		// 2013/04/10 Add memory test range setting.
    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return; 
    }	
    
    errCount = 0;
    for(i = 0; i <= ulMemRange[accSize]; i++) 
    { 
        if(errCount >= MAX_ERROR_COUNT) 
        {
            break;
        }
      
        _RamReadByte(i, &ucData);               
        if(ucData != (unsigned char)(i + (i>>8) + (i>>16)) ) 
        {
            printf("ERR Addr:%06X\twriteData:%02X\treadData:%02X\n" ,i
                   ,(unsigned char)(i + (i>>8) + (i>>16))
                   ,ucData  );
            errCount++;
        }   
    }
    
    if(errCount == 0)
        printf("Read Pass\n");
    else
        printf("Read Failed!\n");
    
    return;
}
void gc_sram_write_errbyte()
{
    int i,j;
    
    printf("Input Write Address: ");scanf("%d",&j);
    printf("Input Write Byte Value: 0x");scanf("%2x",&i);
    
    _RamWriteByte(j,(unsigned char) i);   
    
    return;
}

bool IsEnabled;
unsigned long ulStartAddr;
unsigned long ulEndAddr;
unsigned int  uMirrorBKMode;
bool IsFinished;
unsigned long ulRange;
unsigned char ucCode;
unsigned char ucDataS;
unsigned char ucDataD;

void gc_sram_mirror_setting_mode()
{
    _GetRamMirrorEn(&IsEnabled);
    if(!IsEnabled)
    {
				printf("Mirror - It is disabled.");
    }
    else
    {
				ulStartAddr = 0;
				ulEndAddr = 0;
				uMirrorBKMode = 0x2;
				// 2013/03/28 Debug - Modify mirror input format from decimal to hex.  
				//printf("BK Mode(0x2/0x3): 0x");scanf("%u",&uMirrorBKMode); 
				//printf("Start Addr: ");scanf("%lu",&ulStartAddr); 
				//printf("End   Addr: ");scanf("%lu",&ulEndAddr); 
				printf("BK Mode(0x2/0x3): 0x");scanf("%x",&uMirrorBKMode); 
				printf("Start Addr: 0x");scanf("%x",&ulStartAddr); 
				printf("End   Addr: 0x");scanf("%x",&ulEndAddr);
 				//  ulStartAddr and ulEndAddr need to be even.
				_SetRamMirrorRange((unsigned char)uMirrorBKMode, ulStartAddr, ulEndAddr);
    }
}

void gc_sram_mirror_start()
{
    int i;

    _GetRamMirrorEn(&IsEnabled);
    if(!IsEnabled)
    {
				printf("Mirror - It is disabled.\n");
    }
    else
    {
		// test
		//unsigned char ucDataTest;
		//_RamReadByte(ulStartAddr + 0x100000, &ucDataTest);
		//printf("Before mirror - addr : 0x%x, value : 0x%x \n", ulStartAddr+0x100000, ucDataTest);

    		printf("Waiting......\n");
				_SetRamMirrorStart();
				
				for(i = 0; i < 100; i++)
				{
				    //Sleep(100);
				    usleep(100000);
				    _GetRamMirrorIsFinished(&IsFinished);
				    if(IsFinished)
				    {
								printf("Mirror is finished.\n");
								break;
				    }
			      else if(i == 99)
				    {
								printf("Mirror action is more than 10 seconds.\n");
								break;
				    }   
				}
    }
}

void gc_sram_mirror_compare()
{
    unsigned long ulSize;
    int i, errCount;

    printf("Mirror data is comparing..\n");
    _GetRamSize(&ulSize);
    if(uMirrorBKMode == 0x3)
    {
				ulRange = ulSize / 4;
				_RamReadByte(ulSize - 1, &ucCode);
				
				if(ucCode == 0x10)
				{
				    ulRange = ulRange * 3;
				}
    }else
    {
				ulRange = ulSize / 2;
    }
    
    errCount = 0;
    for(i = ulStartAddr; i < (ulEndAddr + 2); i++ )
    {
				if(errCount >= MAX_ERROR_COUNT)
	    			break;
	
				_RamReadByte(i, &ucDataS);
				_RamReadByte(i + ulRange, &ucDataD);
				
				if(ucDataS != ucDataD)
				{
				    errCount++;
				}

				// test
				//printf("After mirror - addr : 0x%x, value : 0x%x \n", ulStartAddr+0x100000+i, ucDataD);

    }
    
    if (errCount == 0) 
    {
				printf("Comparing is finished.\n");
    } 
    else 
    {
				// 2013/03/28 2013/03/28 Debug - Modify mirror output format from decimal to hex.   
				//printf("Compare Error Count = %d\n",errCount);
				printf("Compare Error Count = 0x%x\n",errCount);
    }
}

void gc_flash_get_sz()
{
    unsigned long ulSize;
    
    _GetFlashSize(&ulSize);
    printf("Flash Size: %ld (Bytes)\n",ulSize);
}


// 2013/04/10 Add memory test range setting.
void gc_flash_set_range()
{
    printf("Input Flash Size - [0:32K, 1:64K, 2:128K, 3:256K, 4:512K, 5:1M, 6:2M, 7:4M] : ");
    scanf("%d",&accSize);		
}

void gc_flash_erase_all()
{
    printf("Erase All ING ...\n");
    _FlashEraseAll();
    printf("Done.\n");
    
    return;
}

#if 0
void gc_flash_erase_sector()
{
    unsigned long ulSector;
    
    printf("Which Sector  [0-33] : ");scanf("%lu",&ulSector);
    printf("Erase Sector ING ...\n");
    _FlashEraseSector(ulSector);
    printf("Done.\n");
    return;
}
#endif

void gc_flash_write()
{
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    int i;
    unsigned char ucData;
    
    // 2013/04/10 Add memory test range setting.
    //printf("Input RAM Size - [0:32K, 1:64K, 2:128K, 3:256K, 4:512K, 5:1M, 6:2M, 7:4M] : ");scanf("%d",&accSize);
    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return;
    }
    
    for(i= 0 ; i<= ulMemRange[accSize]; i++) 
    { 
        ucData = (unsigned char)(i + (i>>8) + (i>>16));
        _FlashWriteByte(i, ucData);
    }
    printf("Write done.\n");
    return;
}
void gc_flash_read()
{
    int errCount;
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    int i;
    unsigned char ucData;
 
 		// 2013/04/10 Add memory test range setting.
    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return;
    } 		
    
    errCount = 0;
    for(i = 0; i <= ulMemRange[accSize]; i++) 
    { 
        if(errCount >= MAX_ERROR_COUNT) 
        {
            break;
        }
        _FlashReadByte(i, &ucData);
        
        if(ucData != (unsigned char)(i + (i>>8) + (i>>16)) ) 
        {
            printf("ERR Addr:%06X\twriteData:%02X\treadData:%02X\n" ,i
                   ,(unsigned char)(i + (i>>8) + (i>>16))
                   ,ucData  );
            errCount++;
        }   
    }
    
    if(errCount == 0)
        printf("Read Pass\n");
    else
        printf("Read Failed!\n");
    return;
}


void gc_eeprom_write()
{
    unsigned long ulSize;
    unsigned char ucBuffer[0x100];
    int i;
    
    _GetEEPRomSize(&ulSize);	
    if (ulSize == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
	
    for(i= 0 ; i< ulSize; i++) 
    { 
        ucBuffer[i] = (unsigned char)(i + (i>>8) + (i>>16));
    }
    
    _EEPRomWrite(0, ucBuffer, ulSize);          
    printf("Write done.\n");
}

void gc_eeprom_read()
{
    unsigned long ulSize;
    unsigned char ucBuffer[0x100];
    int errCount;
    int i;
    
    _GetEEPRomSize(&ulSize);
	
    if (ulSize == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    _EEPRomRead(0,ucBuffer,ulSize);
    errCount = 0;
    for(i = 0; i < ulSize; i++) 
    { 
        if(errCount >= MAX_ERROR_COUNT) 
        {
            break;
        }
        
        if(ucBuffer[i] != (unsigned char)(i + (i>>8) + (i>>16)) ) 
        {
            printf("err EEPROM Addr:%06X\twriteData:%02X\treadData:%02X\n"  ,i
                   ,(unsigned char)(i + (i>>8) + (i>>16))
                   ,ucBuffer[i] );
            errCount++;
        }   
    }
    
    if(errCount == 0)
        printf("Read Pass\n");
    else
        printf("Read Failed!\n");
}

// 2013/03/26 Add EEPROM error byte test
void gc_eeprom_write_errbyte()
{
    int i,j;
    
    printf("Input Write Address: ");scanf("%d",&j);
    printf("Input Write Byte Value: 0x");scanf("%2x",&i);
    
    _RamWriteByte(j,(unsigned char) i); 
    _EEPRomWrite(j, &i, 1);	  
    
    return;
}

// 2013/04/10 Add EXT SRAM functions.
void gc_ext_sram_get_sz()
{
    unsigned long ulSize;
    
    _GetExtSRAMSize(&ulSize);
    printf("EXT SRAM Size: 0x%x (Bytes)\n",ulSize);
}

// 2013/04/10 Add EXT SRAM functions.
void gc_ext_sram_set_range()
{
    printf("Input RAM Size - [0:32K, 1:64K, 2:128K, 3:256K, 4:512K, 5:1M, 6:2M, 7:4M] : ");
    scanf("%d",&accSize);		
}

// 2013/04/10 Add EXT SRAM functions.
void gc_ext_sram_write()
{
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    unsigned char ucData;
    int i;
    
    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return; 
    }
    
    for(i= 0 ; i<= ulMemRange[accSize]; i++) 
    { 
        ucData = (unsigned char)(i + (i>>8) + (i>>16));       
        _ExtSRAMWriteByte(i, ucData);
    }
    printf("Write done.\n");
    
    return;
}

// 2013/04/10 Add EXT SRAM functions.
void gc_ext_sram_write_chk()
{
    int errCount;
    unsigned long ulMemRange[MEMORYTYPECNT] = {0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff};
    unsigned char ucData;
    int i;

    if(accSize < 0)
    {
    		printf("Please set test range.\n");
    		return; 
    }	
    
    errCount = 0;
    for(i = 0; i <= ulMemRange[accSize]; i++) 
    { 
        if(errCount >= MAX_ERROR_COUNT) 
        {
            break;
        }
      
        _ExtSRAMReadByte(i, &ucData);               
        if(ucData != (unsigned char)(i + (i>>8) + (i>>16)) ) 
        {
            printf("ERR Addr:%06X\twriteData:%02X\treadData:%02X\n" ,i
                   ,(unsigned char)(i + (i>>8) + (i>>16))
                   ,ucData  );
            errCount++;
        }   
    }
    
    if(errCount == 0)
        printf("Read Pass\n");
    else
        printf("Read Failed!\n");
    
    return;
}

// 2013/04/10 Add EXT SRAM functions.
void gc_ext_sram_write_errbyte()
{
    int i,j;
    
    printf("Input Write Address: ");scanf("%d",&j);
    printf("Input Write Byte Value: 0x");scanf("%2x",&i);
    
    _ExtSRAMWriteByte(j,(unsigned char) i);   
    
    return;
}


void gc_intru_get_log_count()
{
    unsigned char ucCount;
    
    _IntrGetLogCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    printf("Intrusion Log Count: %d\n",ucCount);
    return;
}

void gc_intru_rest_time()
{
    unsigned char ucCount;
    SYSTEMTIME st;
    time_t t;
    
    _IntrGetLogCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }
    time(&t);
    
    printf(" Time (System):\n");
    st = timet_to_systime(t);
    printf("Year\tMonth\tDay\tHour\tMinute\tSecond\n");
    printf("%d\t%d\t%d\t%d\t%d\t%d\n", 
           st.wYear,
           st.wMonth,
           st.wDay,
           st.wHour,
           st.wMinute,
           st.wSecond   );

    _IntrResetTime(st);
    printf("Set Done.\n");
    return;
}
void gc_intru_read_time()
{
    SYSTEMTIME st;

    _IntrReadTime(&st);
    printf("Time (Intrusion):\n");
    printf("Year\tMonth\tDay\tHour\tMinute\tSecond\n");
    printf("%d\t%d\t%d\t%d\t%d\t%d\n",
           st.wYear,
           st.wMonth,
           st.wDay,
           st.wHour,
           st.wMinute,
           st.wSecond   );
    return;
}

void gc_intru_init()
{
    unsigned char ucCount;
    
    _IntrGetLogCount(&ucCount);
    if (ucCount == 0) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    }

    _IntrInit();
    return;
}

void gc_intru_load()
{
    int i;
    unsigned char ucCount;
    
    printf("Log LOAD ing... \n");

    _IntrGetLogCount(&ucCount);
    _IntrLoadLog();
    for (i=0;i<ucCount;i++) 
    {
        _IntrCheckLog(i,&intrLog[i]);
    }
    printf("Done. \n");
    return;
}

void gc_intru_clear()
{
    printf("Log Clear ING ...\n");

    _IntrClearLog();
    printf("Done. \n");
    return;
}

void gc_intru_check()
{
    int i;
    unsigned char ucCount;
    
    _IntrGetLogCount(&ucCount);
    
    for (i=0;i<ucCount;i++)
    {
        _IntrCheckLog(i, &intrLog[i]);
    }
    
    printf("Index\tEvent\tYear\tMonth\tDay\tHour\tMinute\tSecond\n");
    for (i=0;i<ucCount;i++) 
    {
        printf("%02d\t%10s\t%d\t%d\t%d\t%d\t%d\t%d\n",
               i,
               event_string(intrLog[i].iEventID),
               intrLog[i].st.wYear,
               intrLog[i].st.wMonth,
               intrLog[i].st.wDay,
               intrLog[i].st.wHour,
               intrLog[i].st.wMinute,
               intrLog[i].st.wSecond    );
    }
    return;
}

void gc_intru_door_count()
{
    unsigned char ucCount;

    _IntrGetDoorCount(&ucCount);
    printf("%d\n", ucCount);
}

void gc_intru_door_get_status()
{
    unsigned char ucCount;
    int i;
    bool bFlag;

    _IntrGetDoorCount(&ucCount);
    for (i = 0; i< ucCount; i++) 
    {
        _IntrGetDoorStatus((unsigned long)i ,&bFlag);
        
        if (bFlag == 0x01) 
        {
            printf("Intrution Door %d Status: OPEN\n",i);
        } 
        else 
        {
            printf("Intrution Door %d Status: CLOS\n",i);
        }
    }
}

void gc_wdt_set_ena()
{
    unsigned int uiFlag;
    bool bFlag;

    _WDTEnable(&bFlag);
    if (!bFlag) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    } 

    printf("[0] disable [1] enable :");scanf("%d", &uiFlag);
    _SetWDTEnable((bool)uiFlag);
    
    return;
}

void gc_wdt_set_rst()
{
    bool bFlag;

    _WDTEnable(&bFlag);
    if (!bFlag) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    } 

    _SetWDTReset(true);
    printf("WDT Rest Done\n");
    
    return;
}

void gc_wdt_set_currval()
{
    unsigned int uiData;
    bool bFlag;

    _WDTEnable(&bFlag);
    if (!bFlag) 
    {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    } 
    
    printf("WDT Curr Value: ");scanf("%u",&uiData);
    
    _SetWDTCountValue((unsigned short)uiData);
    
    return;
}

void gc_wdt_get_currval()
{
    unsigned short usData;
    bool bFlag;

    _WDTEnable(&bFlag);
    if (!bFlag) {
        printf("FUNCTION NOT SUPPORT THIS TYPE.\n");
        return;
    } 
    
    _GetWDTCountValue(&usData);
    printf("%d\n",usData);

    return;
}

// Pulse
void gc_pulse_get_count()
{
     unsigned char ucCount;

    _GetPulseCount(&ucCount);
    printf("%d\n", ucCount); 
}

// Pulse
void gc_pulse_set_irq()
{
    unsigned int uFlag;

   	printf("Set interrupt Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    _SetPulseIrqEn((bool)uFlag);
}

// Pulse
void gc_pulse_set_SW_code()
{
    unsigned int uSWCode;

   	printf("Set switch code for GA2200 [0x0= Normal GPO, 0x5=HLH, 0x6=LHL] : 0x:");scanf("%u",&uSWCode);
    _SetPulseSwitchCode(uSWCode);
    printf("Set Done.\n");
}

// Pulse
void gc_pulse_set_mode()
{
    unsigned int ucPulseMode;

   	printf("Set pulse mode [0x0= Normal GPO, 0x1=HLH, 0x3=LHL] : 0x:");scanf("%02x",&ucPulseMode);
    _GetPulseTypeCode(ucPulseMode);
    printf("Set Done.\n");
}

void gc_pulse_set_Fre()
{
    unsigned char ucCount;
    unsigned int ucFreq;
    unsigned long uIdx;
		
		_GetPulseCount(&ucCount);
    printf("Set pulse Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Set pulse frequency : 0x");scanf("%02x",&ucFreq);
		_SetPulseFrequence(uIdx, (unsigned char)ucFreq);
		printf("Set Done.\n");	
}

void gc_pulse_add_counter()
{
    unsigned char ucCount;
    unsigned int ucCounter;
    unsigned long uIdx;	
	
		_GetPulseCount(&ucCount);
    printf("Set pulse Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    printf("Add pulse counter : 0x");scanf("%02x",&ucCounter);	
		_AddPulseCounter(uIdx, (unsigned char)ucCounter);
		printf("Set Done. \n");
}

void gc_pulse_get_counter()
{
    unsigned char ucCount, ucCounter;
    unsigned long uIdx;
		
		_GetPulseCount(&ucCount);
    printf("Get pulse Pin Number [0-%d] : ",ucCount-1);scanf("%lu",&uIdx); 
    if(_GetPulseUnsentCounter(uIdx, &ucCounter)== false){
    		printf("Get counter failed. \n");
    		return;
    }else {
    		printf("Get pulse counter : 0x%02x \n", ucCounter);	
  	}
}

void gc_serialoutput_set_SW_code()
{
    unsigned int uSWCode;

   printf("Set switch code for GA1200 [0x0= Normal GPO, 0x1= SerialOutput] : 0x");scanf("%02x",&uSWCode);
    _SetSerialOutputSwitchCode((unsigned char)uSWCode);
    printf("Set Done.\n");			
}

void gc_serialoutput_set_pulse_mode()
{
    unsigned int ucPulseMode;

    printf("Set pulse mode (0x0=Normal GPO, 0x1=HLH, 0x3=LHL): 0x");scanf("%02x",&ucPulseMode);	
    _SetSerialOutputPulseMode((unsigned char)ucPulseMode);
    printf("Set Done.\n");	
}

void gc_serialoutput_set_value()
{
   	unsigned int ucData;
   	bool bFlag;
	int i;

   	printf("Set Serial output data (0x0~0xff): 0x");scanf("%02x",&ucData);
   	_SetSerialOutput(ucData);
   	
	  for(i=0; i < 100; i++)
		{
				usleep(100000);
				_GetSerialOutputIsFinished(&bFlag);
				if(bFlag)
				{
						printf("SerialOutput end. \n");
						break;
				}
		
				if(i == 99)
				{
						printf("SerialOutput sending fail. \n");
				}
		}					
}

void gc_lamp_flash_get_ena_pin()
{
    bool bFlag;
    unsigned long uIdx;
      
    printf("Get flash Pin Number [0-15] : ");scanf("%lu",&uIdx); 
    if (_GetLampFlashIdxEn(uIdx,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
    
    return;
}

void gc_lamp_flash_set_ena_pin()
{
		unsigned long uIdx;
		unsigned int uFlag;
		
    printf("Set flash Pin Number [0-15] : ");scanf("%lu",&uIdx); 
    printf("Set flash Pin Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    
    _SetLampFlashIdxEn(uIdx, (bool)uFlag);
    printf("Set Done. \n");		
}

void gc_lamp_flash_get_ena_pins()
{
    unsigned short usData;
    int i;
    
    usData =0;
    
    _GetLampFlashEn(&usData);
        
    printf("PIN HIGH:");
    for (i=0;i<16;i++) 
    {   
		    if ((usData & (1 << i )) != 0x00) 
		    {
		        printf(" %d",i);
		    }
    }
    printf("\n");
    printf("PIN LOW:");
    for (i=0;i<16;i++) 
    { 
        if ((usData & (1 << i )) == 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");		
}

void gc_lamp_flash_set_ena_pins() 
{
    unsigned short usData;
    
    printf("Flash(15:00) Flag [0x0~0xffff] : 0x");scanf("%04x",&usData);
    _SetLampFlashEn(usData);
    printf("Set Done. \n");
}

void gc_lamp_flash_get_value()
{
		unsigned char ucData;
		
		_GetLampFlashValue(&ucData);
		printf("Get Flash value : 0x%04x \n", ucData);
}

void gc_lamp_flash_set_value()
{
		unsigned int uData;
		
		printf("Set Flash value[0x1~0x7]: 0x");scanf("%u",&uData);
		_SetLampFlashValue(uData);
		printf("Set Done. \n");
}

void gc_lamp_brightness_get_ena_pin()
{
    bool bFlag;
    unsigned long uIdx;
      
    printf("Get Brightness Pin Number [0-15] : ");scanf("%lu",&uIdx); 
    if (_GetLampBrightnessIdxEn(uIdx,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
}

void gc_lamp_brightness_set_ena_pin()
{
		unsigned long uIdx;
		unsigned int uFlag;
		
    printf("Set Brightness Pin Number [0-15] : ");scanf("%lu",&uIdx); 
    printf("Set Brightness Pin Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    
    _SetLampBrightnessIdxEn(uIdx, (bool)uFlag);
    printf("Set Done. \n");		
}

void gc_lamp_brightness_get_ena_pins()
{
    unsigned short usData;
    unsigned char ucCount;
    int i;
    
    usData =0;
   
    _GetLampBrightnessEn(&usData);
        
    printf("PIN HIGH:");
    for (i=0;i<16;i++) 
    {   
		    if ((usData & (1 << i )) != 0x00) 
		    {
		        printf(" %d",i);
		    }
    }
    printf("\n");
    printf("PIN LOW:");
    for (i=0;i<16;i++) 
    { 
        if ((usData & (1 << i )) == 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");		
}

void gc_lamp_brightness_set_ena_pins() 
{
    unsigned short usData;
    
    printf("Brightness(15:00) Flag [0x0~0xffff] : 0x");scanf("%04u",&usData);
    _SetLampBrightnessEn(usData);
    printf("Set Done. \n");
}

void gc_lamp_brightness_get_value()
{
		unsigned char ucData;
		
		_GetLampBrightnessValue(&ucData);
		printf("Get Brightness value : 0x%02x \n", ucData);
}
	
void gc_lamp_brightness_set_value()
{
		unsigned int uValue;
		
		printf("Set Brightness value[0x1~0x7]: 0x");scanf("%u",&uValue);
		_SetLampBrightnessValue(uValue);
		printf("Set Done. \n");
}

void gc_individlamp_get_count()
{
		unsigned char ucCount;

    _GetIndividLampBrightnessCount(&ucCount);
    printf("%d\n", ucCount); 		
}

void gc_individlamp_brightness_get_ena_pin()
{
    bool bFlag;
    unsigned long uIdx;
    unsigned char ucCount;
      
 		_GetIndividLampBrightnessCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get individual Brightness Pin Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 
    if (_GetIndividLampBrightnessIdxEn(uIdx,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }
}

void gc_individlamp_brightness_set_ena_pin()
{
		unsigned long uIdx;
		unsigned int uFlag;
		unsigned char ucCount;
		
		_GetIndividLampBrightnessCount(&ucCount);
		
		if(ucCount == 0){
				printf("Not implement. \n");
				return;
		}
		
    printf("Set individual Brightness Pin Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 
    printf("Set individual Brightness Pin Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    
    _SetIndividLampBrightnessIdxEn(uIdx, (bool)uFlag);
    printf("Set Done. \n");		
}

void gc_individlamp_brightness_get_ena_pins()
{
    unsigned short usData;
    unsigned char ucCount;
    int i;
    
    usData =0;

		_GetIndividLampBrightnessCount(&ucCount);		
		if(ucCount == 0){
				printf("Not implement. \n");
				return;
		}   
		
    _GetIndividLampBrightnessEn(&usData);
        
    printf("PIN HIGH:");
    for (i=0;i<ucCount;i++) 
    {   
		    if ((usData & (1 << i )) != 0x00) 
		    {
		        printf(" %d",i);
		    }
    }
    printf("\n");
    printf("PIN LOW:");
    for (i=0;i<ucCount;i++) 
    { 
        if ((usData & (1 << i )) == 0x00) 
        {
            printf(" %d",i);
        }
    }
    printf("\n");		
}

void gc_individlamp_brightness_set_ena_pins() 
{
    unsigned short usData, usMaxData;
    unsigned char ucCount;
    int i;
  
	usMaxData = 0x0;
  	_GetIndividLampBrightnessCount(&ucCount);  
		for (i=0;i<ucCount;i++) 
				usMaxData |= (1<<(i));
    
    printf("individual Brightness(%d:00) Flag [0x0~0x%x] : 0x", ucCount, usMaxData);scanf("%04x",&usData);
    _SetIndividLampBrightnessEn(usData);
    printf("Set Done. \n");
}

void gc_individlamp_brightness_get_value()
{
		unsigned char ucData;
    unsigned char ucCount;
    unsigned long uIdx;

  	_GetIndividLampBrightnessCount(&ucCount);  	
	
		printf("Get individual Brightness Pin Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 	
		_GetIndividLampBrightnessIdxValue(uIdx, &ucData);
		printf("Get individual Brightness value : 0x%02x  \n", ucData);
}
	
void gc_individlamp_brightness_set_value()
{
		unsigned int uValue;
    unsigned char ucCount;
    unsigned long uIdx;
 
  	_GetIndividLampBrightnessCount(&ucCount);  	
	
		printf("Set individual Brightness Pin Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 			
		printf("Set individual Brightness value[0x1~0x7]: 0x");scanf("%u",&uValue);
		_SetIndividLampBrightnessIdxValue(uIdx, uValue);
		printf("Set Done. \n");
}

void gc_hopper_get_count()
{
     unsigned char ucCount;

    _GetHopperCount(&ucCount);
    printf("%d\n", ucCount); 		
}

void gc_hopper_set_pulse_mode()
{
    unsigned int ucPulseMode;

   	printf("Set pulse mode [0x0= Normal GPO, 0x1=HLH, 0x3=LHL] : 0x:");scanf("%02x",&ucPulseMode);
    _SetPulseSwitchCode((unsigned char)ucPulseMode);
    printf("Set Done.\n");		
}

void gc_hopper_set_counter()
{
		unsigned char ucCount;
		unsigned long uIdx, ulCounter;
		
 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 
    printf("Get Hopper Counter : ");scanf("%lu",&ulCounter); 
    
		_SetHopperCounter(uIdx-1, ulCounter);
 		printf("Set Done. \n");		
}

void gc_hopper_get_counter()
{
		unsigned char ucCount;
		unsigned long uIdx, ulCounter;
		
 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 
    if (_GetHopperCounter(uIdx-1,&ulCounter) == false) 
    {
        return;
    }
     
 		printf("Unsent Counter : %lu \n", ulCounter);
}		

void gc_hopper_get_irq_ena_pin()
{
    bool bFlag;
    unsigned long uIdx;
    unsigned char ucCount;
      
 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper interrupt Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 
    if (_GetIrqHopperEnableIdx(uIdx-1,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }		
}

void gc_hopper_set_irq_ena_pin()
{
    unsigned long uIdx;
    unsigned char ucCount;
    unsigned int uFlag;
      
 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper interrupt Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 
    printf("Set Hopper interrupt Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    
    _SetIrqHopperEnableIdx(uIdx-1, (bool)uFlag);
    
    printf("Set Done. \n");
}

void gc_hopper_reset()
{
		unsigned int ucResetCode;
		unsigned char ucCount;
		unsigned long uIdx;

 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 		
		printf("Reset Hopper[0x1:Reset all(clear all data), 0x2:Reset from error ] : 0x" );scanf("%02x",&ucResetCode); 
		_ResetHopper(uIdx-1, (unsigned char)ucResetCode);
		printf("Set Done. \n");
}

void gc_hopper_set_restart()
{
		unsigned char ucCount;
		unsigned long uIdx;

 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 	
    _SetHopperReStart(uIdx-1);
    printf("Set Done. \n");			
}

void gc_hopper_set_pause()
{
		unsigned char ucCount;
		unsigned long uIdx;

 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 	
    _SetHopperPause(uIdx-1);
    printf("Set Done. \n");			
}

void gc_hopper_get_state()
{
		unsigned char ucCount;
		unsigned char ucState;
		unsigned long uIdx;

 		_GetHopperCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get Hopper Number [1-%d] : ", ucCount);scanf("%lu",&uIdx); 	
    _GetHopperState(uIdx-1, &ucState);
    printf("State code( 0x0: Default,   0x1: Inprocessing,  0x2: Stop,  0x3: Pause,  0x11: Coin is insufficient,  0x21: Jammed ) : 0x%x  \n", ucState); 
}

void gc_hpmotorout_get_count()
{
    unsigned char ucCount;

    _GetHPMotorOutCount(&ucCount);
    printf("%d\n", ucCount); 			
}

void gc_hpmotorout_get_ena_pin()
{
    bool bFlag;
    unsigned long uIdx;
    unsigned char ucCount;
      
 		_GetHPMotorOutCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get HPMotorOut PIN  Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 
    if (_GetHPMotorOutIdx(uIdx,&bFlag) == false) 
    {
        return;
    }
     
    if (bFlag == true) 
    {
        printf("ENABLE\n");
    } 
    else 
    {
        printf("DISABLE\n");
    }				
}

void gc_hpmotorout_set_ena_pin()
{
    unsigned long uIdx;
    unsigned char ucCount;
    unsigned int uFlag;
    
    // 2013/02/25 Debug HPMotorout count. 	  
    //_GetHopperCount(&ucCount);
    _GetHPMotorOutCount(&ucCount);
    if(ucCount == 0){
    		printf("Not implement. \n");
    		return;
    }
    
    printf("Get HPMotorOut PIN Number [0-%d] : ", ucCount-1);scanf("%lu",&uIdx); 
    printf("Set HPMotorOut PIN Status [0=DISABLE, 1=ENABLE] : ");scanf("%u",&uFlag);
    
    _SetHPMotorOutIdx(uIdx, (bool)uFlag);
    
    printf("Set Done. \n");
}

void tes_about()
{
    printf("\n\t=============================================\n");	
    printf("\t  AEWIN Gaming Cotroller Test Sample App.\n");
    printf("\t---------------------------------------------\n");
    printf("\t          Version: 3.3.0\n");
    printf("\t=============================================\n\n");	
}

static bool itemloop;
void tes_exit()
{
    itemloop = false;
}

void menu(void);

enum ItemIdx {
    ITEM,
    M_MAIN,
    M_GPOFT,
    M_GPIFT,
//    M_IRQFT,
    M_IIFT,
    M_ITAFT,
    M_ITFT,
    M_SPRFT,
    M_RTCFT,
    M_SRAMFT,
    M_BROMFT,
    M_EEPRFT,
    // 2013/04/10 Add EXT SRAM functions.
    M_EXTSRAMFT,
    M_IFT,
    M_WDTFT,
		M_PULSE01FT,
		M_PULSE02FT,
		M_LAMPFT,
		M_HOPPERFT,
		M_SERIALOUTISAFT,
		M_SERIALOUTPCIFT,	
		M_HPOUTPUTF,
};

static struct FNLIST {
    bool enable;
    void (*fn)();
    unsigned int item_menu;
    unsigned int item_idx;
    char* title;
} fnlist []= {
    {true,menu,         M_MAIN, M_MAIN,   "MAIN"},
    
    {true,tes_exit,     M_MAIN, ITEM,     "Test Exit"},
    {true,gc_info,      M_MAIN, ITEM,     "AGC Info"},
    {true,menu,         M_MAIN, M_GPOFT,  "Gernric Purpose Output       Func. Test"},
    {true,menu,         M_MAIN, M_GPIFT,  "Gernric Purpose Input        Func. Test"},
 /*   
    {true,menu,         M_MAIN,  M_IRQFT, "IRQ Func Tes"},
    {true,menu,         M_IRQFT, M_IIFT,  "Interrupt for Input        Func. Tes"},
    {true,menu,         M_IRQFT, M_ITAFT, "Interrupt for Timer (Auto) Func. Tes"},
    {true,menu,         M_IRQFT, M_ITFT,  "Interrupt for Timer        Func. Tes"},  
    {true,gc_intr_ev_loop, M_IRQFT, ITEM,    "Interrupt Event Tes"},
*/

    {true,menu,         M_MAIN, M_IIFT,  "Interrupt for Input        Func. Test"},
    {true,menu,         M_MAIN, M_ITAFT, "Interrupt for Timer (Auto) Func. Test"},
    {true,menu,         M_MAIN, M_ITFT,  "Interrupt for Timer        Func. Test"},  
    {true,gc_intr_ev_loop, M_MAIN, ITEM,    "Interrupt Event Test"},
    
    {true,menu,            M_MAIN, M_SPRFT,  "SPR Func Test"},
    {true,gc_random_tes,   M_MAIN, ITEM,     "Random Func Test"},
    {true,menu,            M_MAIN, M_RTCFT,  "RTC       Func. Test"},
    {true,menu,            M_MAIN, M_SRAMFT, "SRAM      Func. Test"},
    {true,menu,            M_MAIN, M_BROMFT, "BootROM   Func. Test"},
    {true,menu,            M_MAIN, M_EEPRFT, "EEPROM    Func. Test"},
    // 2013/04/10 Add EXT SRAM functions.
    {true,menu,            M_MAIN, M_EXTSRAMFT, "EXT SRAM      Func. Test"},
    {true,menu,            M_MAIN, M_IFT,    "Intrusion Func. Test"},
    {true,menu,            M_MAIN, M_WDTFT,  "WDT       Func. Test"},
    {true,menu,            M_MAIN, M_PULSE01FT, "Pulse    Func. Test(GA2200)"},
    {true,menu,            M_MAIN, M_PULSE02FT, "Pluse    Func. Test"},
    {true,menu,            M_MAIN, M_LAMPFT,    "Lamp			Func. Test"},
    {true,menu,            M_MAIN, M_HOPPERFT,  "Hopper   Func. Test"},    
    {true,menu,            M_MAIN, M_SERIALOUTISAFT,    "Serial Output Func. Test(GA1200)"},
    {true,menu,            M_MAIN, M_SERIALOUTPCIFT,    "Serial Output Func. Test"},
    {true,menu,            M_MAIN, M_HPOUTPUTF, "HPOutput Func. Test"},    
            
    {true,gc_output_count,    M_GPOFT, ITEM, "Get Number of Output"},
    {true,gc_output_set_pin,  M_GPOFT, ITEM, "Set Pin  On/Off"},
    {true,gc_output_set_pins, M_GPOFT, ITEM, "Set Pins On/Off"},
    {true,gc_output_set_all,  M_GPOFT, ITEM, "Set All  On/Off"},
    {true,gc_output_get_pin,  M_GPOFT, ITEM, "Get Pin  On/Off"},
    {true,gc_output_get_pins, M_GPOFT, ITEM, "Get Pins On/Off"},
    
    {true,gc_input_count,      M_GPIFT, ITEM, "Get Number of Input"},
    {true,gc_input_get_pin,    M_GPIFT, ITEM, "Get Pin  Press/Release"},
    {true,gc_input_get_pins,   M_GPIFT, ITEM, "Get Pins Press/Release"},
    {true,gc_input_set_deb,    M_GPIFT, ITEM, "Set Input Debunce Time"},

    {true,gc_intr_input_count,           M_IIFT, ITEM, "Get The Number of Interrupts for Input"},
    {true,gc_intr_input_set_ena_pin,     M_IIFT, ITEM, "Set PI Pin  Irq En/Dis"},
    {true,gc_intr_input_set_ena_pins,    M_IIFT, ITEM, "Set PI Pins Irq En/Dis"},
    {true,gc_intr_input_set_ena_all,     M_IIFT, ITEM, "Set PI All  Irq En/Dis"},
    {true,gc_intr_input_get_ena_pin,     M_IIFT, ITEM, "Get PI Pin  Irq En/Dis"},
    {true,gc_intr_input_get_ena_pins,    M_IIFT, ITEM, "Get PI Pins Irq En/Dis"},
    {true,gc_intr_input_get_times,       M_IIFT, ITEM, "Get the number of PI Pressed Count"},
    {true,gc_intr_input_get_times_all,   M_IIFT, ITEM, "Get the number of PI Pressed Count (All)"},
    {true,gc_intr_input_set_edge_pin,    M_IIFT, ITEM, "Set PI Pin  Irq Fal/Ris"},
    {true,gc_intr_input_set_edge_pins,   M_IIFT, ITEM, "Set PI Pins Irq Fal/Ris"},
    {true,gc_intr_input_set_edge_all,    M_IIFT, ITEM, "Set PI All  Irq Fal/Ris"},
    {true,gc_intr_input_get_edge_pin,    M_IIFT, ITEM, "Get PI Pin  Irq Fal/Ris"},
    {true,gc_intr_input_get_edge_pins,   M_IIFT, ITEM, "Get PI Pins Irq Fal/Ris"},
    
    {true,gc_intr_autmr_count,         M_ITAFT, ITEM, "Get The Number of Interrupts for AutoTimer"},
    {true,gc_intr_autmr_set_ena_pin,   M_ITAFT, ITEM, "Set Auto Tmr Pin Irq En/Dis"},
    {true,gc_intr_autmr_get_ena_pin,   M_ITAFT, ITEM, "Get Auto Tmr Pin Irq En/Dis"},
    
    {true,gc_intr_timer_count,         M_ITFT, ITEM, "Get The Number of Interrupts for Timer"},
    {true,gc_intr_timer_set_ena_pin,   M_ITFT, ITEM, "Set Timer Pin Irq En/Dis"},
    {true,gc_intr_timer_get_ena_pin,   M_ITFT, ITEM, "Get Timer Pin Irq En/Dis"},
    {true,gc_intr_timer_set_setup,     M_ITFT, ITEM, "Set Timer Setup"},
    {true,gc_intr_timer_get_setup,     M_ITFT, ITEM, "Get Timer Setup"},
    
    {true,gc_spr_get_count,     M_SPRFT, ITEM, "Get The Number of SPR"},
    {true,gc_spr_set_attr_pin,  M_SPRFT, ITEM, "Set SPR Pin Attr"},
    {true,gc_spr_set_attr_pins, M_SPRFT, ITEM, "Set SPR Pins Attr"},
    {true,gc_spr_set_attr_all,  M_SPRFT, ITEM, "Set SPR All Attr"},
    {true,gc_spr_get_attr_pin,  M_SPRFT, ITEM, "Get SPR Pin Attr"},
    {true,gc_spr_get_attr_all,  M_SPRFT, ITEM, "Get SPR All Attr"},
    {true,gc_spr_set_data_pin,  M_SPRFT, ITEM, "Set SPR Pin Data"},
    {true,gc_spr_set_data_pins, M_SPRFT, ITEM, "Set SPR Pins Data"},
    {true,gc_spr_set_data_all,  M_SPRFT, ITEM, "Set SPR All Data"},
    {true,gc_spr_get_data_pin,  M_SPRFT, ITEM, "Get SPR Pin Data"},
    {true,gc_spr_get_data_all,  M_SPRFT, ITEM, "Get SPR All Data"},

    {true,gc_rtc_get_access_status, M_RTCFT, ITEM, "Get RTC Access Status"},
    {true,gc_rtc_get_time,          M_RTCFT, ITEM, "Get RTC Time"},
    {true,gc_rtc_set_time,          M_RTCFT, ITEM, "Set RTC Time"},
    
    {true,gc_sram_get_sz,                   M_SRAMFT, ITEM, "SRAM size"},
    // 2013/04/10 Add memory test range setting item
    {true,gc_sram_set_range,                M_SRAMFT, ITEM, "Set SRAM Test Range"},
    {true,gc_sram_write,                    M_SRAMFT, ITEM, "SRAM Write"},
    {true,gc_sram_write_chk,                M_SRAMFT, ITEM, "SRAM Write Check"},
    {true,gc_sram_write_errbyte,            M_SRAMFT, ITEM, "SRAM Write Error Byte"},
    {true,gc_sram_mirror_setting_mode,  		M_SRAMFT, ITEM, "SRAM Mirror Setting Mode"},
    {true,gc_sram_mirror_start,             M_SRAMFT, ITEM, "SRAM Mirror START"},
    {true,gc_sram_mirror_compare,           M_SRAMFT, ITEM, "SRAM Mirror COMPARE"},
    {true,gc_flash_get_sz,       M_BROMFT, ITEM, "Flash size"},
    // 2013/04/10 Add memory test range setting item
    {true,gc_flash_set_range,    M_BROMFT, ITEM, "Set Flash Test Range"},
    {true,gc_flash_erase_all,    M_BROMFT, ITEM, "Flash Erase All"},
    //{true,gc_flash_erase_sector, M_BROMFT, ITEM, "Flash Erase Sector"},
    {true,gc_flash_write,        M_BROMFT, ITEM, "Flash Write Bytes"},
    {true,gc_flash_read,         M_BROMFT, ITEM, "Flash Write Check"},
    {true,gc_eeprom_write,       M_EEPRFT, ITEM, "EEPROM Write"},
    {true,gc_eeprom_read,        M_EEPRFT, ITEM, "EEPROM Read"},
    // 2013/03/26 Add EEPROM error byte test
    {true,gc_eeprom_write_errbyte,     M_EEPRFT, ITEM, "EEPROM Write Error Byte"},
 		// 2013/04/10 Add EXT SRAM functions.
    {true,gc_ext_sram_get_sz,                   M_EXTSRAMFT, ITEM, "EXT SRAM size"},
    {true,gc_ext_sram_set_range,                M_EXTSRAMFT, ITEM, "Set EXT SRAM Test Range"},
    {true,gc_ext_sram_write,                    M_EXTSRAMFT, ITEM, "EXT SRAM Write"},
    {true,gc_ext_sram_write_chk,                M_EXTSRAMFT, ITEM, "EXT SRAM Write Check"},
    {true,gc_ext_sram_write_errbyte,            M_EXTSRAMFT, ITEM, "EXT SRAM Write Error Byte"}, 
    
    {true,gc_intru_get_log_count,      M_IFT, ITEM, "Get   Intrusion Log Count"},
    {true,gc_intru_rest_time,          M_IFT, ITEM, "Reset Intrusion Time"},
    {true,gc_intru_read_time,          M_IFT, ITEM, "Read  Intrusion Time"},
    {true,gc_intru_init,               M_IFT, ITEM, "Init  Intrusion"},
    {true,gc_intru_load,               M_IFT, ITEM, "Load  Intrusion Log"},
    {true,gc_intru_clear,              M_IFT, ITEM, "Clear Intrusion Log"},
    {true,gc_intru_check,              M_IFT, ITEM, "Check Intrusion Log"},
    {true,gc_intru_door_count,         M_IFT, ITEM, "Get   Intrusion Door count"},
    {true,gc_intru_door_get_status,    M_IFT, ITEM, "Get   Intrusion Door status"},
    
    
    {true,gc_wdt_set_ena,     M_WDTFT, ITEM, "Set WDT Enable"},
    {true,gc_wdt_set_rst,     M_WDTFT, ITEM, "Set WDT Reset"},
    {true,gc_wdt_set_currval, M_WDTFT, ITEM, "Set WDT Curr Value"},
    {true,gc_wdt_get_currval, M_WDTFT, ITEM, "Get WDT Curr Value"},
    
    {true,gc_pulse_get_count,			M_PULSE01FT, ITEM, "Get numbers of Pulse."},
    {true,gc_pulse_set_irq,			 	M_PULSE01FT, ITEM, "Enable/Disable Pulse interrupt."},
    {true,gc_pulse_set_SW_code, 	M_PULSE01FT, ITEM, "Set Pulse mode."},    
    {true,gc_pulse_set_Fre,			 	M_PULSE01FT, ITEM, "Set frequence of Pulse."},
    {true,gc_pulse_add_counter, 	M_PULSE01FT, ITEM, "Add Pulse Counter."},   
		{true,gc_pulse_get_counter, 	M_PULSE01FT, ITEM, "Get unsent Counter of Pulse."},
		
    {true,gc_pulse_get_count,			M_PULSE02FT, ITEM, "Get numbers of Pulse."},
    {true,gc_pulse_set_irq,			 	M_PULSE02FT, ITEM, "Enable/Disable Pulse interrupt."},
    {true,gc_pulse_set_mode,		 	M_PULSE02FT, ITEM, "Set Pulse mode."},    
    {true,gc_pulse_set_Fre,			 	M_PULSE02FT, ITEM, "Set frequence of Pulse."},
    {true,gc_pulse_add_counter, 	M_PULSE02FT, ITEM, "Add Pulse Counter."},   
		{true,gc_pulse_get_counter, 	M_PULSE02FT, ITEM, "Get unsent Counter of Pulse."},  

		// Serial Output	
    {true,gc_serialoutput_set_SW_code, 		M_SERIALOUTISAFT, ITEM, "Set switch code of serial ouput."},    
    {true,gc_serialoutput_set_value,			M_SERIALOUTISAFT, ITEM, "Set serial output value."},        

		// Serial Output	
    {true,gc_serialoutput_set_pulse_mode, 		M_SERIALOUTPCIFT, ITEM, "Set pulse mode of serial ouput."},    
    {true,gc_serialoutput_set_value,					M_SERIALOUTPCIFT, ITEM, "Set serial output value."},      
    		
		// Flash setting of Lamp.
    {true,gc_lamp_flash_get_ena_pin,		M_LAMPFT, ITEM, "Get flash pin of Lamp En/Dis."},
    {true,gc_lamp_flash_set_ena_pin,		M_LAMPFT, ITEM, "Set flash pin of Lamp En/Dis."},
    {true,gc_lamp_flash_get_ena_pins,		M_LAMPFT, ITEM, "Get flash pins of Lamp En/Dis."},
    {true,gc_lamp_flash_set_ena_pins,		M_LAMPFT, ITEM, "Set flash pins of Lamp En/Dis."},
    {true,gc_lamp_flash_get_value,			M_LAMPFT, ITEM, "Get flash value."},
    {true,gc_lamp_flash_set_value,			M_LAMPFT, ITEM, "Set flash value."},    
    // Brightness setting of Lamp
    {true,gc_lamp_brightness_get_ena_pin,			M_LAMPFT, ITEM, "Get brightness pin of Lamp En/Dis."},
    {true,gc_lamp_brightness_set_ena_pin,			M_LAMPFT, ITEM, "Set brightness pin of Lamp En/Dis."},
    {true,gc_lamp_brightness_get_ena_pins,		M_LAMPFT, ITEM, "Get brightness pins of Lamp En/Dis."},
    {true,gc_lamp_brightness_set_ena_pins,		M_LAMPFT, ITEM, "Set brightness pins of Lamp En/Dis."},
    {true,gc_lamp_brightness_get_value,				M_LAMPFT, ITEM, "Get brightness value."},
    {true,gc_lamp_brightness_set_value,				M_LAMPFT, ITEM, "Set brightness value."},    
    // Brightness setting of Lamp
    {true,gc_individlamp_get_count,									M_LAMPFT, ITEM, "Get numbers of Lamp which they may adjust their brightness."},    
    {true,gc_individlamp_brightness_get_ena_pin,		M_LAMPFT, ITEM, "Get brightness pin of individual Lamp En/Dis."},
    {true,gc_individlamp_brightness_set_ena_pin,		M_LAMPFT, ITEM, "Set brightness pin of individual Lamp En/Dis."},
    {true,gc_individlamp_brightness_get_ena_pins,		M_LAMPFT, ITEM, "Get brightness pins of individual Lamp En/Dis."},
    {true,gc_individlamp_brightness_set_ena_pins,		M_LAMPFT, ITEM, "Set brightness pins of individual Lamp En/Dis."},
    {true,gc_individlamp_brightness_get_value,			M_LAMPFT, ITEM, "Get brightness value of individual lamp."},
    {true,gc_individlamp_brightness_set_value,			M_LAMPFT, ITEM, "Set brightness value of individual lamp."},    
    
		// Hopper.
    {true,gc_hopper_get_count,						M_HOPPERFT, ITEM, "Get numbers of Hopper."},  
    {true,gc_hopper_set_pulse_mode,				M_HOPPERFT, ITEM, "Set pulse mode of hopper."},
    {true,gc_hopper_get_counter,					M_HOPPERFT, ITEM, "Get hopper's counter."},  
    {true,gc_hopper_set_counter,					M_HOPPERFT, ITEM, "Set hopper's counter."},    
    {true,gc_hopper_get_irq_ena_pin,			M_HOPPERFT, ITEM, "Get hopper interrupt pin En/Dis."},
    {true,gc_hopper_set_irq_ena_pin,			M_HOPPERFT, ITEM, "Set hopper interrupt pin En/Dis."},
    {true,gc_hopper_reset,								M_HOPPERFT, ITEM, "Reset hopper."},
    {true,gc_hopper_set_restart,					M_HOPPERFT, ITEM, "Restart hopper."},
    {true,gc_hopper_set_pause,						M_HOPPERFT, ITEM, "pause hopper."},  
    {true,gc_hopper_get_state,						M_HOPPERFT, ITEM, "Get state of hopper."},      
    
    // HPMotorOut
    {true,gc_hpmotorout_get_count,						M_HPOUTPUTF, ITEM, "Get numbers of HP Motor Output."},    
    {true,gc_hpmotorout_get_ena_pin,					M_HPOUTPUTF, ITEM, "Get HP Motor Output pin En/Dis."},
    {true,gc_hpmotorout_set_ena_pin,					M_HPOUTPUTF, ITEM, "Set HP Motor Output pin En/Dis."}, 
                
};

void charline(unsigned int inp, char a)
{
    int i;
    
    for (i = 0; i< inp;i++)
    {
        printf("%c",a);
    }
    printf("\n");
}

static unsigned int maxlst;
static unsigned int handle;
#define BUFFERSIZE	1024

void Update_fnlist_SerialOutput_Enbled_flag()
{
    unsigned char ucCountBuffer[BUFFERSIZE];
    unsigned char ucDriverType;
    int i;

    //```read param to buffer
    FILE* fp = fopen("FPGAParameter.txt","rb\n ");
    if (fp == NULL) {
        printf("!!! open FPGAParameter.txt Failure !!!\n ");
        return;
    }

    if (fread(ucCountBuffer,sizeof(unsigned char),BUFFERSIZE,fp) == 0) {
        printf("!!! read FPGAParameter.txt Failure !!!\n ");
        return;
    }

    fclose(fp); //```read param to buffer		
    
    #define DRIVER_TYPE_FLAG_POS		34
    ucDriverType = ucCountBuffer[DRIVER_TYPE_FLAG_POS];	
    if(ucDriverType == 0x01)
    {
        for ( i = 0; i < maxlst; i++ )
        {
	    // 2013/03/28 Debug - serialoutput interface.  	
            if (( fnlist[i].item_menu == M_SERIALOUTPCIFT) || ( fnlist[i].item_idx == M_SERIALOUTPCIFT))
            //if (( fnlist[i].item_menu == M_SERIALOUTISAFT) || ( fnlist[i].item_idx == M_SERIALOUTISAFT))
            {
                fnlist[i].enable = false;
            }
        }									    		
    }else
    {
        for ( i = 0; i < maxlst; i++ )
        {
	    // 2013/03/28 Debug - serialoutput interface.  	 
            if (( fnlist[i].item_menu == M_SERIALOUTISAFT) || ( fnlist[i].item_idx == M_SERIALOUTISAFT))
            //if (( fnlist[i].item_menu == M_SERIALOUTPCIFT) || ( fnlist[i].item_idx == M_SERIALOUTPCIFT)) 
           {
                fnlist[i].enable = false;
            }
        }				
    }				
}

void Update_fnlist_Enbled_flag()
{
		unsigned char ucCount, ucPulseType;
		unsigned long ulSize; 
		bool bFlag;
		int i;
		
		maxlst =  sizeof(fnlist) / sizeof (struct FNLIST);
		
		_GetFPGAOCount(&ucCount);
		if(ucCount == 0)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_GPOFT) || ( fnlist[i].item_idx == M_GPOFT))
            {
                fnlist[i].enable = false;
            }
        }				
		}	
		
		_GetFPGAICount(&ucCount);
		if(ucCount == 0)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_GPIFT) || ( fnlist[i].item_idx == M_GPIFT))
            {
                fnlist[i].enable = false;
            }
        }				
		}	
		
		_GetIrqPICount(&ucCount);
		if(ucCount == 0)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_IIFT) || ( fnlist[i].item_idx == M_IIFT))
            {
                fnlist[i].enable = false;
            }
        }				
		}			
			
		_AutoTimerEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_ITAFT) || ( fnlist[i].item_idx == M_ITAFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}		
		
		// 2013/03/28 Reset function list enable flag of Random number.
		_RandomEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if ( strcmp(fnlist[i].title, "Random Func Test") == 0) 
            {
                fnlist[i].enable = false;
            }
        }									
		}		

		_TimerEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_ITFT) || ( fnlist[i].item_idx == M_ITFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}			
		
		_SPREnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_SPRFT) || ( fnlist[i].item_idx == M_SPRFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_RTCEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_RTCFT) || ( fnlist[i].item_idx == M_RTCFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_GetRamSize(&ulSize);
		if(ulSize == 0)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_SRAMFT) || ( fnlist[i].item_idx == M_SRAMFT))
            {
                fnlist[i].enable = false;
            }
        }				
		}else
		{
			_GetRamMirrorEn(&bFlag);
			if(!bFlag)
			{
				for ( i = 0; i < maxlst; i++ )
				{
				    if (( fnlist[i].item_menu == M_SRAMFT) && (strncmp(fnlist[i].title, "SRAM Mirror", 11) == 0))
				    {
					fnlist[i].enable = false;
				    }
				}
			}				
		}		
		
		_FlashEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_BROMFT) || ( fnlist[i].item_idx == M_BROMFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_EEPROMEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_EEPRFT) || ( fnlist[i].item_idx == M_EEPRFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
				
		_IntrusionEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_IFT) || ( fnlist[i].item_idx == M_IFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_WDTEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_WDTFT) || ( fnlist[i].item_idx == M_WDTFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_PulseEnable(&bFlag);
		if(bFlag)
				_GetPulseTypeCode(&ucPulseType);
		else
				ucPulseType = 0x0;
		
		if((ucPulseType == 0x0) || (ucPulseType == 0x1))
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_PULSE01FT) || ( fnlist[i].item_idx == M_PULSE01FT))
            {
                fnlist[i].enable = false;
            }
        }									
		}					

		if((ucPulseType == 0x0) || (ucPulseType == 0x2))
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_PULSE02FT) || ( fnlist[i].item_idx == M_PULSE02FT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
		
		_LampEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_LAMPFT) || ( fnlist[i].item_idx == M_LAMPFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
	
		_HopperEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_HOPPERFT) || ( fnlist[i].item_idx == M_HOPPERFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	
	
		// 2013/03/28 Reset function list enable flag of SerialOutput.	
		_SerialOutputEnable(&bFlag);
		if(!bFlag)
		{
	for ( i = 0; i < maxlst; i++ )
	{			
	    if (( fnlist[i].item_menu == M_SERIALOUTISAFT) || ( fnlist[i].item_idx == M_SERIALOUTISAFT) || ( fnlist[i].item_menu == M_SERIALOUTPCIFT) || ( fnlist[i].item_idx == M_SERIALOUTPCIFT))
	    {
	         fnlist[i].enable = false;
	    }
	}
		}else
	    {
	    	Update_fnlist_SerialOutput_Enbled_flag();
	    }

		_HPMotorOut(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_HPOUTPUTF) || ( fnlist[i].item_idx == M_HPOUTPUTF))
            {
                fnlist[i].enable = false;
            }
        }									
		}	    	
    	
		// 2013/04/10 Add EXT SRAM functions.
		_ExtSRAMEnable(&bFlag);
		if(!bFlag)
		{
        for ( i = 0; i < maxlst; i++ )
        {
            if (( fnlist[i].item_menu == M_EXTSRAMFT) || ( fnlist[i].item_idx == M_EXTSRAMFT))
            {
                fnlist[i].enable = false;
            }
        }									
		}	    			 																
}


void menu(void)
{  
    unsigned int idx;
    unsigned int menu;
    unsigned int handle_inp;
    
    int i;
    
    // 算出總選項數
    maxlst =  sizeof(fnlist) / sizeof (struct FNLIST);
    
    charline(6+ strlen(fnlist[handle].title),'=');     // 我是字元線
    printf("%03d - %s\n",handle,fnlist[handle].title); // 本層目錄選項
    charline(6+ strlen(fnlist[handle].title),'=');     // 我是字元線   
    
    // 取來找出所屬項目
    idx = fnlist[handle].item_idx;
    
    // 取得該層之上一層選單代號
    menu = fnlist[handle].item_menu;
    
    
    // 印出返迴選項 [[[
    if (idx != M_MAIN) // 選單在第三層(含)以上時
    {
        // 找出上一層選單項
        for ( i = 0; i < maxlst; i++ )
        {
            if ( fnlist[i].item_idx == fnlist[handle].item_menu)
            {
                break;
            }
        }
        printf("%03d + %s\n",i,fnlist[i].title);    // 回上一層選項
        if (handle != 0 && menu != M_MAIN)
        {
            printf("%03d + %s\n",0,fnlist[0].title);    // 回主目錄選項
        }
    }
    // 印出返迴選項 ]]]
    
    // 每個選單都要提示的離開選項
    printf("%03d : %s\n",1,fnlist[1].title);
    
    // 印出該選單所屬的項目, 從處理代號2開始
    for (i= 2; i< maxlst; i++)
    {
        if ((fnlist[i].enable == true) && (fnlist[i].item_menu == idx))
        {
            if (fnlist[i].item_idx == ITEM)
            {
                printf("%03d : %s\n",i,fnlist[i].title);    
            } else
            {
                printf("%03d + %s\n",i,fnlist[i].title);    
            }
        }
    }
    
    // 輸入選項代號
    printf("Input Function No: "); scanf("%u",&handle_inp); printf("\n");
    
    // 判斷輸入選項是否正確
    if (handle_inp >= maxlst) // 保待現在的選單處理代號
    {
        printf("\terr: Handle Number Over Range.\n");
    } 
    else // 變更為所選擇的處理代號
    {
        handle = handle_inp;
    }	
}

int main(int argc, char* argv[])
{    
    unsigned int dwError = 0;
    int i; 
    
    if(_FPGAInit(&dwError, InterruptFun) == false)
    {
        printf("\tNO AEWIN Gaming Controller Work.\n");
        return -1;
    }
    
    // Select test items.
    Update_fnlist_Enbled_flag(); 

    tes_about();		
  
    // 進入測試選單迴圈
    itemloop = true;
    while(itemloop)
    {
        // 確認測試項是否關閉
        if (fnlist[handle].enable == false)
        {
            printf("\tITEM DISABLED\n");
            // 找出除能項目之所屬選單項目
            for ( i = 0; i < maxlst; i++ )
            {
                if ( fnlist[i].item_idx == fnlist[handle].item_menu)
                {
                    handle = i;
                    break;
                }
            }
            continue;
        }
        
        if (fnlist[handle].item_idx == ITEM)
        {
            // 執行測試項 (呼叫測試函式)
            fnlist[handle].fn();
            if (handle != 1) printf("\n");
            
            // 找出呼叫函式執行完成後之所屬選單項
            for ( i = 0; i < maxlst; i++ )
            {
                if ( fnlist[i].item_idx == fnlist[handle].item_menu)
                {
                    handle = i;
                    break;
                }
            }
        }
        else
        {
            // 執行選單項 (呼叫選單函式)
            fnlist[handle].fn();
        }
    } 
    
    //  卸除開啟的 AGC 裝置
    if (_FPGAUnInit())
    {
        printf("\t UnrRegistered.\n", i);
    }
        
    return 0;
}
