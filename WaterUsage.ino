#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Deklarasi PIN
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte flowSensor       = 2;
const int IRSensor    = 8;
const int relay       = 13; 
const int pushButton  = 7;
const int redLight    = 9;
const int greenLight  = 11;
const int blueLight   = 10;

// Pengaturan kapasitas tangki dan kuota air per orang (Liter)
float kapasitasTangki = 15; 
float kuota           = 1;

// Deklarasi variabel
int buttonState       = 0;  // Status push button 
int IRState           = 0;  // Status infrared sensor
float konst           = 7;  // Kontanta pembagi untuk sensor water flow 
float debit_air;
volatile byte count;        // Counter
unsigned int flow_mlt;
unsigned long vol_terpakai;
unsigned long oldTime;
unsigned long lastUse;
int afteruse          = 0;  // Konstanta untuk menyatakan penggunaan selesai

// Setup
void setup() 
{
  //Inisialisasi LCD
  lcd.init();
  lcd.backlight();

  //PIN Mode
  pinMode (IRSensor, INPUT);
  pinMode(flowSensor, INPUT); 
  pinMode(pushButton, INPUT);
  pinMode (relay, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(greenLight, OUTPUT);
  pinMode(blueLight, OUTPUT);
  
  digitalWrite(relay, HIGH);
  digitalWrite(flowSensor, HIGH);
  
  attachInterrupt(digitalPinToInterrupt(flowSensor), countPulse, FALLING);
  kapasitasTangki = kapasitasTangki * 1000;
  delay (2000);
  resetcount();
}

// Looping
void loop(){
  // Membaca status sensor IR dan psuh button
  buttonState = digitalRead(pushButton);
  IRState = digitalRead (IRSensor);

  // Jika push button ditekan, maka kuota akan di-reset
  if (buttonState == HIGH) {
    resetcount();
  }

  // Persyaratan untuk menyalakan/mematikan aliran air
  if (IRState == 0 || vol_terpakai >= 0.90*kuota*1000 || (kapasitasTangki < kuota*1000 && afteruse==0 && ((millis() - lastUse)>3000))){
    digitalWrite(relay, HIGH);
    afteruse=0;
  }
  else{
    afteruse=1;
    digitalWrite(relay, LOW);
    lastUse = millis();
  }    

  // Perhitungan volume air terpakai
  if ((millis() - oldTime) > 1000) {
    detachInterrupt(digitalPinToInterrupt(flowSensor));
    debit_air = ((1000.0 / (millis() - oldTime)) * count) / konst;
    oldTime = millis();
    flow_mlt = (debit_air / 60) * 1000;
    vol_terpakai += flow_mlt;
    kapasitasTangki -= flow_mlt;
    count = 0;
    attachInterrupt(digitalPinToInterrupt(flowSensor), countPulse, FALLING);
  }

  // Menentukan warna lampu indikator LED
  if ((vol_terpakai > 0)  && (vol_terpakai <= kuota*1000/2)){
    RGB_color(0, 255, 0); //Hijau
  }
  else if ((vol_terpakai > kuota*1000/2) && (vol_terpakai <= kuota*1000*4/5)){
    RGB_color(255, 255, 0); // Kuning
  }
  else if (vol_terpakai > kuota*1000*4/5){
    RGB_color(255, 0, 0); //Merah
  }
  else{
    RGB_color(0, 0, 0); //Off
  }
  
  // Menentukan keluaran LCD display
  if (vol_terpakai >= kuota*1000){
    lcd.setCursor(0,0);
    lcd.print("Limit Exceeded! ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }
  else if (kapasitasTangki < kuota*1000 && afteruse==0 && ((millis() - lastUse)>3000)){
    lcd.setCursor(0,0);
    lcd.print("No Water!");
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }else{
    lcdprint();
  }
}

// Fungsi mereset kuota
void resetcount(){
  count = 0;
  debit_air = 0.0;
  flow_mlt = 0;
  vol_terpakai = 0;
  oldTime = 0;
  afteruse=0;
}

// Fungsi menghitung pulse
void countPulse(){
  count++;
}

// Fungsi print LCD
void lcdprint(){
  // Baris 1
  lcd.setCursor(0,0);
  lcd.print("Tank:");
  lcd.print(kapasitasTangki/1000.0,1);
  lcd.print("L         ");

  //Baris 2
  lcd.setCursor(0,1);
  lcd.print("Used:");
  lcd.print(vol_terpakai/1000.0,2);
  lcd.print("L/");
  lcd.print(kuota,1);
  lcd.print("L     ");
}

// Fungsi mengatur warna indikator LED
void RGB_color(int redValue, int greenValue, int blueValue)
 {
  analogWrite(redLight, redValue);
  analogWrite(greenLight, greenValue);
  analogWrite(blueLight, blueValue);
}
