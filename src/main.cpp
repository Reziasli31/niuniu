#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
WebServer server(80);

// Pin LED sesuai rangkaian Anda
const int U_RED = 21, U_YELLOW = 19, U_GREEN = 18;
const int B_RED = 5,  B_YELLOW = 17, B_GREEN = 16;
const int S_RED = 26, S_YELLOW = 25, S_GREEN = 33;
const int T_RED = 12, T_YELLOW = 14, T_GREEN = 27;

// Urutan dibalik: 0 = Utara, 1 = Barat, 2 = Selatan, 3 = Timur
const int urutanHijau[] = {U_GREEN, B_GREEN, S_GREEN, T_GREEN};
unsigned long durasiTotal[] = {7000, 7000, 7000, 7000};
const unsigned long DURASI_KUNING = 1000; // 1 detik kuning bagian dari total durasi

int indeksSekarang = 0;
int indeksTujuan = 1;
bool faseKuning = false;
unsigned long waktuSebelumnya = 0;

int getYellowPin(int h) {
  if (h == U_GREEN) return U_YELLOW;
  if (h == B_GREEN) return B_YELLOW;
  if (h == S_GREEN) return S_YELLOW;
  return T_YELLOW;
}

int getRedPin(int h) {
  if (h == U_GREEN) return U_RED;
  if (h == B_GREEN) return B_RED;
  if (h == S_GREEN) return S_RED;
  return T_RED;
}

void mulaiFaseHijau(int idx) {
  int h = urutanHijau[idx];
  digitalWrite(getRedPin(h), LOW);
  digitalWrite(getYellowPin(h), LOW);
  digitalWrite(h, HIGH);
}

