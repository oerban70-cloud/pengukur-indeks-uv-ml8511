# Pengukur Indeks UV — ESP32 + ML8511 + LCD I2C

Matahari terasa biasa saja, tapi kulitmu bisa terbakar diam-diam. Alat kecil
ini membaca sinar UV dengan modul ML8511 dan mengubahnya jadi angka indeks
UV 0 sampai 11 lebih, lengkap dengan kategorinya — Rendah, Sedang, Tinggi,
Sangat Tinggi, atau Ekstrem — tampil di LCD 16x2.

Video penjelasan (60 detik):
https://www.youtube.com/shorts/LdSjxymwpg8

## Komponen

| Bagian | Keterangan |
|---|---|
| ESP32 DevKit | Otak alat |
| ML8511 | Modul sensor UV analog (Lapis Semiconductor), output tegangan |
| LCD 16x2 I2C | Tampil indeks UV + kategori (backpack PCF8574, alamat 0x27) |
| Adaptor USB 5V 2A | Catu daya ESP32 |

## Wiring

| Dari | Ke |
|---|---|
| ML8511 VIN | ESP32 3V3 |
| ML8511 GND | ESP32 GND |
| ML8511 OUT | ESP32 GPIO34 (ADC1, input analog) |
| LCD VCC | ESP32 3V3 |
| LCD GND | ESP32 GND |
| LCD SDA | ESP32 GPIO21 (SDA I2C) |
| LCD SCL | ESP32 GPIO22 (SCL I2C) |

Skema lengkap: [schematic.png](schematic.png)

## Cara kerja

1. ML8511 adalah sensor UV analog: fotodioda UV di dalam chip menangkap
   sinar UV 280-390 nm (UV-A + UV-B, puncak sensitivitas 365 nm), lalu
   penguat arus-ke-tegangan mengubahnya jadi tegangan output yang naik
   linear dengan intensitas UV.
2. ESP32 membaca tegangan output lewat ADC 12-bit di GPIO34
   (`analogReadMilliVolts`, dirata-rata dari 8 sampel) lalu menghitung
   indeks UV dengan rumus datasheet ML8511 (FEDL8511-00 Rev1.2):
   indeks UV = (tegangan output - 1,0 V) x 12,5. Tanpa sinar UV,
   tegangan output sekitar 1,0 V sehingga indeksnya 0; pada intensitas
   10 mW/cm2 outputnya 2,2 V sehingga indeksnya 15.
3. Indeks UV dikelompokkan menurut skala WHO: 0-2 Rendah, 3-5 Sedang,
   6-7 Tinggi, 8-10 Sangat Tinggi, 11 ke atas Ekstrem.
4. ESP32 membaca sensor dan memperbarui LCD tiap 500 milidetik (2 kali
   per detik). LCD baris pertama menampilkan "UV Index: X.X", baris
   kedua menampilkan kategorinya. Tegangan dan indeks juga dikirim ke
   Serial Monitor (115200 baud) untuk debugging.
5. Modul ML8511 dicatu 3,3 V (rentang kerja 2,7-3,6 V) dan LCD berbagi
   rel 3V3 yang sama. Modul ML8511 yang dipakai di sini 3 pin
   (VIN, GND, OUT) — tanpa pin EN.

## Angka acuan

- ML8511 (Lapis Semiconductor, datasheet FEDL8511-00 Rev1.2): sensor UV
  analog, deteksi 280-390 nm, puncak sensitivitas 365 nm.
- Tegangan output tanpa UV: sekitar 1,0 V (0,95-1,05 V).
- Tegangan output pada 10 mW/cm2: 2,2 V (2,08-2,32 V).
- Relasi datasheet: indeks UV / (tegangan output - tegangan tanpa UV)
  = 12,5 indeks UV per volt.
- Catu ML8511: 2,7-3,6 V (tipikal 3,3 V); arus aktif tipikal 300 uA.
- Skala indeks UV (WHO): 0-2 Rendah, 3-5 Sedang, 6-7 Tinggi,
  8-10 Sangat Tinggi, 11+ Ekstrem.
- ADC ESP32: 12-bit (0-4095), dibaca lewat `analogReadMilliVolts`
  (kalibrasi internal) dan dirata-rata 8 sampel.

## Konfigurasi (di awal sketch)

- `PIN_ML8511_OUT` (34) — pin analog output ML8511 (ADC1).
- `PIN_I2C_SDA` (21) / `PIN_I2C_SCL` (22) — pin bus I2C untuk LCD.
- `ADDR_LCD` (0x27) — alamat I2C LCD 16x2.
- `TEGANGAN_TANPA_UV` (1,0) — tegangan output ML8511 tanpa sinar UV.
- `FAKTOR_UV_PER_VOLT` (12,5) — faktor konversi datasheet.
- `AMBANG_SEDANG` (3,0) / `AMBANG_TINGGI` (6,0) /
  `AMBANG_SANGAT_TINGGI` (8,0) / `AMBANG_EKSTREM` (11,0) — ambang kategori.
- `JUMLAH_SAMPEL` (8) — sampel ADC yang dirata-rata per pembacaan.
- `INTERVAL_BACA_MS` (500) — seberapa sering sensor dibaca dan LCD
  diperbarui (2 kali per detik).

## Library yang dibutuhkan

| Library | Versi | Keterangan |
|---|---|---|
| LiquidCrystal I2C | 1.1.2 | Driver LCD 16x2 I2C (Frank de Brabander) |

Library di atas juga tersedia sebagai ZIP di folder [`libraries/`](libraries/)
— install lewat Arduino IDE: Sketch → Include Library → Add .ZIP Library.
Modul ML8511 tidak butuh library tambahan (dibaca analog langsung).

## Compile

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 PengukurIndeksUvMl8511
```

Terverifikasi 26 Nov 2026: sketch 324.821 byte (24% flash),
RAM 24.704 byte (7%), ESP32 core 4.0.0.

## Lisensi

MIT — lihat [LICENSE](LICENSE).
