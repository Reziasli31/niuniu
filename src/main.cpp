#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// Pin LED
const int U_RED = 21, U_YELLOW = 19, U_GREEN = 18;
const int B_RED = 5,  B_YELLOW = 17, B_GREEN = 16;
const int S_RED = 26, S_YELLOW = 25, S_GREEN = 33;
const int T_RED = 12, T_YELLOW = 14, T_GREEN = 27;

// Urutan: 0 = Utara, 1 = Timur, 2 = Selatan, 3 = Barat
const int urutanHijau[] = {U_GREEN, T_GREEN, S_GREEN, B_GREEN};
unsigned long durasiHijau[] = {5000, 10000, 15000, 5000};

int indeksSekarang = 0;
bool faseKuning = false;
unsigned long waktuSebelumnya = 0;

// ================= HALAMAN UTAMA (KEYPAD & MONITOR) =================
const char HTML_KEYPAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Kontrol Lampu Lalu Lintas</title>
  <style>
    body { font-family: Arial, sans-serif; background: #1e293b; color: #f8fafc; text-align: center; padding: 20px; }
    .card { background: #334155; max-width: 400px; margin: auto; padding: 20px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); }
    .btn { display: inline-block; width: 100%; padding: 14px; margin-top: 12px; font-size: 16px; font-weight: bold; border: none; border-radius: 8px; cursor: pointer; text-decoration: none; color: white; box-sizing: border-box; }
    .btn-blue { background: #3b82f6; }
    .btn-green { background: #10b981; }
    .status { font-size: 20px; margin: 15px 0; padding: 12px; background: #0f172a; border-radius: 8px; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Kontrol Simpang 4</h2>
    <div class="status" id="info">Memuat status...</div>
    <a href="/durasi" class="btn btn-blue">Atur Durasi Lampu</a>
  </div>
  <script>
    setInterval(() => {
      fetch('/status').then(r => r.json()).then(d => {
        document.getElementById('info').innerHTML = 
          "Jalur Hijau: <b>" + d.jalur + "</b><br>Sisa Waktu: <b>" + d.sisa + " detik</b>";
      });
    }, 500);
  </script>
</body>
</html>
)rawliteral";

// ================= HALAMAN PENGATURAN DURASI =================
const char HTML_DURASI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Atur Durasi Lampu</title>
  <style>
    body { font-family: Arial, sans-serif; background: #1e293b; color: #f8fafc; text-align: center; padding: 20px; }
    .card { background: #334155; max-width: 400px; margin: auto; padding: 20px; border-radius: 12px; }
    .row { display: flex; justify-content: space-between; align-items: center; margin: 12px 0; }
    input { width: 80px; padding: 8px; font-size: 16px; border-radius: 6px; border: none; text-align: center; }
    button { width: 100%; padding: 14px; margin-top: 15px; font-size: 16px; font-weight: bold; border: none; border-radius: 8px; cursor: pointer; background: #10b981; color: white; }
    a { color: #93c5fd; display: inline-block; margin-top: 15px; text-decoration: none; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Atur Durasi Hijau (Detik)</h2>
    <form action="/simpan" method="GET">
      <div class="row"><label>Utara (U):</label><input type="number" name="u" min="1" max="120" value="5" required></div>
      <div class="row"><label>Timur (T):</label><input type="number" name="t" min="1" max="120" value="10" required></div>
      <div class="row"><label>Selatan (S):</label><input type="number" name="s" min="1" max="120" value="15" required></div>
      <div class="row"><label>Barat (B):</label><input type="number" name="b" min="1" max="120" value="5" required></div>
      <button type="submit">Simpan Durasi</button>
    </form>
    <a href="/">&larr; Kembali ke Menu Utama</a>
  </div>
</body>
</html>
)rawliteral";

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

void handleStatus() {
  String namaJalur[] = {"UTARA", "TIMUR", "SELATAN", "BARAT"};
  unsigned long totalFase = faseKuning ? 2000 : durasiHijau[indeksSekarang];
  unsigned long berlalu = millis() - waktuSebelumnya;
  long sisaDetik = (berlalu >= totalFase) ? 0 : (totalFase - berlalu + 999) / 1000;

  String json = "{\"jalur\":\"" + namaJalur[indeksSekarang] + (faseKuning ? " (KUNING)" : "") + "\",\"sisa\":" + String(sisaDetik) + "}";
  server.send(200, "application/json", json);
}

void handleSimpan() {
  if (server.hasArg("u")) durasiHijau[0] = server.arg("u").toInt() * 1000UL;
  if (server.hasArg("t")) durasiHijau[1] = server.arg("t").toInt() * 1000UL;
  if (server.hasArg("s")) durasiHijau[2] = server.arg("s").toInt() * 1000UL;
  if (server.hasArg("b")) durasiHijau[3] = server.arg("b").toInt() * 1000UL;

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
  Serial.print("Menyambung WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung! IP: " + WiFi.localIP().toString());

  server.on("/", []() { server.send(200, "text/html", HTML_KEYPAD); });
  server.on("/durasi", []() { server.send(200, "text/html", HTML_DURASI); });
  server.on("/status", handleStatus);
  server.on("/simpan", handleSimpan);

  server.begin();
  Serial.println("Web Server Aktif!");

  mulaiFaseHijau(indeksSekarang);
  waktuSebelumnya = millis();
}

void loop() {
  server.handleClient();

  unsigned long waktuSekarang = millis();
  int hijauAktif = urutanHijau[indeksSekarang];
  int indeksBerikutnya = (indeksSekarang + 1) % 4;
  int hijauBerikutnya = urutanHijau[indeksBerikutnya];

  if (!faseKuning) {
    if (waktuSekarang - waktuSebelumnya >= durasiHijau[indeksSekarang]) {
      digitalWrite(hijauAktif, LOW);
      digitalWrite(getYellowPin(hijauAktif), HIGH);
      digitalWrite(getRedPin(hijauBerikutnya), HIGH);
      digitalWrite(getYellowPin(hijauBerikutnya), HIGH);
      faseKuning = true;
      waktuSebelumnya = waktuSekarang;
    }
  } else {
    if (waktuSekarang - waktuSebelumnya >= 2000) {
      digitalWrite(getYellowPin(hijauAktif), LOW);
      digitalWrite(getRedPin(hijauAktif), HIGH);
      digitalWrite(getYellowPin(hijauBerikutnya), LOW);
      faseKuning = false;
      indeksSekarang = indeksBerikutnya;
      mulaiFaseHijau(indeksSekarang);
      waktuSebelumnya = waktuSekarang;
    }
  }
}
