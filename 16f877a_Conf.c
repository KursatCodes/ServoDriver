
// PIC16F877A konfig�rasyon ayarlar? burada yer al?r


#pragma config FOSC = XT // PIC'in harici kristal osilat�r kullanaca??n? belirtin (XT)
#pragma config WDTE = OFF // Watchdog zamanlay?c?s?n? devre d??? b?rak?n
//#pragma config PWRTE = ON     // Power-up Timer Enable bit (PWRT disabled)
#pragma config PWRTE = OFF // G�� �zerinden s?f?rlama zamanlay?c?s?n? devre d??? b?rak?n
#pragma config BOREN = ON // �?k?? d�?�? voltaj? izleme (Brown-out Reset) etkinle?tirilsin
#pragma config LVP = OFF // D�?�k gerilim programlamay? devre d??? b?rak?n
#pragma config CPD = OFF // EEPROM i�eri?i koruma devre d??? b?rak?n
#pragma config WRT = OFF // Flash haf?za yazma koruma devre d??? b?rak?n
#pragma config CP = OFF // Flash haf?za kod koruma devre d??? b?rak?n