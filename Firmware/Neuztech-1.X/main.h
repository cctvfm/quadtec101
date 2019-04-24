/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAIN
#define	MAIN

#include <xc.h> // include processor files - each processor file is guarded.  

// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    
    

// DSPIC30F3012 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FOSFPR = FRC_PLL16       // Oscillator (FRC w/PLL 16x)
#pragma config FCKSMEN = CSW_ON_FSCM_OFF// Clock Switching and Monitor (Sw Enabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_OFF         // PBOR Enable (Disabled)
#pragma config MCLRE = MCLR_DIS          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>






    

    void __attribute__((__interrupt__,__auto_psv__))  _T1Interrupt(void);
    unsigned int ReadAIN (unsigned char channel);
    void chipconfig (void);
    unsigned long int IntToFreq (unsigned int value);
    void PutSPIByte (unsigned char data);
    void PutSPIByte16 (unsigned int data);
    //void WriteDac(signed int data1, signed int data2, signed int data3, signed int data4);
    void WriteDac(unsigned char DAC,signed int data1, signed int data2);
    signed int getSample (unsigned int index, unsigned int blend);
    signed int squarewave (unsigned int acc);
    signed int sawwave (unsigned int acc);
    signed int vgamewave (unsigned int acc);
    signed int triwave (unsigned int acc);
    signed int getAverage(signed int * array,unsigned char length);
    signed int Scale(signed int sample,unsigned int scalevalue);
    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 
    
    unsigned int voctscale[410]= {0,111,222,334,445,557,669,781,893,1006,1119,1232,1345,1458,1572,1685,1799,1913,2027,2142,2257,2371,2486,2602,2717,2833,2949,3065,3181,3297,3414,3531,3648,3765,3882,4000,4118,4236,4354,4472,4591,4710,4829,4948,5067,5187,5307,5427,5547,5667,5788,5909,6030,6151,6273,6394,6516,6638,6760,6883,7006,7128,7252,7375,7498,7622,7746,7870,7995,8119,8244,8369,8494,8620,8745,8871,8997,9123,9250,9376,9503,9630,9758,9885,10013,10141,10269,10398,10526,10655,10784,10914,11043,11173,11303,11433,11563,11694,11825,11956,12087,12219,12350,12482,12615,12747,12880,13012,13145,13279,13412,13546,13680,13814,13949,14083,14218,14353,14489,14624,14760,14896,15032,15169,15305,15442,15580,15717,15855,15993,16131,16269,16408,16546,16686,16825,16964,17104,17244,17384,17525,17666,17806,17948,18089,18231,18373,18515,18657,18800,18943,19086,19229,19373,19517,19661,19805,19950,20094,20240,20385,20530,20676,20822,20969,21115,21262,21409,21556,21704,21852,22000,22148,22296,22445,22594,22744,22893,23043,23193,23343,23494,23645,23796,23947,24099,24251,24403,24555,24708,24861,25014,25167,25321,25475,25629,25783,25938,26093,26248,26404,26559,26715,26872,27028,27185,27342,27500,27657,27815,27973,28132,28290,28449,28608,28768,28928,29088,29248,29409,29569,29731,29892,30054,30216,30378,30540,30703,30866,31029,31193,31357,31521,31685,31850,32015,32180,32346,32512,32678,32844,33011,33178,33345,33513,33680,33848,34017,34185,34354,34524,34693,34863,35033,35203,35374,35545,35716,35888,36060,36232,36404,36577,36750,36923,37097,37271,37445,37619,37794,37969,38144,38320,38496,38672,38849,39026,39203,39380,39558,39736,39914,40093,40272,40451,40631,40810,40991,41171,41352,41533,41714,41896,42078,42260,42443,42626,42809,42993,43177,43361,43545,43730,43915,44101,44286,44472,44659,44845,45032,45220,45407,45595,45783,45972,46161,46350,46540,46730,46920,47110,47301,47492,47684,47876,48068,48260,48453,48646,48840,49033,49227,49422,49617,49812,50007,50203,50399,50595,50792,50989,51186,51384,51582,51781,51979,52179,52378,52578,52778,52978,53179,53380,53582,53783,53986,54188,54391,54594,54798,55001,55206,55410,55615,55820,56026,56232,56438,56645,56852,57059,57267,57475,57683,57892,58101,58310,58520,58730,58941,59152,59363,59575,59787,59999,60212,60425,60638,60852,61066,61280,61495,61710,61926,62142,62358,62575,62792,63009,63227,63445,63664,63883,64102,64321,64541,64762,64983,65204,65425};
    unsigned int octavemultipler[12]={1,2,4,8,16,32,64,128,256,512,1024,2048};
    //unsigned int scale[410];
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

