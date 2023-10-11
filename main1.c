#define _XTAL_FREQ 4000000
// 10.10.2023 eprom döngüsü 200 saat yap?ld?
//lcd pinleri tan?mlamalari
#define RS RB5
#define EN RB4
#define D4 RB3
#define D5 RB2
#define D6 RB1
#define D7 RB0
#define RW RD6
//yön pinleri tan?mlamalari
#define FWD RE0
#define REW RE1
#define RewFEAD  RC0
#define FwdFEAD  RE2
#define FwdLMT RC1
#define RewLMT RC2

#define AcEror RD0
#define DcEror RC3
#include <xc.h>
#include "lcd.h"
#include <stdio.h>
#include "uart.h"
#include "16f877a_Conf.c"
#include <stdbool.h>

void yagBakim() {
    Lcd_Clear();
    for (int i = 0; i < 10; i++) {
        Lcd_Set_Cursor(1, 1);     
        Lcd_Write_String("YAG BAKIMI YAPINIZ");
        Lcd_Set_Cursor(2, 1);     
        Lcd_Write_String("OIL THE GEARS");
        RA4 = 1;
    }
    Lcd_Clear();
    RA4 = 0;
}

unsigned int eepromWRclc = 0;
unsigned int kesmeSayaci = 0;
int epromBaslaAdress = 0x01; // Ba?lang?ç EEPROM adresi


struct Time {
    unsigned int carpan;
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
};

struct Time currentTime = {000,000, 00, 00}; // Saat, dakika ve saniye cinsinden süre

void writeEEPROM(unsigned int address, unsigned int data) {
    // EEPROM'a veri yazma i?lemi
    EEADR = address;   // EEPROM adresini ayarla
    EEDATA = data;     // EEPROM verisini ayarla
    WREN = 1;          // EEPROM yazma izni ver
    GIE = 0;           // Kesmeleri devre d??? b?rak
    EECON2 = 0x55;     // 
    EECON2 = 0xAA;     // 
    WR = 1;            // EEPROM yazma i?lemini ba?lat
    GIE = 1;           // Kesmeleri etkinle?tir
    WREN = 0;          // EEPROM yazma iznini kapat
    while (WR == 1);   // Yazma i?lemi tamamlanana kadar bekle
}

unsigned int readEEPROM(unsigned int address) {
    // EEPROM'dan veri okuma i?lemi
    EEADR = address;   // EEPROM adresini ayarla
    RD = 1;            // EEPROM okuma i?lemini ba?lat
    return EEDATA;     // Okunan veriyi döndür
}

void incrementTime(struct Time* time) {
    time->seconds++;  
    if (time->seconds >= 60) {
        time->seconds = 0;
        
        time->minutes++;
        if (time->minutes >= 60) {
            time->minutes = 0;
        
            time->hours++;
            if (time-> hours >= 250){
                time-> hours = 0;
                time->carpan++;
                
            }
        }
    }
}

