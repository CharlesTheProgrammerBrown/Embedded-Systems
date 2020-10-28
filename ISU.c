#pragma chip PIC18F452
#define NC 50000
char T100ms,blinkT100ms;

//#include "math24F.h"

// return from interrupt
// on lowgoing edge of BT4 give 0.1s pulse from RB0
// RB7 is for blinkalive LED
// TIMER0 generates 100 ms interrupts.
// Fosc=4MHz, 0.1s=100000cc., PS=2, Nc= 100000/2
#define BALEDPin PORTB.0
#define BALEDTrs TRISB.0
#define TrigPin PORTB.1
#define TrigTrs TRISB.1
#define ECHOPin PORTB.2
#define ECHOTrs TRISB.2







void InitPorts(void){
    BALEDTrs = 0;//PORTB.0 = 0;
    TrigTrs = 0;//PORTB.1 = 0;
    ECHOTrs = 1;//PORTB.2 = 1;

    TRISC.6=0;
    TRISC.7=1;
} // RC7..RC0 output

void InitUART(){
    TRISC.6=0;TRISC.7=1;
    BRGH=1; SPBRG=25; TXEN=1;CREN=1;SPEN=1;
}

void Blink(void){
    
    //BALEDPin can used instade of PORTB.0;
    static char BAcount;
    if(BAcount==0) BALEDPin=0; // LED on
    else BALEDPin=1; // LED off
    BAcount++; if(BAcount>9) BAcount=0;
}
void bauddelay(void){
    char i=(104-8)/3 ;
    do{}while(--i);
        }

void transmit(uns16 dmm)
{

    char a[6];
    
    a[0]='0'; while(dmm>10000){++a[0]; dmm-=10000;}
    a[1]='0'; while(dmm>1000){++a[1]; dmm-=1000;}
    a[2]='0'; while(dmm>100){++a[2]; dmm-=100;}
    a[3]='0'; while(dmm>10){++a[3]; dmm-=10;}
    a[4]='0'+dmm;
    a[5]=0x0D;
    

    char i,y;
    for(i=0; i<7; ++i){
        do{}while(!TXIF); TXREG=a[i];
        y=8;while(--y){}
    }
    
    
}

void set100ms() {
    // set Timer0 for 0.1sec
    TMR0H=-NC/256; TMR0L=-NC%256; TMR0IF=0;
    T0CON=0b1000.0000;
}
void main(void)
{
    char PPB, PB, PBpushed;
    T100ms=0;
    InitPorts();
    InitUART();
    
    PB=PORTB;
    do{
        set100ms();
        PPB=PB; PB=PORTB; PBpushed=~PB&PPB;
        if (T100ms >= 50) {
            T100ms = 0;
            TrigPin = 1; // trig
            
            char i = 20/3;
            do{}while(--i);
            
            TrigPin = 0;
            do{}while(!ECHOPin);
            uns16 t1,t2,td; t1.low8 = TMR0L;t1.high8 = TMR0H;
            do{}while(ECHOPin && !TMR0IF );
            
            t2.low8 = TMR0L;t2.high8 = TMR0H;
            td = t2 - t1;
            
            uns16 dmm;
           // dmm=tp*0.34;
           
            //multiplyconstfrac(tp,84,dmm);
            if(!TMR0IF){
                WREG=td.low8; multiply(87);
                dmm.low8=PRODH; WREG=td.high8;
                multiply(87); dmm.low8 += PRODL;
                if(Carry) ++PRODH; dmm.high8=PRODH;
            }else{
                dmm = 1000;
            }
            
                transmit(dmm);
            
            
            
            
     
        }
        
        do{}while(!TMR0IF);
        ++T100ms;
        Blink();
    }while(1); /* now mainloop takes around 20 cc only */
}
