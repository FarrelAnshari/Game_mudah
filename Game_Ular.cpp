// ============================================================
//  SNAKE NEON - Console Game (Windows)
//  Compile : g++ snake_neon.cpp -o snake_neon.exe
//  Jalankan di CMD / PowerShell (konsol klasik lebih disarankan)
// ============================================================
#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstring>
#include <sstream>
using namespace std;

// --- kompatibilitas compiler lama ---
#define GetTickCount64 GetTickCount
string angka(int n) { ostringstream o; o << n; return o.str(); }

// ---------------- Konstanta ----------------
const int LEBAR = 32;     // lebar arena (sel), termasuk dinding
const int TINGGI = 22;    // tinggi arena (sel), termasuk dinding
const int KOLOM = 90;     // lebar jendela konsol (karakter)
const int BARIS = 25;     // tinggi jendela konsol

const char* NAMA_KESULITAN[3] = { "Mudah", "Sedang", "Sulit" };
const char* NAMA_MODE[2] = { "Klasik", "Tembus" };

const char* HURUF[5][5] = {
    { " ____  ", "/ ___| ", "\\___ \\ ", " ___) |", "|____/ " },
    { " _   _ ", "| \\ | |", "|  \\| |", "| |\\  |", "|_| \\_|" },
    { "    _    ", "   / \\   ", "  / _ \\  ", " / ___ \\ ", "/_/   \\_\\" },
    { " _  __", "| |/ /", "| ' / ", "| . \\ ", "|_|\\_\\" },
    { " _____ ", "| ____|", "|  _|  ", "| |___ ", "|_____|" }
};

// ---------------- Struktur & variabel global ----------------
struct Titik { int x, y; };
Titik buatTitik(int x, int y) { Titik t; t.x = x; t.y = y; return t; }
struct Popup { int x, y, umur; string teks; };

HANDLE hOut;
CHAR_INFO layar[BARIS][KOLOM];

deque<Titik> ular;
deque<Titik> antrian;               // antrian arah (anti putar balik 180 derajat)
bool rintangan[TINGGI][LEBAR];
Titik makanan, bonus;
bool bonusAktif = false;
ULONGLONG bonusSelesai = 0, bannerSampai = 0;
int dx = 1, dy = 0;
int skor = 0, rekor = 0, level = 1, jumlahMakan = 0, delayMs = 140;
int kesulitan = 1, mode = 0;        // mode 0 = klasik, 1 = tembus dinding
int kilatMakan = 0, efekMati = 0;
bool gameOver = false, jeda = false, keluarGame = false;
Popup popup = { 0, 0, 0, "" };

// ---------------- Buffer layar (anti kedip) ----------------
void bersihkan() {
    for (int y = 0; y < BARIS; y++)
        for (int x = 0; x < KOLOM; x++) {
            layar[y][x].Char.AsciiChar = ' ';
            layar[y][x].Attributes = 0x07;
        }
}

void taruh(int x, int y, char c, WORD a) {
    if (x < 0 || x >= KOLOM || y < 0 || y >= BARIS) return;
    layar[y][x].Char.AsciiChar = c;
    layar[y][x].Attributes = a;
}

void tulis(int x, int y, const string& s, WORD a) {
    for (size_t i = 0; i < s.size(); i++) taruh(x + (int)i, y, s[i], a);
}

void tulisTengah(int y, const string& s, WORD a) {
    tulis((KOLOM - (int)s.size()) / 2, y, s, a);
}

void tengahDi(int x0, int lebar, int y, const string& s, WORD a) {
    tulis(x0 + (lebar - (int)s.size()) / 2, y, s, a);
}

void kotak(int x, int y, int w, int h, WORD a) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            char c = ' ';
            bool atas = (j == 0 || j == h - 1), sisi = (i == 0 || i == w - 1);
            if (atas && sisi) c = '+';
            else if (atas) c = '-';
            else if (sisi) c = '|';
            taruh(x + i, y + j, c, a);
        }
}

void tampilkan() {
    COORD ukuran = { (SHORT)KOLOM, (SHORT)BARIS }, asal = { 0, 0 };
    SMALL_RECT area = { 0, 0, (SHORT)(KOLOM - 1), (SHORT)(BARIS - 1) };
    WriteConsoleOutputA(hOut, &layar[0][0], ukuran, asal, &area);
}

void siapkanKonsol() {
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    system("title Snake Neon");
    system("mode con cols=90 lines=25");
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(hOut, &ci);
}

