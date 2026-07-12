#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LCD Kütüphanesi

// ===== LCD EKRAN AYARLARI =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
String sonEkranaYazilan = "";

// Ekrana yazıyı sadece değiştiğinde basan (robotu yavaşlatmayan) akıllı fonksiyon
void lcdYaz(String satir1, String satir2 = "") {
  if (sonEkranaYazilan != satir1 + satir2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(satir1);
    if (satir2 != "") {
      lcd.setCursor(0, 1);
      lcd.print(satir2);
    }
    sonEkranaYazilan = satir1 + satir2;
  }
}

//=======MESAFE SENSOR======
const int ECHO = 13;
const int TRIG = 11;

float sure;
int mesafe;

// ===== CIZGI SENSÖRLER =====
const int CIZGI_SOL = 12;
const int ORTA_CIZGI_SOL = 2;
const int ORTA_CIZGI_ORTA = 4;
const int ORTA_CIZGI_SAG = 7;
const int CIZGI_SAG = 8;

// ===== 3'LÜ ALEV SENSÖRLERİ =====
const int ALEV_SOL  = A0; // Sol sensör A0 pinine
const int ALEV_ORTA = A1; // Orta sensör A1 pinine
const int ALEV_SAG  = A2; // Sağ sensör A2 pinine
const int ALEV_VAR  = LOW; // Alev sensörleri ateş görünce LOW verir

// ===== FAN MODÜLÜ (L9110) =====
const int FAN_INA = 3;
const int FAN_INB = A3;

// ===== MOTOR PİNLERİ =====
const int SAG_IN1 = 5;
const int SAG_IN2 = 6;
const int SOL_IN3 = 9;
const int SOL_IN4 = 10;

// ===== HIZ AYARLARI =====
int yavash_hizli = 70;
int orta_hizli = 120;
int yuksek_hizli = 150;

int hiz = 80;
// Dönüşlerde motorun takılıp "dız" etmemesi için dönüş hızı güçlendirildi
int donusHizi = 110;

const int SIYAH = 1;
const int BEYAZ = 0;

// =====================
// MOTOR FONKSİYONLARI
// =====================

void ileri(int hiz = yavash_hizli) {
  analogWrite(SAG_IN1, hiz);
  analogWrite(SAG_IN2, LOW);
  analogWrite(SOL_IN3, hiz);
  analogWrite(SOL_IN4, LOW);
}

// Geri gidişler genelde manevra içindir, varsayılan olarak donusHizi kullanılsın
void geri(int seciliHiz = donusHizi) {
  analogWrite(SAG_IN2, seciliHiz);
  analogWrite(SAG_IN1, LOW);
  analogWrite(SOL_IN4, seciliHiz);
  analogWrite(SOL_IN3, LOW);
}

void dur() {
  digitalWrite(SAG_IN1, LOW);
  digitalWrite(SAG_IN2, LOW);
  digitalWrite(SOL_IN3, LOW);
  digitalWrite(SOL_IN4, LOW);
}

void sag_ileri(int seciliHiz = yavash_hizli) {
  analogWrite(SAG_IN1, seciliHiz);
  analogWrite(SAG_IN2, LOW);
}

void sol_ileri(int seciliHiz = yavash_hizli) {
  analogWrite(SOL_IN3, seciliHiz);
  analogWrite(SOL_IN4, LOW);
}

void sag_dur() {
  analogWrite(SAG_IN1, LOW);
  analogWrite(SAG_IN2, LOW);
}

void sol_dur() {
  analogWrite(SOL_IN3, LOW);
  analogWrite(SOL_IN4, LOW);
}

// Dönüş manevralarındaki "dız" sesini önlemek için geri dönüşler donusHizi ile yapılır
void sol_geri(int seciliHiz = donusHizi) {
  analogWrite(SOL_IN4, seciliHiz);
  analogWrite(SOL_IN3, LOW);
}

void sag_geri(int seciliHiz = donusHizi) {
  analogWrite(SAG_IN2, seciliHiz);
  analogWrite(SAG_IN1, LOW);
}

void saga_don() {
  lcdYaz("Kavsak Goruldu", "Saga Donuluyor..");
  ileri();
  delay(150);
  dur();
  delay(300);

  // Yüksek güçle (donusHizi) dönerek takılmayı (dızlamayı) önlüyoruz
  sol_geri(donusHizi);
  sag_ileri(donusHizi);
  delay(100);

  // Siyahı görene kadar yüksek güçte dönmeye devam et
  while (digitalRead(ORTA_CIZGI_SAG) != SIYAH) {
    sol_geri(donusHizi);
    sag_ileri(donusHizi);
  }
  dur();
  delay(500);
}