// ================= HALAMAN UTAMA =================
void halamanUtama() {
  int hU = durasiTotal[0] / 1000;
  int hB = durasiTotal[1] / 1000;
  int hS = durasiTotal[2] / 1000;
  int hT = durasiTotal[3] / 1000;

  int mU = hB + hS + hT;
  int mB = hU + hS + hT;
  int mS = hU + hB + hT;
  int mT = hU + hB + hS;

  String namaJalur[] = {"Utara (U)", "Barat (B)", "Selatan (S)", "Timur (T)"};
  String statusAktif = namaJalur[indeksSekarang] + (faseKuning ? " -> Kuning ke " + namaJalur[indeksTujuan] : " (HIJAU)");

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kontrol & Durasi Lampu Lalu Lintas</title>
    <style>
        body { text-align: center; font-family: sans-serif; margin-top: 25px; background-color: #f4f4f9; padding: 0 15px 40px 15px; }
        table { margin: 0 auto 20px auto; border-collapse: collapse; width: 100%; max-width: 500px; background: #fff; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); }
        th, td { padding: 12px 15px; border: 1px solid #ddd; text-align: center; }
        th { background-color: #007BFF; color: white; }
        .box { background: #fff; max-width: 500px; margin: 0 auto 20px auto; padding: 20px; border-radius: 8px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); box-sizing: border-box; }
        .row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
        input { width: 80px; padding: 8px; font-size: 16px; border: 1px solid #ccc; border-radius: 5px; text-align: center; }
        .grid-arah { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; margin-bottom: 12px; }
        .btn { display: inline-block; width: 100%; padding: 12px; color: #fff; border: none; border-radius: 5px; font-size: 15px; font-weight: bold; cursor: pointer; margin-top: 6px; text-decoration: none; box-sizing: border-box; }
        .btn-blue { background: #007BFF; margin-top: 0; }
        .btn-blue:hover { background: #0056b3; }
        .btn-green { background: #28a745; }
        .btn-green:hover { background: #218838; }
        .btn-red { background: #dc3545; }
        .btn-red:hover { background: #c82333; }
        small { color: #666; display: block; margin-bottom: 15px; }
        .badge { display: inline-block; padding: 6px 14px; background: #e9ecef; border-radius: 20px; font-weight: bold; margin-bottom: 15px; color: #333; }
    </style>
</head>
<body>
    <h2>Monitoring & Pengaturan Durasi</h2>
    <div class="badge">Status: )rawliteral" + statusAktif + R"rawliteral(</div>
    
    <table>
    <tr><th>Arah Jalur</th><th>Durasi Giliran (+1s Kuning)</th><th>Lampu Merah</th></tr>
    <tr><td><b>Utara (U)</b></td><td>)rawliteral" + String(hU) + " Detik</td><td>" + String(mU) + R"rawliteral( Detik</td></tr>
    <tr><td><b>Timur (T)</b></td><td>)rawliteral" + String(hB) + " Detik</td><td>" + String(mB) + R"rawliteral( Detik</td></tr>
    <tr><td><b>Selatan (S)</b></td><td>)rawliteral" + String(hS) + " Detik</td><td>" + String(mS) + R"rawliteral( Detik</td></tr>
    <tr><td><b>Barat (B)</b></td><td>)rawliteral" + String(hT) + " Detik</td><td>" + String(mT) + R"rawliteral( Detik</td></tr>
</table>


    <div class="box">
        <h3>Lompat Paksa Lampu Hijau ke Arah:</h3>
        <div class="grid-arah">
            <a href="/lompat?arah=0" class="btn btn-blue">Utara (U)</a>
            <a href="/lompat?arah=1" class="btn btn-blue">Timur (T)</a>
            <a href="/lompat?arah=2" class="btn btn-blue">Selatan (S)</a>
            <a href="/lompat?arah=3" class="btn btn-blue">Barat (B)</a>
        </div>
        <a href="/reset" class="btn btn-red" onclick="return confirm('Reset semua durasi kembali ke 7, 7, 7, 7 detik?')">Reset ke Durasi Default (7, 7, 7, 7)</a>
    </div>

    <div class="box">
        <h3>Ubah Total Durasi Giliran (Detik)</h3>
        <small>*1 detik terakhir otomatis menjadi lampu kuning</small>
        <form action="/simpan" method="GET">
    <div class="row"><label><b>Utara (U):</b></label><input type="number" name="u" min="2" max="120" value=")rawliteral" + String(hU) + R"rawliteral(" required></div>
    <div class="row"><label><b>Timur (T):</b></label><input type="number" name="b" min="2" max="120" value=")rawliteral" + String(hB) + R"rawliteral(" required></div>
    <div class="row"><label><b>Selatan (S):</b></label><input type="number" name="s" min="2" max="120" value=")rawliteral" + String(hS) + R"rawliteral(" required></div>
    <div class="row"><label><b>Barat (B):</b></label><input type="number" name="t" min="2" max="120" value=")rawliteral" + String(hT) + R"rawliteral(" required></div>
    <button type="submit" class="btn btn-green">Simpan Perubahan</button>
</form>

    </div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleSimpan() {
  if (server.hasArg("u")) durasiTotal[0] = max(2L, server.arg("u").toInt()) * 1000UL;
  if (server.hasArg("b")) durasiTotal[1] = max(2L, server.arg("b").toInt()) * 1000UL;
  if (server.hasArg("s")) durasiTotal[2] = max(2L, server.arg("s").toInt()) * 1000UL;
  if (server.hasArg("t")) durasiTotal[3] = max(2L, server.arg("t").toInt()) * 1000UL;

  server.sendHeader("Location", "/");
  server.send(303);
}

// Fitur 1: Reset semua durasi ke default (U=7, B=7, S=7, T=7)
void handleReset() {
  durasiTotal[0] = 7000;
  durasiTotal[1] = 7000;
  durasiTotal[2] = 7000;
  durasiTotal[3] = 7000;

  server.sendHeader("Location", "/");
  server.send(303);
}

// Fitur 2: Lompat langsung ke arah spesifik (0=U, 1=B, 2=S, 3=T)
void handleLompat() {
  if (server.hasArg("arah")) {
    int target = server.arg("arah").toInt();
    if (target >= 0 && target < 4 && !faseKuning && target != indeksSekarang) {
      indeksTujuan = target;
      int hijauAktif = urutanHijau[indeksSekarang];
      int hijauBerikutnya = urutanHijau[indeksTujuan];

      digitalWrite(hijauAktif, LOW);
      digitalWrite(getYellowPin(hijauAktif), HIGH);
      digitalWrite(getRedPin(hijauBerikutnya), HIGH);
      digitalWrite(getYellowPin(hijauBerikutnya), HIGH);

      faseKuning = true;
      waktuSebelumnya = millis();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  pinMode(U_RED, OUTPUT); pinMode(U_YELLOW, OUTPUT); pinMode(U_GREEN, OUTPUT);
  pinMode(B_RED, OUTPUT); pinMode(B_YELLOW, OUTPUT); pinMode(B_GREEN, OUTPUT);
  pinMode(S_RED, OUTPUT); pinMode(S_YELLOW, OUTPUT); pinMode(S_GREEN, OUTPUT);
  pinMode(T_RED, OUTPUT); pinMode(T_YELLOW, OUTPUT); pinMode(T_GREEN, OUTPUT);

  digitalWrite(U_RED, HIGH);
  digitalWrite(B_RED, HIGH);
  digitalWrite(S_RED, HIGH);
  digitalWrite(T_RED, HIGH);

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  Serial.println("WiFi Terhubung! IP: " + WiFi.localIP().toString());

  server.on("/", halamanUtama);
  server.on("/simpan", handleSimpan);
  server.on("/reset", handleReset);
  server.on("/lompat", handleLompat);

  server.begin();
  mulaiFaseHijau(indeksSekarang);
  indeksTujuan = (indeksSekarang + 1) % 4;
  waktuSebelumnya = millis();
}

void loop() {
  server.handleClient();

  unsigned long waktuSekarang = millis();
  int hijauAktif = urutanHijau[indeksSekarang];
  unsigned long durasiHijauMurni = durasiTotal[indeksSekarang] - DURASI_KUNING;

  if (!faseKuning) {
    if (waktuSekarang - waktuSebelumnya >= durasiHijauMurni) {
      indeksTujuan = (indeksSekarang + 1) % 4;
      int hijauBerikutnya = urutanHijau[indeksTujuan];

      digitalWrite(hijauAktif, LOW);
      digitalWrite(getYellowPin(hijauAktif), HIGH);
      digitalWrite(getRedPin(hijauBerikutnya), HIGH);
      digitalWrite(getYellowPin(hijauBerikutnya), HIGH);

      faseKuning = true;
      waktuSebelumnya = waktuSekarang;
    }
  } else {
    if (waktuSekarang - waktuSebelumnya >= DURASI_KUNING) {
      int hijauBerikutnya = urutanHijau[indeksTujuan];

      digitalWrite(getYellowPin(hijauAktif), LOW);
      digitalWrite(getRedPin(hijauAktif), HIGH);
      digitalWrite(getYellowPin(hijauBerikutnya), LOW);

      faseKuning = false;
      indeksSekarang = indeksTujuan;
      mulaiFaseHijau(indeksSekarang);
      waktuSebelumnya = waktuSekarang;
    }
  }
}