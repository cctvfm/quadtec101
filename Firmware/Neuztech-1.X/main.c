/*
 * File:   main.c
 * Author: filip
 *
 * Created on February 5, 2019, 12:38 PM
 */

#define FOSC (120000000ULL)
#define FCY FOSC/2
#include <libpic30.h>
#include "main.h"
#include "waveforms.h"
#include <math.h>
#include <dsp.h>


#define AIN TRISBbits.TRISB0

#define NEUZOUT LATDbits.LATD9

#define TWELVEBITMODE   //change in waveforms.h too!

//#define EIGHTBITMODE


#define SWIRL_CHANNEL       0b000
#define RELATION_CHANNEL    0b001
#define OSC1_CHANNEL        0b010
#define OSC2_CHANNEL        0b011
#define WAV1_CHANNEL        0b100
#define WAV2_CHANNEL        0b101
#define OSC1CV_CHANNEL      0b110  
#define OSC2CV_CHANNEL      0b111
#define ADCSAMPLES 10   //number of samples to be taken and averaged
                        //higher number means less jitter, but longer response rate

#define _XTAL_FREQ 120000000

#define SAMPLERATE 30000
#define TIMER1_RELOAD 750   //40KHz sample rate tested //750 for 40k, 1000 for 30k
#define HZPHASOR    108420     //108420 for 40K tested 144560 for 30k untested
#define LFOPHASORSCALE  100 //must be much slower than the hz phasor
                            //multiply by 4095/2 to get max swirl speed, about 2Hz? maybe too fast
                            //untested

//#define F0 16.35 //lowest possible note, all others are calculated from this one - Low C
#define F0 14 //make it a nice integer, should still be able to tune?
#define A 1.059463094   //power factor for semitones to frequency conversion
#define SINE 1
#define TRI 2
#define SQUARE 3

unsigned long int phaseaccumulator=0;
unsigned long int phaseaccumulator2 = 0;
unsigned long int phaseaccumulator3 = 0;
unsigned long int phaseaccumulator4 = 0;

unsigned long int lfoaccumulator=0;

unsigned int swirler[4];

unsigned long int phasor1;
unsigned long int phasor2;

unsigned long int lfophasor = LFOPHASORSCALE*100;
unsigned char lfodirection = 0;
#define NONE 0
#define LEFT 1
#define RIGHT 2

unsigned char VCAMODE=0;
#define OFF 0
#define ON 1
unsigned int waveshape1;
unsigned int waveshape2;
int waveshape1samples[ADCSAMPLES];

unsigned int relation = 0;

