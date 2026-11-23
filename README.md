# Snake Neon 🐍

Game Snake berbasis konsol Windows yang dibuat dengan C++. Tampilannya berwarna, ada animasi, menu, level, dan penyimpanan rekor.

## Fitur
- Tampilan berwarna dengan animasi (makanan berdenyut, efek kilat saat makan, animasi game over)
- Menu utama dengan pilihan tingkat kesulitan (Mudah / Sedang / Sulit)
- Dua mode dinding: **Klasik** (tabrak dinding = kalah) dan **Tembus** (muncul di sisi seberang)
- Naik level setiap 5 makanan, kecepatan bertambah, dan rintangan muncul mulai level 3
- Makanan bonus `$$` bernilai 5x lipat dengan batas waktu
- Rekor tertinggi tersimpan otomatis di `highscore.txt`
- Fitur jeda (pause) dan dukungan tombol panah maupun WASD

## Kontrol
| Tombol | Fungsi |
|--------|--------|
| W / A / S / D atau tombol panah | Menggerakkan ular |
| P | Jeda |
| X / Esc | Keluar ke menu |
| Enter | Memilih menu |

## Cara Menjalankan
**Persyaratan:** Windows dan compiler C++ (MinGW / g++ atau Falcon C++).

Dengan g++:
```
g++ Game_Ular.cpp -o Game_Ular.exe
Game_Ular.exe
```

Dengan Falcon C++: buka `Game_Ular.cpp`, lalu tekan **Compile & Run**.

> Disarankan menjalankan di CMD atau PowerShell klasik agar ukuran jendela dan warna tampil dengan benar.

## Struktur File
- `Game_Ular.cpp` : kode sumber utama game Snake Neon
- `highscore.txt` : dibuat otomatis setelah game pertama selesai

## Catatan
Game ini memakai `conio.h` dan `windows.h`, sehingga hanya berjalan di Windows.

## Pembuat
Farrel Anshari
