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
g++ snake_neon_compat.cpp -o snake_neon.exe
snake_neon.exe
```

Dengan Falcon C++: buka `snake_neon_compat.cpp`, lalu tekan **Compile & Run**.

> Disarankan menjalankan di CMD atau PowerShell klasik agar ukuran jendela dan warna tampil dengan benar.

## Struktur File
- `snake_neon_compat.cpp` : kode sumber utama (kompatibel dengan compiler lama)
- `highscore.txt` : dibuat otomatis setelah game pertama selesai

## Catatan
Game ini memakai `conio.h` dan `windows.h`, sehingga hanya berjalan di Windows.

## Pembuat
Farrel Anshari
