// Pengukur Indeks UV — ESP32 + ML8511 + LCD I2C 16x2
//
// Modul ML8511 (Lapis Semiconductor) adalah sensor UV analog:
// fotodioda UV + penguat arus-ke-tegangan di dalam chip. Tegangan
// output naik linear dengan intensitas UV. Menurut datasheet
// FEDL8511-00 Rev1.2:
//   - Tanpa sinar UV, tegangan output sekitar 1,0 V.
//   - Pada intensitas 10 mW/cm2 (puncak 365 nm), output 2,2 V.
//   - Indeks UV = (tegangan output - 1,0 V) x 12,5.
// Skala indeks UV (WHO): 0-2 Rendah, 3-5 Sedang, 6-7 Tinggi,
// 8-10 Sangat Tinggi, 11 ke atas Ekstrem.
// Sensor mendeteksi UV 280-390 nm (UV-A + UV-B), catu 2,7-3,6 V.
//
// Wiring:
//   ML8511 VIN  -> ESP32 3V3
//   ML8511 GND  -> ESP32 GND
//   ML8511 OUT  -> ESP32 GPIO34 (ADC1, input-only)
//   LCD VCC     -> ESP32 3V3
//   LCD GND     -> ESP32 GND
//   LCD SDA     -> ESP32 GPIO21 (SDA I2C)
//   LCD SCL     -> ESP32 GPIO22 (SCL I2C)
//
// Terverifikasi compile: lihat README.md.

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- Konfigurasi ----------------

// Pin output analog ML8511 (GPIO34 = ADC1 channel 6, input-only).
const int PIN_ML8511_OUT = 34;

// Pin bus I2C untuk LCD (pin default ESP32).
const int PIN_I2C_SDA = 21;
const int PIN_I2C_SCL = 22;

// Alamat I2C LCD 16x2 (backpack PCF8574).
const uint8_t ADDR_LCD = 0x27;
const uint8_t LCD_KOLOM = 16;
const uint8_t LCD_BARIS = 2;

// Konstanta datasheet ML8511 (FEDL8511-00 Rev1.2).
// Tegangan output saat tidak ada sinar UV.
const float TEGANGAN_TANPA_UV = 1.0;
// Faktor konversi datasheet: indeks UV per volt di atas tegangan tanpa UV.
const float FAKTOR_UV_PER_VOLT = 12.5;

// Ambang kategori indeks UV (skala WHO).
const float AMBANG_SEDANG = 3.0;
const float AMBANG_TINGGI = 6.0;
const float AMBANG_SANGAT_TINGGI = 8.0;
const float AMBANG_EKSTREM = 11.0;

// Jumlah sampel ADC yang dirata-rata per pembacaan.
const int JUMLAH_SAMPEL = 8;
// Seberapa sering sensor dibaca dan LCD diperbarui (2 kali per detik).
const unsigned long INTERVAL_BACA_MS = 500;

LiquidCrystal_I2C lcd(ADDR_LCD, LCD_KOLOM, LCD_BARIS);

// ---------------- Fungsi ----------------

// Baca tegangan output ML8511 dalam volt.
// analogReadMilliVolts memakai kalibrasi ADC internal ESP32,
// jadi hasilnya voltase sebenarnya tanpa hitung manual.
float bacaTeganganSensor() {
  long jumlahMilliVolt = 0;
  for (int i = 0; i < JUMLAH_SAMPEL; i++) {
    jumlahMilliVolt += analogReadMilliVolts(PIN_ML8511_OUT);
    delay(5);
  }
  float rataMilliVolt = (float)jumlahMilliVolt / JUMLAH_SAMPEL;
  return rataMilliVolt / 1000.0;
}

// Hitung indeks UV dari tegangan output (rumus datasheet).
// Di bawah tegangan tanpa UV, indeks dijepit ke nol.
float hitungIndeksUV(float teganganVolt) {
  float indeks = (teganganVolt - TEGANGAN_TANPA_UV) * FAKTOR_UV_PER_VOLT;
  if (indeks < 0.0) {
    indeks = 0.0;
  }
  return indeks;
}

// Kategori indeks UV menurut skala WHO.
const char* kategoriIndeksUV(float indeks) {
  if (indeks < AMBANG_SEDANG) {
    return "RENDAH";
  }
  if (indeks < AMBANG_TINGGI) {
    return "SEDANG";
  }
  if (indeks < AMBANG_SANGAT_TINGGI) {
    return "TINGGI";
  }
  if (indeks < AMBANG_EKSTREM) {
    return "SANGAT TINGGI";
  }
  return "EKSTREM";
}

// Tampilkan indeks UV + kategori di LCD 16x2.
// Baris 1: "UV Index: 7.5" — baris 2: kategori.
void tampilkanLCD(float indeks, const char* kategori) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UV Index: ");
  lcd.print(indeks, 1);
  lcd.setCursor(0, 1);
  lcd.print(kategori);
}

void setup() {
  Serial.begin(115200);
  // Resolusi ADC 12-bit (0-4095), sesuai analogReadMilliVolts.
  analogReadResolution(12);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Pengukur UV");
  lcd.setCursor(0, 1);
  lcd.print("ML8511 siap...");
  delay(1000);
}

void loop() {
  float tegangan = bacaTeganganSensor();
  float indeks = hitungIndeksUV(tegangan);
  const char* kategori = kategoriIndeksUV(indeks);

  tampilkanLCD(indeks, kategori);

  Serial.print("Tegangan: ");
  Serial.print(tegangan, 2);
  Serial.print(" V | Indeks UV: ");
  Serial.print(indeks, 1);
  Serial.print(" | ");
  Serial.println(kategori);

  delay(INTERVAL_BACA_MS);
}