// Kesme için gerekli tan?mlamalar
void __interrupt() timer_isr(void) {
    if (T0IF) { // Zamanlay?c? 0 kesmesi mi?
        T0IF = 0; // Kesme bayra??n? s?f?rla
        TMR0 =61; // Zamanlay?c? de?erini ayarla (256 - 250 = 6)
  
 
        kesmeSayaci++;
        if (kesmeSayaci==20){
            kesmeSayaci = 0 ;
            incrementTime(&currentTime);
 eepromWRclc++;
            if (eepromWRclc==60){
                eepromWRclc=0;
            }
 
 
        writeEEPROM(epromBaslaAdress    , currentTime.hours);
        writeEEPROM(epromBaslaAdress + 1, currentTime.minutes);
        writeEEPROM(epromBaslaAdress + 2, currentTime.seconds);
        writeEEPROM(epromBaslaAdress + 3, currentTime.carpan);
        
        if( readEEPROM(readEEPROM(epromBaslaAdress)==200 )){
            
        epromBaslaAdress= epromBaslaAdress +(readEEPROM(epromBaslaAdress+3))*4;
        
        }
        
       
     
               
        }        
    }
}
_Bool limitler = 0;
int main()
{
  unsigned int a;
  
 //lcd pinleri
TRISB = 0b00000000; // Port B pinlerini ç?k?? olarak ayarla
TRISA4 = 0;           // RA4 BUZZER Pini ç?k?? olarak ayarla
TRISE0 = 1; // sag giris olarak 
TRISE1 = 1; // sol giris olarak ayarla  YÖN P?NLER?
TRISC1 = 1; // limit giris 
TRISC2 = 1; // limit giris
TRISE2 = 1; //SOL FEED Giri?
TRISC0 = 1; //SAG FEED giri? set
TRISC3 = 1; // dc servo eror pin giri?
TRISD0 = 1; // AC SERVO EROR G?R??
//pot pinleri
TRISA1 = 1; //// RA1 Pin dc servonun speed'in potu giri? 
TRISA2 = 1; // RPM POTU G?R??
TRISA3 = 1; // KARTIN ÜSTÜNDEK? AYAR POTU
Lcd_Init();
//uart pinleri
TRISC6 = 0; // TX Pin ç?k?? olarak ayarla
TRISC7 = 1; // RX Pin giri? olarak ayarla

//kullan?lmayan pinler c?k?? olarak ayarlan?yor
TRISA0 = 0;
TRISA5 = 0;
TRISC4 = 0;
TRISC5 = 0;
TRISD = 0b00000001; // RD1, RD2, RD3, RD4, RD5, RD6, RD7 ç?k??; di?eri giri?

  //buzzer ba?lang?çta kapat
  __delay_ms(200); //
  RA4 = 0; // Buzz
  //buzzer bitti
   
  // EEPROM'dan kaydedilmi? süreyi oku
  // Saat de?erini bellekten okuma
  
    currentTime.hours   = readEEPROM(0x01);
    currentTime.minutes = readEEPROM(0x02);
    currentTime.seconds = readEEPROM(0x03);
    currentTime.carpan  = readEEPROM(0x04);

   
    char lcdText[9]; // HHHHH:MM + null karakteri için yeterli bo?luk

    OPTION_REGbits.T0CS = 0; // Timer0 için dahili osilatörü kullan
    OPTION_REGbits.PSA = 0;  // Prescaler'? Timer0'a ata
    OPTION_REGbits.PS2 = 1;  // Prescaler bölme oran?n? 1:256 yap
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    
     // Kesmeleri etkinle?tir
    INTCONbits.GIE = 1;    // Genel kesmeleri etkinle?tir
    INTCONbits.PEIE = 1;   // Periferik kesmeleri etkinle?tir
    INTCONbits.T0IE = 1;   // Timer0 kesmelerini etkinle?tir
//kesme biti?  
  
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("UZMANLAR MAKINA");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("www.borvek.com");
    Lcd_Set_Cursor(2,17);
    Lcd_Write_String("V3");
     
    __delay_ms(1000);
    Lcd_Clear();
    
   
// UART ayarlar?
    
    SPBRG = ((_XTAL_FREQ/16)/19200) - 1; // SPBRG de?eri 19200 baud h?z? için hesapland?
    UART_Initialize();
unsigned int displayValue = 0; // Güncel de?eri saklamak için bir de?i?ken

//uart bitti

 /// ADC yap?land?rmas?
    ADCON1bits.ADFM = 1; // Sonuçlar sa?dan sola hizalan?r (8-bit sonuç için)
    ADCON1bits.PCFG = 0b100; // Tüm pinler dijital giri?/ç?k?? olarak ayarlan?r, RA1 RA2 RA3 analog kal?r
    ADCON0bits.ADCS = 0b10; // Fosc/32 h?z?nda dönü?üm (uygun bir de?er seçebilirsiniz)
    ADCON0bits.ADON = 1; // ADC'yi etkinle?tir

    float adcValue1; // Birinci potansiyometre de?eri
    int   adcValue2; // ?kinci potansiyometre de?eri
    float adcValue3; // trimpot de?eri
 
    // adc bitti


 // Yön bayraklar?n? ba?lang?çta s?f?ra ayarla
    FwdLMT = 0;
    FwdFEAD = 0;
    FWD = 0;
    RewLMT = 0;
    RewFEAD = 0;
    REW = 0;

int ilkAcilis = 1;

while(1){
    
    //e?er eprom de?erinde 255 yazarsa s?f?rlamak için 
  
      for ( int epromAdresi = 0 ; epromAdresi<255; epromAdresi++){   
        int olmazlar = readEEPROM(epromAdresi);  
        if (olmazlar==255){        
       writeEEPROM(epromAdresi,0);         
        }
    }
    
 /*
    if( readEEPROM(0x02)== 255 && readEEPROM(0x01)==255 && readEEPROM(0x04)==255 ){
        
        writeEEPROM(0x01, 0);
        writeEEPROM(0x02, 0);
        writeEEPROM(0x03, 0);
        writeEEPROM(0x04, 0);
    }
  */
    
    
    
if (ilkAcilis) {
    
   
        if ( FwdFEAD == 0 && FWD == 0 &&  RewFEAD == 0 && REW == 0) {
            ilkAcilis = 0; // ?lk çal??t?rma i?lemi tamamland?
        } else {
            // Yön bayraklar? hala 0 de?ilse ana döngüye girmeyin
             Lcd_Set_Cursor(1,1);
    Lcd_Write_String("STOP DURUMUNA GETIR");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("PUT IT IN STOP STATE");
    __delay_ms(500);
            return 0; // Ana döngüye girmeyin
        }
    }

if( !DcEror && !AcEror && limitler == 0){
  if ( FwdFEAD == 0 && FWD == 0 &&  RewFEAD == 0 && REW == 0) {
            limitler = 0; // limitler bayragi dustu
        }

    
    int dakika = readEEPROM(epromBaslaAdress+1);
    int realSaat = readEEPROM((epromBaslaAdress+3)*250+readEEPROM(epromBaslaAdress));
   
   
 // Saati ve dakikay? do?ru formatta birle?tirip lcdText'e yaz
   
        sprintf(lcdText, "%5uh %02um", realSaat, dakika);
        
    // LCD'ye saati yazd?r
     Lcd_Set_Cursor(1, 10);
     Lcd_Write_String("  ");
     Lcd_Set_Cursor(1, 11);
     Lcd_Write_String(lcdText); 
     // Saat, dakika ve saniyeyi yazd?r
     
      // yag bakim uyari cagirici
    if ( (  realSaat % 500 == 0 && realSaat != 0 
     || realSaat % 500 == 1 && realSaat != 1
     || realSaat % 500 == 2 && realSaat != 2
     ) && currentTime.minutes == 0 ) {
    yagBakim();
}
    //yag bakimi cagrildi
        

     
         // üçüncü( kart?n üstündeki trimpot) de?eri
    ADCON0bits.CHS = 0b0010; // RA2 pini seçildi (Analog giri? pininiz)
    __delay_us(50);
    ADCON0bits.GO = 1; // ADC dönü?ümünü ba?lat
    while (ADCON0bits.GO); // Dönü?ümün tamamlanmas?n? bekle
    adcValue3 = (ADRESH << 8) | ADRESL;
    float oranValue = adcValue3/1024.0; 
        
 // rpm potansiyometre de?eri
    ADCON0bits.CHS = 0b0011; // RA3 pini seçildi (Analog giri? pininiz)
    __delay_us(50);
    ADCON0bits.GO = 1; // ADC dönü?ümünü ba?lat
    while (ADCON0bits.GO); // Dönü?ümün tamamlanmas?n? bekle
    adcValue2 = (ADRESH << 8) | ADRESL;
    
    float integerPart = adcValue2*oranValue*3; // Tam say? k?sm?
    
    int binler =  (int)integerPart/1000;
    int yuzler = ((int)integerPart-binler*1000)/100;
    int onlar  = ((int)integerPart-binler*1000-yuzler*100)/10;
    int birler =  (int)integerPart%10; 
    
   
    
  // feed rate pot de?eri 
     
        ADCON0bits.CHS = 0b0001; // RA1 pini seçildi (Analog giri? pininiz)
        __delay_us(50);
        ADCON0bits.GO = 1; // ADC dönü?ümünü ba?lat
        while (ADCON0bits.GO); // Dönü?ümün tamamlanmas?n? bekle
        adcValue1 = (ADRESH << 8) | ADRESL;
        if (FwdFEAD==0 && RewFEAD== 0 ){
        displayValue = (int)adcValue1 >> 1; // ADC sonucunu 0-512 aras?na dönü?tür
        }
        float gostermeliklcd = adcValue1 / 1024 * 1000;
       

            
// feed rate pot de?erini LCD'ye yazd?r
        Lcd_Set_Cursor(2, 1);
        Lcd_Write_String("FeedRate:");
        char adcString1[4];
        sprintf(adcString1, "%003u", (int)gostermeliklcd);
        Lcd_Set_Cursor(2, 10);
        Lcd_Write_String(adcString1);

// rpm potansiyometre de?erini LCD'ye yazd?r
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("RPM:");
        char rpmString[5]; // Virgülden sonraki 1 basamak için 5 hane yeterlidir
sprintf(rpmString, "%u%u%u.%u", binler,yuzler,onlar,birler);
        
Lcd_Set_Cursor(1, 5);
Lcd_Write_String(rpmString);


            
//yönler basla

// Fwd k?sm?
 if (FwdLMT == 1 && (FWD == 1 || FwdFEAD == 1)) {
     limitler=1;
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" FWD LMT");
            UART_Write_Text("s0\r\n");
        
        } else if ((FwdFEAD == 1 && FWD == 1) || (FwdFEAD == 1 && REW == 0)) {
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" FWD 1.6K");
            if (displayValue < 800) {
                char uartMessage[8];
                displayValue += 80;
                
                if (displayValue > 800) {
                    displayValue = 800;
                }
                sprintf(uartMessage, "s%d\r\n", displayValue);
                UART_Write_Text(uartMessage);
            }
          
        } else if (FWD == 1) {
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" FWD          ");
            char combinedText[20];
            sprintf(combinedText, "s%d\r\n", displayValue);
            UART_Write_Text(combinedText);
            
//rew k?sm?            
        } else if (RewLMT == 1 && (REW == 1 || RewFEAD == 1)) {
            limitler=1;
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" REW LMT");
            UART_Write_Text("s0\r\n");
       
        } else if ((RewFEAD == 1 && REW == 1) || (RewFEAD == 1 && FWD == 0)) {
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" REW 1.6K");
            int negativeFeedValue = -displayValue;
            if (negativeFeedValue > -800) {
                char uartMessage[8];
                negativeFeedValue -= 80;
                if (negativeFeedValue < -800) {
                    negativeFeedValue = -800;
                }
                displayValue=-negativeFeedValue;
                sprintf(uartMessage, "s%d\r\n", negativeFeedValue);
                UART_Write_Text(uartMessage);
            }
        
        } else if (REW == 1) {
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" REW         ");
            int negativeDisplayValue = -displayValue;
            char combinedText[20];
            sprintf(combinedText, "s%d\r\n", negativeDisplayValue);
            UART_Write_Text(combinedText);
        } else {
          
            Lcd_Set_Cursor(2, 13);
            Lcd_Write_String(" STOP       ");
            UART_Write_Text("s0\r\n");
        }

    