// ---------------- Input ----------------
// Mengembalikan: 0 = tidak ada tombol, -1 = tombol diabaikan,
// selain itu karakter huruf kecil. Tombol panah dipetakan ke w/a/s/d.
int bacaTombol() {
    if (!_kbhit()) return 0;
    int k = _getch();
    if (k == 0 || k == 224) {
        k = _getch();
        switch (k) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
            default: return -1;
        }
    }
    return tolower(k);
}

// ---------------- Rekor ----------------
void muatRekor() {
    ifstream f("highscore.txt");
    if (!(f >> rekor)) rekor = 0;
}

void simpanRekor() {
    ofstream f("highscore.txt");
    f << rekor;
}

// ---------------- Logika game ----------------
int hitungDelay() {
    int dasar[3] = { 140, 105, 75 };
    int d = dasar[kesulitan] - (level - 1) * 7;
    return d < 40 ? 40 : d;
}

bool dipakai(int x, int y) {
    if (rintangan[y][x]) return true;
    for (size_t i = 0; i < ular.size(); i++)
        if (ular[i].x == x && ular[i].y == y) return true;
    if (x == makanan.x && y == makanan.y) return true;
    if (bonusAktif && x == bonus.x && y == bonus.y) return true;
    return false;
}

Titik acak() {
    Titik t;
    do {
        t.x = rand() % (LEBAR - 2) + 1;
        t.y = rand() % (TINGGI - 2) + 1;
    } while (dipakai(t.x, t.y));
    return t;
}

void tambahRintangan(int n) {
    for (int i = 0; i < n; i++) {
        for (int coba = 0; coba < 200; coba++) {
            Titik t = acak();
            if (abs(t.x - ular[0].x) + abs(t.y - ular[0].y) < 7) continue;
            rintangan[t.y][t.x] = true;
            break;
        }
    }
}

void mulaiGame() {
    for (int y = 0; y < TINGGI; y++)
        for (int x = 0; x < LEBAR; x++) rintangan[y][x] = false;
    ular.clear();
    for (int i = 0; i < 3; i++) ular.push_back(buatTitik(LEBAR / 2 - i, TINGGI / 2));
    dx = 1; dy = 0;
    antrian.clear();
    skor = 0; level = 1; jumlahMakan = 0;
    gameOver = false; kilatMakan = 0; efekMati = 0;
    bonusAktif = false; bannerSampai = 0;
    popup.umur = 0;
    makanan = buatTitik(-1, -1);
    makanan = acak();
    delayMs = hitungDelay();
}

void naikLevel() {
    level++;
    delayMs = hitungDelay();
    bannerSampai = GetTickCount64() + 2000;
    if (level >= 3 && level <= 15) tambahRintangan(3);
}

void bacaInput() {
    int k;
    while ((k = bacaTombol()) != 0) {
        int nx = 0, ny = 0;
        bool baru = false;
        if (k == 'w') { ny = -1; baru = true; }
        else if (k == 's') { ny = 1; baru = true; }
        else if (k == 'a') { nx = -1; baru = true; }
        else if (k == 'd') { nx = 1; baru = true; }
        else if (k == 'p') jeda = true;
        else if (k == 'x' || k == 27) keluarGame = true;

        if (baru) {
            Titik terakhir = antrian.empty() ? buatTitik(dx, dy) : antrian.back();
            bool sama = (nx == terakhir.x && ny == terakhir.y);
            bool balik = (nx == -terakhir.x && ny == -terakhir.y);
            if (!sama && !balik && antrian.size() < 3) antrian.push_back(buatTitik(nx, ny));
        }
    }
}

void langkah() {
    if (!antrian.empty()) {
        dx = antrian.front().x;
        dy = antrian.front().y;
        antrian.pop_front();
    }

    Titik kepala = { ular[0].x + dx, ular[0].y + dy };

    if (mode == 1) {                       // tembus dinding
        if (kepala.x <= 0) kepala.x = LEBAR - 2;
        else if (kepala.x >= LEBAR - 1) kepala.x = 1;
        if (kepala.y <= 0) kepala.y = TINGGI - 2;
        else if (kepala.y >= TINGGI - 1) kepala.y = 1;
    } else if (kepala.x <= 0 || kepala.x >= LEBAR - 1 ||
               kepala.y <= 0 || kepala.y >= TINGGI - 1) {
        gameOver = true;
        return;
    }

    if (rintangan[kepala.y][kepala.x]) { gameOver = true; return; }

    bool makan = (kepala.x == makanan.x && kepala.y == makanan.y);
    bool makanBonus = bonusAktif && kepala.x == bonus.x && kepala.y == bonus.y;
    bool tumbuh = makan || makanBonus;

    size_t batas = ular.size() - (tumbuh ? 0 : 1);   // ekor akan bergeser
    for (size_t i = 0; i < batas; i++)
        if (ular[i].x == kepala.x && ular[i].y == kepala.y) { gameOver = true; return; }

    ular.push_front(kepala);
    if (!tumbuh) ular.pop_back();

    int poin = 10 * (kesulitan + 1);
    if (makan) {
        skor += poin;
        jumlahMakan++;
        kilatMakan = 9;
        popup.x = kepala.x; popup.y = kepala.y; popup.umur = 18;
        popup.teks = "+" + angka(poin);
        makanan = acak();
        if (jumlahMakan % 5 == 0) naikLevel();
        if (jumlahMakan % 4 == 0 && !bonusAktif) {
            bonus = acak();
            bonusAktif = true;
            bonusSelesai = GetTickCount64() + 7000;
        }
    } else if (makanBonus) {
        skor += poin * 5;
        bonusAktif = false;
        kilatMakan = 15;
        popup.x = kepala.x; popup.y = kepala.y; popup.umur = 18;
        popup.teks = "+" + angka(poin * 5) + "!";
    }
}

