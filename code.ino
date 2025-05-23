#include <Arduino.h> 
#include <math.h> 

// Definisi pin 
#define IN1 6 
#define IN2 7 
#define IN3 5 
#define IN4 4 
#define ENA 9 
#define ENB 10 
#define TRIG 2 
#define ECHO 3 

float jarakSebelumnya = 0; 

// Fungsi Keanggotaan 
float mu_terlalu_dekat(float s) { 
  if (s <= 6) return 1.0; 
  else if (s < 8) return (8 - s) / 2.0; 
  else return 0.0; 
} 

float mu_target(float s) { 
  return exp(-pow(s - 10, 2) / 2.0); 
} 

float mu_dekat(float s) { 
  if (s < 10) return 0.0; 
  else if (s <= 15) return (s - 10) / 5.0; 
  else if (s <= 20) return (20 - s) / 5.0; 
  else return 0.0; 
} 

float mu_jauh(float s) { 
  if (s <= 14) return 0.0; 
  else if (s < 18) return (s - 14) / 4.0; 
  else return 1.0; 
} 

float mu_mendekat(float ds) { 
  if (ds >= -4 && ds < -1) return (-ds - 1) / 3.0; 
  else if (ds < -4) return 1.0; 
  else return 0.0; 
} 

float mu_stabil(float ds) { 
  return exp(-pow(ds, 2) / 2.0); 
} 

float mu_menjauh(float ds) { 
  if (ds > 1 && ds <= 4) return (ds - 1) / 3.0; 
  else if (ds > 4) return 1.0; 
  else return 0.0; 
} 

// Logika Fuzzy 
float fuzzyLogic(float s, float ds) { 
  float sum_alpha_output = 0.0; 
  float sum_alpha = 0.0; 
  float alpha; 

  // Aturan Fuzzy dengan nilai output yang disesuaikan 
  alpha = fminf(mu_terlalu_dekat(s), mu_mendekat(ds)); 
  sum_alpha_output += alpha * (-200); sum_alpha += alpha; 

  alpha = fminf(mu_terlalu_dekat(s), mu_stabil(ds)); 
  sum_alpha_output += alpha * (-100); sum_alpha += alpha; 

  alpha = fminf(mu_terlalu_dekat(s), mu_menjauh(ds)); 
  sum_alpha_output += alpha * 0; sum_alpha += alpha; 

  alpha = fminf(mu_target(s), mu_stabil(ds)); 
  sum_alpha_output += alpha * 0; sum_alpha += alpha; 

  alpha = fminf(mu_target(s), mu_mendekat(ds)); 
  sum_alpha_output += alpha * (-100); sum_alpha += alpha; 

  alpha = fminf(mu_target(s), mu_menjauh(ds)); 
  sum_alpha_output += alpha * 100; sum_alpha += alpha; 

  alpha = fminf(mu_dekat(s), mu_mendekat(ds)); 
  sum_alpha_output += alpha * (-100); sum_alpha += alpha; 

  alpha = fminf(mu_dekat(s), mu_stabil(ds)); 
  sum_alpha_output += alpha * 0; sum_alpha += alpha; 

  alpha = fminf(mu_dekat(s), mu_menjauh(ds)); 
  sum_alpha_output += alpha * 50; sum_alpha += alpha; 

  alpha = fminf(mu_jauh(s), mu_mendekat(ds)); 
  sum_alpha_output += alpha * 100; sum_alpha += alpha; 

  alpha = fminf(mu_jauh(s), mu_stabil(ds)); 
  sum_alpha_output += alpha * 200; sum_alpha += alpha; 

  alpha = fminf(mu_jauh(s), mu_menjauh(ds)); 
  sum_alpha_output += alpha * 200; sum_alpha += alpha; 

  // Defuzzifikasi 
  if (sum_alpha > 0) return sum_alpha_output / sum_alpha; 
  else return 0; 
} 

// Kontrol Motor 
void gerakMotor(int pwm) { 
  int kecepatan = constrain(abs(pwm), 0, 255); 

  if (pwm > 0) { 
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); 
  } else if (pwm < 0) { 
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
  } else { 
    digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); 
    digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW); 
  } 

  analogWrite(ENA, kecepatan); 
  analogWrite(ENB, kecepatan); 
} 

// Baca Sensor 
float bacaSensor() { 
  digitalWrite(TRIG, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TRIG, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(TRIG, LOW); 

  long duration = pulseIn(ECHO, HIGH); 
  return duration * 0.0343 / 2; 
} 

// Visualisasi PWM 
void tampilkanBarPWM(int pwm) { 
  int barLength = map(abs(pwm), 0, 255, 0, 20); 
  Serial.print("PWM: "); 

  if (pwm > 0) for (int i = 0; i < barLength; i++) Serial.print(">"); 
  else if (pwm < 0) for (int i = 0; i < barLength; i++) Serial.print("<"); 
  else Serial.print("0"); 

  Serial.println(); 
} 

// Setup 
void setup() { 
  Serial.begin(9600); 

  pinMode(TRIG, OUTPUT); 
  pinMode(ECHO, INPUT); 

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); 
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT); 

  gerakMotor(0); 
} 

// Loop 
void loop() { 
  float jarak = bacaSensor(); 
  float delta_s = jarak - jarakSebelumnya; 

  float pwm = fuzzyLogic(jarak, delta_s); 
  pwm = constrain(pwm, -200, 200); 

  if (abs(pwm) < 20) pwm = 0; 

  gerakMotor(pwm); 

  Serial.print(jarak); Serial.print("	"); 
  Serial.print(delta_s); Serial.print("	"); 
  Serial.println(pwm); 

  jarakSebelumnya = jarak; 

  delay(100); 
}
