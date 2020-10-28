#pragma chip PIC18F452
#define CLR_LCD; PrintLCD("\xff\x02\x82\x28\x01\x03\x0C\x06");
#define NC 50000
#define h 1000
#define on 0
#define off 1
#define LNW PORTB.1
#define LBA PORTB.0
char ustr[16];
char bstr[16];
char T100ms,Tbase;
char rstr[8];
char *btr;
char io;
char string_ready;
uns16 iwl;
void str2i();
#pragma origin 0x08
void ISR(){
    static char WX,SX,BX;
    WX=WREG;SX=STATUS;BX=BSR;
    if(TMR0IF){
        T0CON=0b10000.000; // set TMR0 for 0.1 sec
        TMR0H=-NC/256; TMR0L=-NC%256;TMR0IF=0;
        T100ms++;Tbase=1;
    }
    if(RCIF){
        char t;
        t = RCREG;
        
       // RCIF=0;
        if(t == 0x0D){
            
            btr=rstr;
            str2i();
        }else if(t!=6){
            *btr = t;
        
            ++btr;
        }
    }

    STATUS = SX;BSR=BX;WREG=WX;
    retint();
}




bit ADC_go_done @ ADCON0.2;
//Global variables for LCD
// Data and tris bits, each bit may be defined at any io pin.
bit LCD4 @PORTC.2, LCD5 @PORTC.3, LCD6 @PORTC.4, LCD7 @PORTC.5,
LCD4T @TRISC.2, LCD5T @TRISC.3, LCD6T @TRISC.4, LCD7T @TRISC.5;

bit LCDR @PORTC.0, LCDRT @TRISC.0, // Control RS and its tris
LCDE @PORTC.1, LCDET @TRISC.1, // Control E and its tris
LCDS, LCDC ; // single control char, and control toggle flags


// Procedures related to LCD

void LCDw2u(char W){ // W x 3 microsecond for 4MHz.
    WREG =W; do{ }while( --WREG ); }

void LCDNibble(char Ch)
{
    LCD4T=0; LCD5T=0; LCD6T=0; LCD7T=0;
    LCDRT=0; LCDET=0; LCDE = 0;
    if(LCDC) LCDR=0; else LCDR = 1;
    if(Ch.4) LCD4=1; else LCD4=0; if(Ch.5) LCD5=1; else LCD5=0;
    if(Ch.6) LCD6=1; else LCD6=0; if(Ch.7) LCD7=1; else LCD7=0;
    LCDE=1;
    LCDw2u(2);
    LCDE=0;
    LCD4=0;nop();LCD5=0;nop();LCD6=0;nop();LCD7=0; LCDw2u(5);
}

void PrintLCD(const char *Ch)
{
    char WC,WP=0; LCDS=0 ; LCDC=0 ; LCDE=0 ;
    do{ WC=Ch[WP]; WP++;
        if(WC){ LCDC=0;
            if(WC==0xFF) LCDS ^=1 ;
            else { LCDC=WC.7||LCDS; if(WC<4) LCDC=1;
                LCDNibble( WC & 0xF0);
                if(LCDS && WC==0x28){//set mode takes 3ms time
                    char T=12;do{LCDw2u(0);}while(--T);}
                LCDNibble( swap(WC) & 0xF0 );
                if(WC<4){char T=12; do{ LCDw2u(0);}while(--T);}
                LCDw2u(20); } }
    }while(WC);
}


char* i2a(uns16 k, char *a){
    char i = 5;
    if(!k){
        --i;a[i]=0;
    }
    while(k){ --i;a[i]=k%10; a[i]+='0'; k/=10;}
    while(i){--i;a[i]=' ';}
    a+=5;
    return a;
}

void str2i() {
    
   
    static char iy;
    iy=0;
    iwl=0;
    /*do{}while(!TXIF);TXREG=rstr[0];
    do{}while(!TXIF);TXREG=rstr[1];
    do{}while(!TXIF);TXREG=rstr[2];
    do{}while(!TXIF);TXREG=rstr[3];
    do{}while(!TXIF);TXREG=rstr[4];
    do{}while(!TXIF);TXREG=0x0D;*/
    //tt=a[i]-'0';res += tt*mult;
    //res*=10;res +=a[i]-'0';
    //do{}while(!TXIF);TXREG=rstr[i];
    while(iy<5){iwl*=10;
        iwl +=rstr[iy]-'0';++iy;}


   if(iwl>1000)
       iwl=1000;
    //2,0,0,0,0 original in ISU
    //0,0,0,0,2 recieved
}


   

void initADC() {
    ADCON0 = 0b11.000.0.0.1;
    ADCON1 = 0b0.0.00.1110;
}

void startADC(){
    ADC_go_done=1; do{}while(ADC_go_done); // wait ADC to be over
}

void InitPorts(void){
    TRISB.0=0; // RB0 Blink alive LED out
    TRISB.1=0;
    TRISA.0 = 1;//POT input
    TRISC.6=0; // TX UART
    TRISC.7=1; //TR UART
} // RC7..RC0 output

void InitUART(){
    TRISC.6=0;TRISC.7=1;
    BRGH=1; SPBRG=25; TXEN=1;CREN=1;SPEN=1;
}
void initINT(){
    TMR0IE = 1;TMR0IP=0;TMR0IF=0;
    RCIE = 1; RCIF=0;RCIP=0;
    IPEN = 0;GIE=1;PEIE=1;
}
char* strcpy( char *a, const char *b)
{
    char t;
    do{
        t=*b;
        *a=t;
        ++a; ++b;
    }while(t);
    return --a;
}

void blink(void) {
    
    static char BAcount;
    if(BAcount == 0) LBA = on;
    else LBA = off;
    ++BAcount;
    if(BAcount > 9) BAcount = 0;
}

void main(void){
    initINT();
    InitPorts();
    InitUART();
    initADC();
    Tbase=0;
    LNW = off;
    TMR0IF=1;
    T100ms = 0;
    //btr = rstr;
    btr=rstr;
    string_ready = 0;
    uns16 ipot=0,iwp=0,htmp;
    iwl=0;
    
    {char i=200; do{LCDw2u(250);}while(--i);}//reset
    CLR_LCD;
    PrintLCD("\x80Welcome...");
    char *pt;

    do{
        if(T100ms>50) {
            
            pt = strcpy(ustr,"\x80L Water=");
            iwp = h - iwl;
            htmp = h / 100;iwp/=htmp;
            pt = i2a(iwp,pt);
            
                
            *pt='%';pt++;*pt=0x0D;
            PrintLCD(ustr);
            
            
            startADC();
            ipot = ADRESH;
            ipot*=100;ipot/=255;
            pt = strcpy(bstr,"\xC0L POT  =");
            pt = i2a(ipot,pt);
            *pt='%';pt++;*pt=0;
            PrintLCD(bstr);
            
            if(iwp<ipot)
                LNW = on;
            else
                LNW = off;
            T100ms = 0;
            
            
        }
        
   
  
        
        
        if(Tbase){
            blink();
            Tbase=0;
        }
        
    }while(1);
}