// ---------------- Tampilan ----------------
void gambarPanel(ULONGLONG now) {
    int px = 68;
    tulis(px, 1, "=== SNAKE NEON ===", (WORD)(0x09 + (now / 150) % 6));
    tulis(px, 3, "SKOR   : " + angka(skor), 0x0F);
    tulis(px, 4, "REKOR  : " + angka(skor > rekor ? skor : rekor), 0x0E);
    tulis(px, 5, "LEVEL  : " + angka(level), 0x0B);
    tulis(px, 6, "PANJANG: " + angka(ular.size()), 0x0A);

    int p = jumlahMakan % 5;
    string bar = "[";
    for (int i = 0; i < 5; i++) bar += (i < p) ? '#' : '-';
    bar += "]";
    tulis(px, 8, "Menuju level:", 0x07);
    tulis(px, 9, bar, 0x0A);

    int k = (170 - delayMs) / 13;
    if (k < 1) k = 1;
    if (k > 10) k = 10;
    string b2 = "[";
    for (int i = 0; i < 10; i++) b2 += (i < k) ? '>' : ' ';
    b2 += "]";
    tulis(px, 11, "Kecepatan:", 0x07);
    tulis(px, 12, b2, 0x0C);

    if (bonusAktif) {
        long long sisa = (long long)bonusSelesai - (long long)now;
        if (sisa < 0) sisa = 0;
        int n = (int)(sisa * 10 / 7000);
        string b3 = "[";
        for (int i = 0; i < 10; i++) b3 += (i < n) ? '=' : ' ';
        b3 += "]";
        tulis(px, 14, "BONUS $$ !", ((now / 150) % 2) ? 0x0E : 0x0D);
        tulis(px, 15, b3, 0x0E);
    }

    tulis(px, 17, string("Level : ") + NAMA_KESULITAN[kesulitan], 0x07);
    tulis(px, 18, string("Dinding: ") + NAMA_MODE[mode], 0x07);
    tulis(px, 20, "WASD/Panah: gerak", 0x08);
    tulis(px, 21, "P: jeda  X: keluar", 0x08);
}