void tam_don(){
  lcdYaz("Engel/Hat Sonu", "U-Donusu..");
  dur();
  delay(300);
  geri(donusHizi);
  delay(200);

  // Güçlü dönüş başlangıcı
  sol_geri(donusHizi);
  sag_ileri(donusHizi);
  delay(100);

  while (digitalRead(ORTA_CIZGI_SAG) != SIYAH) {
    sol_geri(donusHizi);
    sag_ileri(donusHizi);
  }
  dur();
  delay(500);
}

int mesafe_hesapla() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // Robot takılmasın diye timeout
  sure = pulseIn(ECHO, HIGH, 3000);
  if(sure == 0) return 999;

  mesafe = sure * 0.034 / 2;
  Serial.println(mesafe);
  delay(15);
  return mesafe;
}

void cizgi_izle(int sol, int orta, int sag) {
  if (sol == SIYAH && orta == BEYAZ && sag == BEYAZ) {
    sol_ileri(yavash_hizli);
    sag_ileri(0); // 70-40=30 çok düşük bir PWM'dir, motoru dızlatır. İptal edip 0 yaptık, daha keskin döner.
  }
  else if (sol == SIYAH && orta == SIYAH && sag == BEYAZ) {
    sol_ileri(yavash_hizli);
    sag_ileri(yavash_hizli-20);
  }
  else if (sol == BEYAZ && orta == BEYAZ && sag == SIYAH) {
    sag_ileri(yavash_hizli);
    sol_ileri(0); // Düşük PWM dızlamasını engellemek için 0'a çekildi.
  }
  else if (sol == BEYAZ && orta == SIYAH && sag == SIYAH) {
    sag_ileri(yavash_hizli);
    sol_ileri(yavash_hizli-20);
  }
  else {
    ileri();
  }
}

void kavsak_karar(int SOL, int SAG) {
  if (SAG == SIYAH) {
    saga_don();
  }
  else {
    ileri();
  }
}

// =====================
// ALEV SÖNDÜRME MANTIĞI
// =====================
void atesi_sondur_ve_gec() {
  lcdYaz("! ATES BULUNDU !", "Sonduruluyor...");
  dur();
  delay(500);

  // Fanı çalıştır
  digitalWrite(FAN_INA, LOW);
  digitalWrite(FAN_INB, HIGH);

  delay(8000);

  while (digitalRead(ALEV_SOL) == ALEV_VAR || digitalRead(ALEV_ORTA) == ALEV_VAR || digitalRead(ALEV_SAG) == ALEV_VAR) {
    delay(50);
  }

  lcdYaz("Ates Sondu!", "Goreve Devam..");
  digitalWrite(FAN_INA, LOW);
  digitalWrite(FAN_INB, LOW);
  delay(1000);
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcdYaz("Sistem Basliyor", "Robot Hazir!");
  delay(2000);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(CIZGI_SOL, INPUT);
  pinMode(ORTA_CIZGI_SOL, INPUT);
  pinMode(ORTA_CIZGI_ORTA, INPUT);
  pinMode(ORTA_CIZGI_SAG, INPUT);
  pinMode(CIZGI_SAG, INPUT);

  pinMode(ALEV_SOL, INPUT);
  pinMode(ALEV_ORTA, INPUT);
  pinMode(ALEV_SAG, INPUT);
  pinMode(FAN_INA, OUTPUT);
  pinMode(FAN_INB, OUTPUT);

  pinMode(SAG_IN1, OUTPUT);
  pinMode(SAG_IN2, OUTPUT);
  pinMode(SOL_IN3, OUTPUT);
  pinMode(SOL_IN4, OUTPUT);
}

void loop() {
  // 1. ANLIK ALEV KONTROLÜ
  int solAlev  = digitalRead(ALEV_SOL);
  int ortaAlev = digitalRead(ALEV_ORTA);
  int sagAlev  = digitalRead(ALEV_SAG);

  if (solAlev == ALEV_VAR || ortaAlev == ALEV_VAR || sagAlev == ALEV_VAR) {
    atesi_sondur_ve_gec();
    return;
  }

  // 2. MESAFE KONTROLÜ
  if (mesafe_hesapla() + mesafe_hesapla() + mesafe_hesapla() < 30) {
    tam_don();
  }

  // 3. SENSÖRLERİ OKU VE ÇİZGİ İZLE
  int orta_left  = digitalRead(ORTA_CIZGI_SOL);
  int orta_orta  = digitalRead(ORTA_CIZGI_ORTA);
  int orta_right = digitalRead(ORTA_CIZGI_SAG);

  int sol = digitalRead(CIZGI_SOL);
  int sag = digitalRead(CIZGI_SAG);

  lcdYaz("Cizgi Izleniyor", "Mes: " + String(mesafe) + "cm");

  if (sol == SIYAH || sag == SIYAH) {
    kavsak_karar(sol, sag);
  }

  cizgi_izle(orta_left, orta_orta, orta_right);
}