//yönler bitii  
         
        //limit dü?melerinde buzzer aç kapa
        if((FwdLMT==1) &&( FWD==1 || FwdFEAD  )){
            
            RA4=1;
        }
        else if(( RewLMT==1) && (REW==1 || RewFEAD)){
            RA4=1;
            }
       
        else {
            RA4=0;
        }
        //limitlerde buzzer kontrolu son
        
  }   
else if( DcEror==1) {
    
         //dc servo error

    
    Lcd_Set_Cursor(1,1);         
            Lcd_Write_String("   FEED RATE ERROR       ");
    Lcd_Set_Cursor(2,1);
            Lcd_Write_String("   ILERLEME HATASI        ");      
     UART_Write_Text("s0\r\n");
      RA4=1;
      __delay_ms(1000);
      RA4=0;
      __delay_ms(3000);
      RA4=1;
    
  
} 
// ac eror
     else if (AcEror == 1 ){
    UART_Write_Text("s0\r\n");
    Lcd_Set_Cursor(1,1);         
            Lcd_Write_String("   ROTATION ERROR      ");
    Lcd_Set_Cursor(2,1);
            Lcd_Write_String("   DONME HATASI        ");
                RA4=1;
      __delay_ms(1000);
      RA4=0;
      __delay_ms(3000);
      RA4=1;
}
      else if (limitler==1){
         /*if(akimSayaci>=3){
             //yuksekAkimdaDurdur();
         UART_Write_Text("m2\r\n");
         akimSayaci=0;
         }*/
         UART_Write_Text("s0\r\n");
         Lcd_Set_Cursor(1,1);
         Lcd_Write_String("STOP DURUMUNA GETIR ");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("PUT IT IN STOP STATE");
         RA4=1;
      __delay_ms(1000);
      RA4=0;
      __delay_ms(1000);
      if ( FwdFEAD == 0 && FWD == 0 &&  RewFEAD == 0 && REW == 0) {
            limitler = 0; // limitler bayragi dustu
        }
     }
    }
  return 0;
  
}