void gambarGame(ULONGLONG now) {
    bersihkan();

    // arena
    for (int y = 0; y < TINGGI; y++)
        for (int x = 0; x < LEBAR; x++) {
            int cx = x * 2;
            bool dinding = (x == 0 || y == 0 || x == LEBAR - 1 || y == TINGGI - 1);
            if (dinding) {
                if (mode == 0) {
                    taruh(cx, y, ' ', 0x70);
                    taruh(cx + 1, y, ' ', 0x70);
                } else {
                    WORD a = ((now / 300 + x + y) % 2) ? 0x19 : 0x13;   // dinding berkilau
                    taruh(cx, y, ':', a);
                    taruh(cx + 1, y, ':', a);
                }
            } else if (rintangan[y][x]) {
                taruh(cx, y, '#', 0x6E);
                taruh(cx + 1, y, '#', 0x6E);
            } else {
                taruh(cx, y, ' ', 0x08);
                taruh(cx + 1, y, ((x + y) % 2) ? '.' : ' ', 0x08);
            }
        }

    // makanan (berdenyut)
    {
        WORD a = ((now / 250) % 2) ? 0xCE : 0x4C;
        taruh(makanan.x * 2, makanan.y, '(', a);
        taruh(makanan.x * 2 + 1, makanan.y, ')', a);
    }

    // makanan bonus (berkedip, makin cepat saat hampir habis)
    if (bonusAktif) {
        long long sisa = (long long)bonusSelesai - (long long)now;
        bool tampil = (sisa > 2000) || ((now / 100) % 2);
        WORD a = ((now / 150) % 2) ? 0xE5 : 0x5E;
        if (tampil) {
            taruh(bonus.x * 2, bonus.y, '$', a);
            taruh(bonus.x * 2 + 1, bonus.y, '$', a);
        }
    }

    // ular
    for (size_t i = 0; i < ular.size(); i++) {
        int cx = ular[i].x * 2, cy = ular[i].y;
        WORD a;
        char c1 = ' ', c2 = ' ';
        if (efekMati == 1) a = 0x4C;
        else if (efekMati == 2) a = 0xF0;
        else if ((kilatMakan / 3) % 2 == 1) a = 0xF0;       // kilat putih saat makan
        else if (i == 0) a = 0xB0;                          // kepala aqua
        else a = (i % 2) ? 0x20 : 0xA0;                     // badan belang hijau
        if (i == 0) {                                       // mata sesuai arah
            if (dx == 1) { c1 = ' '; c2 = ':'; }
            else if (dx == -1) { c1 = ':'; c2 = ' '; }
            else if (dy == -1) { c1 = '\''; c2 = '\''; }
            else { c1 = '.'; c2 = '.'; }
        }
        taruh(cx, cy, c1, a);
        taruh(cx + 1, cy, c2, a);
    }

    // popup skor melayang
    if (popup.umur > 0) {
        int rise = (18 - popup.umur) / 4;
        int py = popup.y - 1 - rise;
        int px = popup.x * 2;
        int len = (int)popup.teks.size();
        if (px + len > 62) px = 62 - len;
        if (py >= 1) tulis(px, py, popup.teks, 0x0E);
    }

    gambarPanel(now);

    // banner level
    if (now < bannerSampai) {
        string b = "*** LEVEL " + angka(level) + " ***";
        tulis((LEBAR * 2 - (int)b.size()) / 2, 23, b, ((now / 120) % 2) ? 0x0E : 0x0C);
    }
}

void hitungMundur() {
    for (int n = 3; n >= 0; n--) {
        gambarGame(GetTickCount64());
        kotak(28, 8, 9, 5, 0x1F);
        if (n > 0) taruh(32, 10, (char)('0' + n), 0x1E);
        else tengahDi(28, 9, 10, "GO!", 0x1A);
        tampilkan();
        Sleep(n > 0 ? 700 : 350);
    }
    while (_kbhit()) _getch();
    antrian.clear();
}

void tampilJeda() {
    gambarGame(GetTickCount64());
    kotak(22, 8, 20, 6, 0x1F);
    tengahDi(22, 20, 10, "== JEDA ==", 0x1E);
    tengahDi(22, 20, 12, "P / Enter: lanjut", 0x1F);
    tampilkan();
    while (true) {
        int k = bacaTombol();
        if (k == 'p' || k == 13 || k == ' ') break;
        if (k == 'x' || k == 27) { keluarGame = true; break; }
        Sleep(30);
    }
    while (_kbhit()) _getch();
}

// 0 = main lagi, 1 = kembali ke menu
int layarGameOver(bool rekorBaru) {
    while (_kbhit()) _getch();
    while (true) {
        ULONGLONG now = GetTickCount64();
        gambarGame(now);
        kotak(18, 5, 28, 12, 0x1F);
        tengahDi(18, 28, 7, "G A M E   O V E R", ((now / 300) % 2) ? 0x1C : 0x1F);
        tengahDi(18, 28, 9, "Skor  : " + angka(skor), 0x1F);
        tengahDi(18, 28, 10, "Rekor : " + angka(rekor), 0x1F);
        if (rekorBaru) tengahDi(18, 28, 11, "** REKOR BARU! **", ((now / 200) % 2) ? 0x1E : 0x1A);
        tengahDi(18, 28, 13, "R / Enter : Main lagi", 0x1B);
        tengahDi(18, 28, 14, "M : Menu utama", 0x1B);
        tampilkan();
        int k = bacaTombol();
        if (k == 'r' || k == 13) return 0;
        if (k == 'm' || k == 27) return 1;
        Sleep(30);
    }
}

