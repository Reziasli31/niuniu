#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

// --- PIN LAMPU 4 JALUR ---
const int U_RED = 30;
const int U_YELLOW = 31;
const int U_GREEN = 32;

const int B_RED = 33;
const int B_YELLOW = 34;
const int B_GREEN = 35;

const int S_RED = 36;
const int S_YELLOW = 37;
const int S_GREEN = 38;

const int T_RED = 39;
const int T_YELLOW = 40;
const int T_GREEN = 41;

// --- KONFIGURASI KEYPAD ---
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

uint8_t rowpins[ROWS] = {22, 23, 24, 25};
uint8_t colpins[COLS] = {26, 27, 28, 29};

Keypad keypad = Keypad(makeKeymap(keys), rowpins, colpins, ROWS, COLS);

// --- DURASI DEFAULT MASING-MASING JALUR ---
const int DEF_UTARA   = 7;
const int DEF_BARAT   = 7;
const int DEF_SELATAN = 7;
const int DEF_TIMUR   = 7;

int durasiUtara   = DEF_UTARA;
int durasiBarat   = DEF_BARAT;
int durasiSelatan = DEF_SELATAN;
int durasiTimur   = DEF_TIMUR;

String inputBuffer = ""; 

// Enum untuk melacak target jalur yang sedang dipilih (berputar lewat tombol B)
enum TargetJalur {
  TARGET_UTARA,
  TARGET_BARAT,
  TARGET_SELATAN,
  TARGET_TIMUR
};
TargetJalur jalurDipilih = TARGET_UTARA; // Default awal ke Utara

// --- STATE MACHINE SIKLUS 4 ARAH ---
enum TrafficState {
  U_HIJAU,
  U_KUNING_TRANSISI,
  B_HIJAU,
  B_KUNING_TRANSISI,
  S_HIJAU,
  S_KUNING_TRANSISI,
  T_HIJAU,
  T_KUNING_TRANSISI
};

TrafficState currentState = U_HIJAU;
unsigned long waktuMulai = 0;
unsigned long durasiAktif = 5000; 

