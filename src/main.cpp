#include <Arduino.h>
//=======MESAFE SENSOR======

const int ECHO = 13;
const int TRIG = 11;

float sure;
int mesafe;




// ===== CIZGI SENSÖRLER =====
const int CIZGI_SOL =  12;
const int ORTA_CIZGI_SOL = 2;
const int ORTA_CIZGI_ORTA = 4;
const int ORTA_CIZGI_SAG = 7;
const int CIZGI_SAG = 8;

// ===== MOTOR =====

const int SAG_IN1 = 6;
const int SAG_IN2 = 5;
const int SOL_IN3 = 10;
const int SOL_IN4 = 9;


// ===== HIZ AYARLARI =====

int yavash_hizli = 70;
int orta_hizli = 120;
int yuksek_hizli = 150;

int hiz = 80;
int donusHizi = 100;

// int beklemeSuresi = 50;
// int kurtulmaSuresi = 80;

const int SIYAH = 1;
const int BEYAZ = 0;


// =====================
// MOTOR FONKSİYONLARI ////////////////////////////////////////////////////////////////////
// =====================

void ileri(int hiz = yavash_hizli) {
  analogWrite(SAG_IN1, hiz);
  analogWrite(SAG_IN2, LOW);
  analogWrite(SOL_IN3, hiz);
  analogWrite(SOL_IN4, LOW);
}

void geri() {
  analogWrite(SAG_IN2, yavash_hizli);
  analogWrite(SAG_IN1, LOW);
  analogWrite(SOL_IN4, yavash_hizli);
  analogWrite(SOL_IN3, LOW);
}

void dur() {
  digitalWrite(SAG_IN1, LOW);
  digitalWrite(SAG_IN2, LOW);
  digitalWrite(SOL_IN3, LOW);
  digitalWrite(SOL_IN4, LOW);
}


void sag_ileri(int hiz = yavash_hizli) {
  analogWrite(SAG_IN1, hiz);
  analogWrite(SAG_IN2, LOW);
}
void sol_ileri(int hiz = yavash_hizli) {
  analogWrite(SOL_IN3, hiz);
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

void sol_geri() {
  analogWrite(SOL_IN4, yavash_hizli);
  analogWrite(SOL_IN3, LOW);
}
void sag_geri() {
  analogWrite(SAG_IN2, yavash_hizli);
  analogWrite(SAG_IN1, LOW);
}


void saga_don() {
  ileri();
  delay(150);
  dur();
  delay(300);
  sol_geri();
  sag_ileri();
  delay(100);
  while (!digitalRead(ORTA_CIZGI_SAG) == SIYAH) {
    sol_geri();
    sag_ileri();
  }
  dur();
  delay(500);
}

void tam_don(){
  dur();
  delay(300);
  geri();
  delay(200);
  sol_geri();
  sag_ileri();
  delay(100);
  while (!digitalRead(ORTA_CIZGI_SAG) == SIYAH) {
    sol_geri();
    sag_ileri();
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
  sure = pulseIn(ECHO, HIGH);
  mesafe = sure * 0.034 / 2;
  Serial.println(mesafe);
  delay(15);
  return mesafe;
}


void cizgi_izle(int sol, int orta, int sag) {
  if (sol == SIYAH && orta == BEYAZ && sag == BEYAZ) {
    sol_ileri(yavash_hizli);
    sag_ileri(yavash_hizli-40);
  }
  else if (sol == SIYAH && orta == SIYAH && sag == BEYAZ) {
    sol_ileri(yavash_hizli);
    sag_ileri(yavash_hizli-40);
  }
  else if (sol == BEYAZ && orta == BEYAZ && sag == SIYAH) {
    sag_ileri(yavash_hizli);
    sol_ileri(yavash_hizli-40);
  }
  else if (sol == BEYAZ && orta == SIYAH && sag == SIYAH) {
    sag_ileri(yavash_hizli);
    sol_ileri(yavash_hizli-40);
  }
  // else if (digitalRead(CIZGI_SOL) == BEYAZ && sol == BEYAZ && orta == BEYAZ && sag == BEYAZ && digitalRead(CIZGI_SAG) == BEYAZ) {
  //    tam_don();
  //  }
  else {
    ileri();
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////
void kavsak_karar(int SOL, int SAG) {
  if (SAG == SIYAH) {
    saga_don();
  }
  else {
    ileri();
  }
}



void setup() {
  Serial.begin(9600);

  pinMode(CIZGI_SOL, INPUT);
  pinMode(ORTA_CIZGI_SOL, INPUT);
  pinMode(ORTA_CIZGI_ORTA, INPUT);
  pinMode(ORTA_CIZGI_SAG, INPUT);
  pinMode(CIZGI_SAG, INPUT);

  pinMode(SAG_IN1, OUTPUT);
  pinMode(SAG_IN2, OUTPUT);
  pinMode(SOL_IN3, OUTPUT);
  pinMode(SOL_IN4, OUTPUT);
}

void loop() {
  int orta_left  = digitalRead(ORTA_CIZGI_SOL);
  int orta_orta  = digitalRead(ORTA_CIZGI_ORTA);
  int orta_right = digitalRead(ORTA_CIZGI_SAG);

  int sol = digitalRead(CIZGI_SOL);
  int sag = digitalRead(CIZGI_SAG);

  if (sol == SIYAH || sag == SIYAH) {
    kavsak_karar(sol, sag);
  }
  if (mesafe_hesapla() + mesafe_hesapla() + mesafe_hesapla() < 30) {
    tam_don();
  }

  cizgi_izle(orta_left, orta_orta,orta_right);


}