int jalankanGame() {
    mulaiGame();
    jeda = false;
    keluarGame = false;
    efekMati = 0;
    while (_kbhit()) _getch();
    hitungMundur();

    ULONGLONG terakhirLangkah = GetTickCount64();
    ULONGLONG terakhirFrame = 0;

    while (!gameOver && !keluarGame) {
        ULONGLONG now = GetTickCount64();
        bacaInput();
        if (keluarGame) break;

        if (jeda) {
            jeda = false;
            tampilJeda();
            terakhirLangkah = GetTickCount64();
            continue;
        }

        if (bonusAktif && now >= bonusSelesai) bonusAktif = false;

        if (now - terakhirLangkah >= (ULONGLONG)delayMs) {
            terakhirLangkah = now;
            langkah();
        }

        if (now - terakhirFrame >= 33) {             // ~30 FPS untuk animasi
            terakhirFrame = now;
            gambarGame(now);
            tampilkan();
            if (kilatMakan > 0) kilatMakan--;
            if (popup.umur > 0) popup.umur--;
        }
        Sleep(2);
    }

    bool rekorBaru = (skor > rekor && skor > 0);
    if (rekorBaru) { rekor = skor; simpanRekor(); }

    if (keluarGame) { keluarGame = false; return 1; }

    // animasi kematian
    for (int i = 0; i < 8; i++) {
        efekMati = (i % 2) ? 2 : 1;
        gambarGame(GetTickCount64());
        tampilkan();
        Sleep(130);
    }
    efekMati = 1;
    return layarGameOver(rekorBaru);
}

// ---------------- Menu utama ----------------
bool menuUtama() {
    int pilih = 0;
    while (_kbhit()) _getch();
    while (true) {
        ULONGLONG now = GetTickCount64();
        bersihkan();

        // judul berwarna bergerak
        int x0 = (KOLOM - 36) / 2;
        for (int r = 0; r < 5; r++) {
            int x = x0;
            for (int l = 0; l < 5; l++) {
                WORD w = (WORD)(9 + (l + now / 150) % 6);
                tulis(x, 2 + r, HURUF[l][r], w);
                x += (int)strlen(HURUF[l][r]);
            }
        }
        tulisTengah(8, "~ edisi neon  -  C++ console ~", 0x07);

        // ular hias mengejar makanan
        int hx = (int)((now / 70) % KOLOM);
        taruh((hx + 8) % KOLOM, 10, '(', 0xCE);
        taruh((hx + 9) % KOLOM, 10, ')', 0xCE);
        for (int i = 0; i < 12; i++) {
            int px = ((hx - i * 2) % KOLOM + KOLOM) % KOLOM;
            WORD a = (i == 0) ? 0xB0 : ((i % 2) ? 0x20 : 0xA0);
            taruh(px, 10, ' ', a);
            taruh((px + 1) % KOLOM, 10, ' ', a);
        }

        // item menu
        string teks[4];
        teks[0] = "MULAI MAIN";
        teks[1] = string("Level  : < ") + NAMA_KESULITAN[kesulitan] + " >";
        teks[2] = string("Dinding: < ") + NAMA_MODE[mode] + " >";
        teks[3] = "KELUAR";
        for (int i = 0; i < 4; i++) {
            string s = teks[i];
            WORD a = 0x07;
            if (i == pilih) {
                s = "> " + s + " <";
                a = ((now / 300) % 2) ? 0x0E : 0x0F;
            }
            tulisTengah(13 + i * 2, s, a);
        }

        tulisTengah(21, "REKOR : " + angka(rekor), 0x0E);
        tulisTengah(23, "W/S / panah: pilih    A/D: ubah    Enter: OK", 0x08);
        tampilkan();

        int k = bacaTombol();
        if (k == 'w') pilih = (pilih + 3) % 4;
        else if (k == 's') pilih = (pilih + 1) % 4;
        else if (k == 'a' || k == 'd') {
            if (pilih == 1) kesulitan = (kesulitan + (k == 'd' ? 1 : 2)) % 3;
            else if (pilih == 2) mode = 1 - mode;
        } else if (k == 13 || k == ' ') {
            if (pilih == 0) return true;
            if (pilih == 1) kesulitan = (kesulitan + 1) % 3;
            if (pilih == 2) mode = 1 - mode;
            if (pilih == 3) return false;
        } else if (k == 27) return false;

        Sleep(30);
    }
}

// ---------------- main ----------------
int main() {
    srand((unsigned)time(0));
    siapkanKonsol();
    muatRekor();

    while (menuUtama()) {
        int hasil;
        do {
            hasil = jalankanGame();
        } while (hasil == 0);
    }

    SetConsoleTextAttribute(hOut, 0x07);
    system("cls");
    CONSOLE_CURSOR_INFO ci = { 25, TRUE };
    SetConsoleCursorInfo(hOut, &ci);
    cout << "Terima kasih sudah bermain! Rekor tertinggi: " << rekor << endl;
    return 0;
}