// Fungsi untuk mematikan semua lampu
void matikanSemuaLampu() {
  digitalWrite(U_RED, LOW); digitalWrite(U_YELLOW, LOW); digitalWrite(U_GREEN, LOW);
  digitalWrite(B_RED, LOW); digitalWrite(B_YELLOW, LOW); digitalWrite(B_GREEN, LOW);
  digitalWrite(S_RED, LOW); digitalWrite(S_YELLOW, LOW); digitalWrite(S_GREEN, LOW);
  digitalWrite(T_RED, LOW); digitalWrite(T_YELLOW, LOW); digitalWrite(T_GREEN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(U_RED, OUTPUT); pinMode(U_YELLOW, OUTPUT); pinMode(U_GREEN, OUTPUT);
  pinMode(B_RED, OUTPUT); pinMode(B_YELLOW, OUTPUT); pinMode(B_GREEN, OUTPUT);
  pinMode(S_RED, OUTPUT); pinMode(S_YELLOW, OUTPUT); pinMode(S_GREEN, OUTPUT);
  pinMode(T_RED, OUTPUT); pinMode(T_YELLOW, OUTPUT); pinMode(T_GREEN, OUTPUT);

  // Default awal: Semua Merah, Utara mulai Hijau
  matikanSemuaLampu();
  digitalWrite(U_RED, LOW);  digitalWrite(U_GREEN, HIGH);
  digitalWrite(B_RED, HIGH);
  digitalWrite(S_RED, HIGH);
  digitalWrite(T_RED, HIGH);

  waktuMulai = millis();
  durasiAktif = durasiUtara * 1000;
  
  Serial.println("=================================================");
  Serial.println("Sistem Lampu Lalu Lintas 4 Arah + Keypad");
  Serial.println("Tekan [B] untuk memindah target jalur edit:");
  Serial.println(" (Urutan: UTARA -> BARAT -> SELATAN -> TIMUR)");
  Serial.println("=================================================");
  Serial.println("Target Edit Saat Ini: [ UTARA ]");
}

// --- FUNGSI UTAMA SIKLUS LAMPU ---
void updateSiklusLaluLintas() {
  unsigned long waktuSekarang = millis();

  if (waktuSekarang - waktuMulai >= durasiAktif) {
    switch (currentState) {
      case U_HIJAU:
        digitalWrite(U_GREEN, LOW);
        digitalWrite(U_YELLOW, HIGH);
        digitalWrite(B_RED, HIGH);
        digitalWrite(B_YELLOW, HIGH);
        currentState = U_KUNING_TRANSISI;
        durasiAktif = 2000; 
        break;

      case U_KUNING_TRANSISI:
        digitalWrite(U_YELLOW, LOW);
        digitalWrite(U_RED, HIGH);
        digitalWrite(B_YELLOW, LOW);
        digitalWrite(B_RED, LOW);
        digitalWrite(B_GREEN, HIGH);
        currentState = B_HIJAU;
        durasiAktif = durasiBarat * 1000;
        break;

      case B_HIJAU:
        digitalWrite(B_GREEN, LOW);
        digitalWrite(B_YELLOW, HIGH);
        digitalWrite(S_RED, HIGH);
        digitalWrite(S_YELLOW, HIGH);
        currentState = B_KUNING_TRANSISI;
        durasiAktif = 2000;
        break;

      case B_KUNING_TRANSISI:
        digitalWrite(B_YELLOW, LOW);
        digitalWrite(B_RED, HIGH);
        digitalWrite(S_YELLOW, LOW);
        digitalWrite(S_RED, LOW);
        digitalWrite(S_GREEN, HIGH);
        currentState = S_HIJAU;
        durasiAktif = durasiSelatan * 1000;
        break;

      case S_HIJAU:
        digitalWrite(S_GREEN, LOW);
        digitalWrite(S_YELLOW, HIGH);
        digitalWrite(T_RED, HIGH);
        digitalWrite(T_YELLOW, HIGH);
        currentState = S_KUNING_TRANSISI;
        durasiAktif = 2000;
        break;

      case S_KUNING_TRANSISI:
        digitalWrite(S_YELLOW, LOW);
        digitalWrite(S_RED, HIGH);
        digitalWrite(T_YELLOW, LOW);
        digitalWrite(T_RED, LOW);
        digitalWrite(T_GREEN, HIGH);
        currentState = T_HIJAU;
        durasiAktif = durasiTimur * 1000;
        break;

      case T_HIJAU:
        digitalWrite(T_GREEN, LOW);
        digitalWrite(T_YELLOW, HIGH);
        digitalWrite(U_RED, HIGH);
        digitalWrite(U_YELLOW, HIGH);
        currentState = T_KUNING_TRANSISI;
        durasiAktif = 2000;
        break;

      case T_KUNING_TRANSISI:
        digitalWrite(T_YELLOW, LOW);
        digitalWrite(T_RED, HIGH);
        digitalWrite(U_YELLOW, LOW);
        digitalWrite(U_RED, LOW);
        digitalWrite(U_GREEN, HIGH);
        currentState = U_HIJAU;
        durasiAktif = durasiUtara * 1000;
        break;
    }
    waktuMulai = millis();
  }
}

// --- FUNGSI KEYPAD DENGAN TOMBOL 'B' BERPUTAR ---
void bacaKeypad() {
  char key = keypad.getKey();

  if (key) {
    // 1. TOMBOL 'B' UNTUK BERPINDAH TARGET JALUR BERPUTAR (UTARA -> BARAT -> SELATAN -> TIMUR)
    if (key == 'B') {
      switch (jalurDipilih) {
        case TARGET_UTARA:
          jalurDipilih = TARGET_BARAT;
          Serial.println("\n>>> Target Edit Dipindah ke: [ BARAT ] <<<");
          break;
        case TARGET_BARAT:
          jalurDipilih = TARGET_SELATAN;
          Serial.println("\n>>> Target Edit Dipindah ke: [ SELATAN ] <<<");
          break;
        case TARGET_SELATAN:
          jalurDipilih = TARGET_TIMUR;
          Serial.println("\n>>> Target Edit Dipindah ke: [ TIMUR ] <<<");
          break;
        case TARGET_TIMUR:
          jalurDipilih = TARGET_UTARA;
          Serial.println("\n>>> Target Edit Dipindah ke: [ UTARA ] <<<");
          break;
      }
      inputBuffer = ""; // Reset ketikan angka saat ganti target
    }

    // 2. TOMBOL 'A' -> RESET SEMUA KE DEFAULT AWAL
    else if (key == 'A') {
      durasiUtara   = DEF_UTARA;
      durasiBarat   = DEF_BARAT;
      durasiSelatan = DEF_SELATAN;
      durasiTimur   = DEF_TIMUR;
      Serial.println("\n>>> RESET! Semua durasi kembali ke default (U:7, B:7, S:7, T:7) <<<");
      inputBuffer = "";
    }

    // 3. INPUT ANGKA (0-9) MAKSIMAL 2 DIGIT
    else if (key >= '0' && key <= '9') {
      if (inputBuffer.length() < 2) {
        inputBuffer += key;
        Serial.print("Input Angka Baru: ");
        Serial.println(inputBuffer);
      } else {
        Serial.println("[Peringatan] Maksimal input 2 digit!");
      }
    } 

    // 4. TOMBOL '*' -> BACKSPACE
    else if (key == '*') {
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        Serial.print("Hapus 1 karakter! Sisa input: ");
        Serial.println(inputBuffer.length() > 0 ? inputBuffer : "(kosong)");
      }
    } 

    // 5. TOMBOL '#' -> ENTER / KONFIRMASI PENYIMPANAN
    else if (key == '#') {
      if (inputBuffer.length() > 0) {
        int nilaiInput = inputBuffer.toInt();

        if (nilaiInput < 3) {
          Serial.println("[Gagal] Durasi minimal adalah 3 detik!");
        } else {
          // Menyimpan durasi ke jalur yang sedang aktif dipilih saat ini
          switch (jalurDipilih) {
            case TARGET_UTARA:
              durasiUtara = nilaiInput;
              Serial.print(">>> Sukses! Durasi UTARA diubah ke: ");
              break;
            case TARGET_BARAT:
              durasiBarat = nilaiInput;
              Serial.print(">>> Sukses! Durasi BARAT diubah ke: ");
              break;
            case TARGET_SELATAN:
              durasiSelatan = nilaiInput;
              Serial.print(">>> Sukses! Durasi SELATAN diubah ke: ");
              break;
            case TARGET_TIMUR:
              durasiTimur = nilaiInput;
              Serial.print(">>> Sukses! Durasi TIMUR diubah ke: ");
              break;
          }
          Serial.print(nilaiInput);
          Serial.println(" Detik <<<");
        }

        inputBuffer = "";
      } else {
        Serial.println("Input kosong! Masukkan angka terlebih dahulu.");
      }
    }
  }
}

void loop() {
  updateSiklusLaluLintas(); 
  bacaKeypad();             
}