unsigned int SwirlH,SwirlL;
unsigned int AnalogInputs[10]={0,0,0,0,0,0,0,0,0,0};
unsigned int LastAnalogInputs[10]={0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
unsigned char currentMUXChannel = 0;
unsigned long int counter=0;
unsigned long int samplecounter = 0;
unsigned long int setpoint = 1000;


#define currentSWIRL AnalogInputs[0]
#define currentRELATION AnalogInputs[1]
#define currentOSC1 AnalogInputs[2]
#define currentOSC2 AnalogInputs[3]
#define currentWAV1 AnalogInputs[4]
#define currentWAV2 AnalogInputs[5]
#define currentVOCT1 AnalogInputs[6]
#define currentVOCT2 AnalogInputs[7]
#define currentPANCV AnalogInputs[8]
#define currentNEUZ  AnalogInputs[9]

#define lastSWIRL LastAnalogInputs[0]
#define lastRELATION LastAnalogInputs[1]
#define lastOSC1 LastAnalogInputs[2]
#define lastOSC2 LastAnalogInputs[3]
#define lastWAV1 LastAnalogInputs[4]
#define lastWAV2 LastAnalogInputs[5]
#define lastVOCT1 LastAnalogInputs[6]
#define lastVOCT2 LastAnalogInputs[7]
#define lastPANCV LastAnalogInputs[8]
#define lastNEUZ  LastAnalogInputs[9]


unsigned int DACA,DACB,DACC,DACD;



//Interrupts 40,000 times per second.
void __attribute__((__interrupt__,__auto_psv__)) __attribute__((optimize("-O0")))  _T1Interrupt(void)
{

   
    
    signed int currentsample;
    unsigned int rando;
    
    

    
    if (IFS0bits.T1IF == 1)   //synth engine routine
    {
        IEC0bits.T1IE = 0;
        WriteDac(1,DACA,DACB);  //write the last calculated sample so we're staying true to our sample rate
        WriteDac(2,DACC,DACD);
        
        PR1 = TIMER1_RELOAD;
        IFS0bits.T1IF = 0;          //Have these three lines at end for function forgiveness,
        IEC0bits.T1IE = 1;          //put them at the start for sample rate accuracy
         
        samplecounter++;
         
        phaseaccumulator = phaseaccumulator + phasor1;
        phaseaccumulator2= phaseaccumulator2 + phasor2;

        if(lfodirection == LEFT)
            lfoaccumulator = lfoaccumulator - lfophasor;    //for the swirler
        else if(lfodirection == RIGHT)
            lfoaccumulator = lfoaccumulator + lfophasor;    //for the swirler
        
        //currentsample = getSample(phaseaccumulator>>22,waveshape1);
        //currentsample = currentsample + getSample(phaseaccumulator3>>22,waveshape2);
        
        //currentsample = currentsample /2;
        
        currentsample = Scale(getSample(phaseaccumulator>>22,waveshape1),1023-relation) 
                    // +getSample(phaseaccumulator2>>22,waveshape1)
                       +Scale(getSample(phaseaccumulator2>>22,waveshape2),relation)
                    // +getSample(phaseaccumulator3>>22,waveshape2)
                        ;
        
        if(lfodirection != NONE)
        {
            DACA = Scale(currentsample,swirler[0]);
            DACB = Scale(currentsample,swirler[1]);
            DACC = Scale(currentsample,swirler[2]);
            DACD = Scale(currentsample,swirler[3]);
        }
        else if(VCAMODE == ON)
        {
            DACA = Scale(currentsample,currentPANCV>>2);
            DACB = Scale(currentsample,currentPANCV>>2);
            DACC = Scale(currentsample,currentPANCV>>2);
            DACD = Scale(currentsample,currentPANCV>>2);
        }
        else
        {
            DACA = currentsample;
            DACB = currentsample;
            DACC = currentsample;
            DACD = currentsample;
        }
        //DACC = getSample(phaseaccumulator3>>22,waveshape2);
        //DACD = DACC;
        //DACB = getSample(phaseaccumulator>>22,waveshape1);
        //DACA = DACA + DACB;
        //DACA = DACA>>1;
        //DACB=DACA;
        //to do, when adding these two waveforms, weird stuff happens.
        //edit, i think it's because of phasor discrepancy, if the interval is 0, phasors need to be identical.

        
        
        

        
        if(counter > setpoint)
        {
            rando= rand() % 2;
            if(rando==1)
                NEUZOUT = 1;
            else
                NEUZOUT = 0;

            counter = 0;
        }

        
        counter++;
        
        /*
        PR1 = TIMER1_RELOAD;
        IFS0bits.T1IF = 0;          //Have these three lines at end for function forgiveness,
        IEC0bits.T1IE = 1;          //put them at the start for sample rate accuracy
        */
        
    }
    

            
            
}


int __attribute__((optimize("-O0"))) main(void) {
    
    chipconfig();
    unsigned int fun;
    unsigned char x;

    unsigned int y;
    unsigned int totalADC;
    unsigned int testswirler;

    unsigned int loopcounter = 0;
    unsigned long int templong;

    unsigned int calculatedSWIRL;
    unsigned int VCACOUNTER = 0;
    unsigned long int VCAENTRY = 0;
    unsigned char VCAENTRYFLAG = 0;
    
    
    //grab all the inputs real quick first.
    for(x=0;x<=9;x++)
    {
        AnalogInputs[loopcounter]=ReadAIN(loopcounter);
    }
    
    T1CONbits.TON = 1;  //start sampling timer
    

    fun = 0;    //no more fun
    
    while(1)
    {
           

        AnalogInputs[loopcounter]=ReadAIN(loopcounter);
        
        loopcounter++;
        
        if(loopcounter>9)
            loopcounter = 0;
        
        
        
        //compute 4 scale factors for swirler
        y=lfoaccumulator>>22;
        for(x=0;x<4;x++)
        {
            if(triwave(y)>0)
            {   testswirler = triwave(y);
                testswirler = testswirler >> 1;
                
                swirler[x]=testswirler;
            }
            else
                swirler[x] = 0;
            
            
            y=y+256;
            if(y>1023)
                y=y-1023;
        }

        #define UPPERVCALIMIT 2207
        #define LOWERVCALIMIT 1887

        #define VCACOUNTLIMIT 2000
        #define VCASAMPLEDELAY 40000
        if(currentSWIRL != lastSWIRL || currentPANCV != lastPANCV)
        {

            
            
            //max is 4095
            //mid is 2047
            if(currentSWIRL<=UPPERVCALIMIT && currentSWIRL >= LOWERVCALIMIT)    //we're in VCA MODE!
            {
                
                if(VCAENTRYFLAG == 0 )
                {
                    VCAENTRYFLAG = 1;
                    VCAENTRY = samplecounter;

                
                }

            }
            else
            {
                VCAENTRYFLAG = 0;
            }
            if(VCAENTRYFLAG != 2)
            {
                
                
                calculatedSWIRL = currentSWIRL + currentPANCV;
                calculatedSWIRL = calculatedSWIRL;
 
                
                
                if(calculatedSWIRL>2047)
                {
                    templong = (calculatedSWIRL -2047);   //0-2000 slowest to fastest good
                    templong = templong * LFOPHASORSCALE;
                    lfodirection = LEFT;
                    lfophasor = templong;

                    VCAMODE = OFF;
                }
                else if(calculatedSWIRL<=2047)
                {
                    templong = 2048 - calculatedSWIRL;    //0-2000 fastest to slowest good

                    templong = templong * LFOPHASORSCALE;
                    lfodirection = RIGHT;
                    lfophasor = templong;

                    VCAMODE = OFF;
                }
                    
                
                
            }

            

 
            lastSWIRL = currentSWIRL;
            lastPANCV = currentPANCV;
        }
        if (VCAENTRYFLAG == 1)
        {

            if((samplecounter - VCAENTRY)>VCASAMPLEDELAY)
            {
                VCAENTRYFLAG = 2;
                lfodirection = NONE;
                lfophasor = 0;
            }

        }
         
        if(lfodirection == NONE)
        {
           if(currentPANCV > 300)
           VCAMODE = ON;
        }
        
        if((currentVOCT1 != lastVOCT1) || (currentOSC1 != lastOSC1))
        {
            
            totalADC = currentOSC1 + currentVOCT1;
            
            phasor1 = IntToFreq(totalADC);
            
            
            lastVOCT1 = currentVOCT1;
            lastOSC1 = currentOSC1;
            
        }
        
        if(currentRELATION != lastRELATION)
        {

            
            relation = currentRELATION/4;
            lastRELATION = currentRELATION;
             
        }
        
        if((currentVOCT2 != lastVOCT2) || (currentOSC2 != lastOSC2))
        {
            totalADC = currentOSC2 + currentVOCT2;
            
            phasor2 = IntToFreq(totalADC);
           
            lastVOCT2 = currentVOCT2;
            lastOSC2 = currentOSC2;
            
            
            lastRELATION = currentRELATION;
        }

        waveshape1=currentWAV1;
        waveshape2=currentWAV2;
        
        setpoint = 0xFFF-currentNEUZ;

      
        }
        
   
    

    
    return 0;
}

unsigned long int __attribute__((optimize("-O0"))) IntToFreq (unsigned int value)
{
    unsigned int octave,pitch;
    unsigned long frequencyint;

    
    octave = value / 819;    //octave now contains the octave above the lowest
    pitch = value % 819;     //pitch now contains the value between 0 and 819 which is the range between octaves
    //819 refers to 1V at the 0-4095 / 0-5V scale
    
    //what we're doing is F0*a^(semitone) * HZPHASOR
    //the exponent part is calculated with a lookup table to avoid the slow powf function
    //to reduce size, we're only saving a single octave of the exponent part, then multiplying by the number of octaves
    
    frequencyint = voctscale[(pitch>>1)];
    //frequencyint = frequencyint * HZPHASOR;
    //frequencyint = frequencyint >> 16;    //won't fit in 32 bit
    frequencyint = (frequencyint>>1)*HZPHASOR;   //lose an lsb this way.
    frequencyint = frequencyint >> 15;
    frequencyint = frequencyint + HZPHASOR; //fixed point means we lose the original 1.xxx so we add it back

    frequencyint = frequencyint * octavemultipler[octave];   //multiply by powers of 2, eg 1, 2, 4, 8 - saved in a lookup table
    return (frequencyint * F0);
    
}
unsigned int ReadAIN (unsigned char channel)
{
    ADCHS=channel&0x0F;
    ADPCFG=~((unsigned int)1<<channel);
    ADCON1 = 0x00E0;
    ADCON2 = 0x0000; //channel 0
    //ADCON3 = 0X1F3F;
    ADCON3 = 0x0808;
    
    
    
    ADCSSL=0;
    
    ADCON1bits.ADON = 1;
    ADCON1bits.SAMP = 1;
    
    while(!ADCON1bits.DONE);
    return ADCBUF0;
}


void chipconfig (void)
{
    //code protection sequence
    OSCCONL = 0x46;
    OSCCONL = 0x57;
    OSCCONL = 0x00;//real osccon value here
    OSCCONH = 0x78;
    OSCCONH = 0x9A;
    OSCCONH = 0x03;//real oscconh here
    //write oscconh here
    
    while(OSCCONbits.LOCK ==0);
    
    
    U1MODEbits.UARTEN=0;

    T1CON = 0;
    TMR1 = 0;

    IPC0bits.T1IP = 1;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0;
    INTCON1 = 0x8000;
    INTCON2 = 0x0000;

    
    AIN = 1;

    
    //SPI SETUP
    
    TRISFbits.TRISF6 = 0;   //sck
    TRISFbits.TRISF3 = 0;   //sdo
    TRISFbits.TRISF4 = 0;   //cs2
    TRISDbits.TRISD8 = 0;   //cs1
    TRISFbits.TRISF5 = 0;  //ldac

    
    TRISDbits.TRISD9 = 0;   //Neuz out
    
    //AIN 0-9
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB3 = 1;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB6 = 1;
    TRISBbits.TRISB7 = 1;
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB9 = 1;
    
    #define CS1 LATDbits.LATD8
    #define CS2 LATFbits.LATF4
    #define LDAC LATFbits.LATF5
    LDAC = 1;
    CS1 = 1;
    CS2 = 1;
    SPI1STATbits.SPIROV = 0;
    SPI1CON = 0x003F;
    SPI1CONbits.CKE = 0;
    SPI1CONbits.CKP = 1;
    //SPI1CONbits.MODE16 = 1;
    SPI1STATbits.SPIEN = 1;
    
    //Sample Rate Timer setup (timer1)
    
    
    
}

void __attribute__((optimize("-O0"))) PutSPIByte (unsigned char data)
{
    unsigned char temp;    
    temp = SPI1BUF;
    Nop();
    SPI1BUF = data;
    while(SPI1STATbits.SPITBF ==0);
}

void __attribute__((optimize("-O0"))) PutSPIByte16 (unsigned int data)
{
    unsigned int temp;    
    temp = SPI1BUF;
    Nop();
    SPI1BUF = data;
    while(SPI1STATbits.SPITBF ==0);
}

void __attribute__((optimize("-O0"))) WriteDac(unsigned char DAC,signed int data1, signed int data2)
{
    data1=data1+2047;
    data2=data2+2047;
    
    if(data1 < 0)
        data1=0;
    if(data2 < 0)
        data2=0;
    #define SPIdelay() Nop(); Nop(); Nop();
    unsigned char highbyte,lowbyte;
    //0001 is high gain
    //0011 is low gain
    lowbyte = data1 & 0xff;
    highbyte = (data1 >> 8);
    highbyte = highbyte | 0b00010000;

    if(DAC == 1)
        CS1 = 0;
    else
        CS2 = 0;
    Nop();
    PutSPIByte (highbyte);
    PutSPIByte(lowbyte);

    Nop();
    if(DAC == 1)
        CS1 = 1;
    else
        CS2 = 1;
    
    SPIdelay();
    
    //Send the B
    lowbyte = data2 & 0xff;
    highbyte = (data2 >> 8);
    highbyte = highbyte | 0b10010000;
    if(DAC == 1)
        CS1 = 0;
    else
        CS2 = 0;
    Nop();
    PutSPIByte (highbyte);
    PutSPIByte(lowbyte);
    Nop();
    CS1 = 1;
    CS2 = 1;

    LDAC = 0;
    SPIdelay();
    LDAC = 1;
    
    
    
}

signed int getSample (unsigned int index, unsigned int blend)
{
    
    signed  int sample=0;   //if you change this to long int, it stops working
    //sine, tri, evenangle,eventooth,saw,square -in order of harmonics
    //for 6 waveforms, need 20 point steps (divide by two) - todo add more waveforms
    #define BLENDSTEPS 16   //works with 10, not with 20, works with 15
    #define NUM_WAVES 9
    #define SCALER (4095/((NUM_WAVES-1)*BLENDSTEPS))
    unsigned int blender;
    blender = blend;
    //blend is max 4095
    blender = blender/SCALER;
    //blender needs to be max (num_waves-1 * BLENDSTEPS)
        //blend contains a value between 0 and 50    
    if(blender>((NUM_WAVES-1)*BLENDSTEPS))
        blender=(NUM_WAVES-1)*BLENDSTEPS;
    else if (blender<0)
        blender = 0;
    
    
    signed  int sample2;  //you get half sample rate for some reason...

    /*
    0 = pure sinewave
     * 0-10 blend of sine to tri
     * 10 = pure tri
     * 10-20 = blend of tri and evenangle
     * 20 = pure evenangle
     * 20-30 = blend of evenangle and eventooth
     * 30 = pure eventooth
     * 30-40 = blend of eventooth and saw
     * 40 = pure saw
     * 40-50 = blend of saw and square
     * 50 = pure square     
     */
    
    if(blender % BLENDSTEPS == 0)
    {
        switch(blender){
            case 0:
                sample=triwave(index);
                break;
            case BLENDSTEPS:
                sample = evenangle[index];
                break;
            case (BLENDSTEPS*2):
                sample = eventooth[index];
                break;
                
            case BLENDSTEPS*3:
                sample = cellowave[index];
                break;    
                
            case BLENDSTEPS*4:
                sample = sawwave(index);
                break;
            case BLENDSTEPS*5:
                sample = squarewave(index);
                break;
            case BLENDSTEPS*6:
                sample = vgamewave(index);
                break;
            case BLENDSTEPS*7:
                sample = pluckwave[index];
                break;
            case BLENDSTEPS*8:
                sample = fmwave[index];
                break;                
        }
    }
    else    //sample is not a pure wave, some mixing needs to be done
    {

        if(blender<(BLENDSTEPS))
        {

            sample = triwave(index);
            sample = __builtin_mulsu(sample,(BLENDSTEPS - blender));
            //sample = sample * (BLENDSTEPS-blender);
            sample2 = evenangle[index];
            sample2 = sample2 * (blender); 
            sample = sample+sample2;
            //for BLENDSTEPS == 16, >>4
            sample = sample>>4;
            
        }
        else if(blender<(BLENDSTEPS*2))
        {
            blender = blender-(BLENDSTEPS);
            sample = evenangle[index];
            sample = sample * (BLENDSTEPS-blender);
            sample2 = eventooth[index];
            sample2 = sample2 * (blender); 
            sample = sample + sample2;
            sample = sample>>4;
        }
        
        else if(blender<(BLENDSTEPS*3))
        {
            blender = blender-(BLENDSTEPS*2);
            sample = eventooth[index];
            sample = sample * (BLENDSTEPS-blender);
            sample2 = cellowave[index];
            sample2 = sample2 * (blender);   
            sample = sample+sample2;
            sample = sample>>4;            
            
        }            

        else if(blender<(BLENDSTEPS*4))
        {
            blender = blender-(BLENDSTEPS*3);
            sample = cellowave[index];
            sample = sample * (BLENDSTEPS-blender);
            sample2 = sawwave(index);
            sample2 = sample2 * (blender);   
            sample = sample+sample2;
            sample = sample>>4;            
            
        }        
        else if(blender<(BLENDSTEPS*5))
                
        {
            blender = blender-(BLENDSTEPS*4);
            sample = sawwave(index);
            sample = sample * (BLENDSTEPS-blender);
            sample2 = squarewave(index);
            sample2 = sample2 *(blender); 
            sample = sample+sample2;
            sample = sample>>4;            
        }
        
        else if(blender<(BLENDSTEPS*6))
                
        {
            blender = blender-(BLENDSTEPS*5);
            sample = squarewave(index);
            sample = sample * (BLENDSTEPS-blender);
            sample2 = vgamewave(index);
            sample2 = sample2 *(blender); 
            sample = sample+sample2;
            sample = sample>>4;            
        }
        
        else if(blender<(BLENDSTEPS*7))
                
        {
            blender = blender-(BLENDSTEPS*6);
            sample = vgamewave(index);
            sample = sample * (BLENDSTEPS-blender);
            sample2 = pluckwave[index];
            sample2 = sample2 *(blender); 
            sample = sample+sample2;
            sample = sample>>4;            
        }

        else if(blender<(BLENDSTEPS*8))
                
        {
            blender = blender-(BLENDSTEPS*7);
            sample = pluckwave[index];
            sample = sample * (BLENDSTEPS-blender);
            sample2 = fmwave[index];
            sample2 = sample2 *(blender); 
            sample = sample+sample2;
            sample = sample>>4;            
        }        
        else
        {
            sample = triwave(index);   //default value in case anything messes up
        }    
        
    }
        
    return sample;
}

signed int squarewave (unsigned int acc)
{
    if(acc<512)
        return 2047;
    else
        return -2047;
}
signed int sawwave (unsigned int acc)
{
    int value;
    value = 2047-(acc*4);

    return (unsigned)value;
}

signed int vgamewave (unsigned int acc)
{
    if(acc<200)
        return 0;
    else if(acc>849)
        return 0;
    else if(acc>549)
        return -32;
    else
        return (partialvgame[acc-200]);
}
signed int triwave (unsigned int acc)
{
    if(acc<=256)
    {
        return (acc*7);
    }
    else if(acc<=512)
    {
        acc=acc-256;
        acc = 256 - acc;
        return (acc*7);
    }
    else if(acc<=768)
    {
        acc = acc-512;
        return 0-(acc*7);
    }
    else
    {
        acc= acc-768;
        acc=256-acc;
        return 0-(acc*7);
    }
}
signed int getAverage(signed int * array,unsigned char length)
{
    signed long value=0;
    unsigned char x;
    for( x = 0; x < length; x++)
    {
        value = value+ *array;
        array++;
    }
    value = value/length;
    return (signed int)value;
}

signed int Scale(signed int sample,unsigned int scalevalue)
{   //scale value is 0 - 1023;
    signed long result;
    result = sample;
    result = result * scalevalue;   //multiply by 0 - 1023

    result = result/1024;            //shift right 10 = divide by 1024

    return (signed int)result;
    
}