#include <LiquidCrystal_I2C.h //Penambahan Library Liquid Crystal_I2C
LiquidCrystal_I2C lcd(0x27, 20, 4); //Setting jenis I2C (Angka 20, 4 berarti sekarang LCD yang digunakan memiliki 4 baris dan 20 kolom karakter)
#include <SPI.h> //Penambahan Library SPI untuk penghubungan ESP32 dengan Ethernet
#include <Ethernet.h> //Penambahan Library Ethernet untuk setting syntax penghubung ke ethernet

//Konektivitas Ethernet
char server[] = "192.168.13.2"; //IP address disesuaikan dengan target pengiriman (Di sini PT. Sinko memiliki IP 192.168.13.2)
IPAddress ip(192, 168, 13, 167); //IP address mikrokontroler pengirim ( 3 IP pertama harus sama dengan server[] namun IP keempat dirubah antara 1 - 255 namun tidak boleh sama dengan IP server[])
EthernetClient client; //Inisialisasi client Ethernet (Ini adalah syntax bawaan)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //Setting MAC Address ( Tidak perlu dirubah - rubah )


const int trigPin = 4; //Menginisiasi pin  mikrokontroler sebagai trigger sensor ultrasonik yang ada di loker
const int echoPin = 5; //Menginisiasi pin mikrokontroler sebagai echo (penerima/receiver) untuk sensor ultrasonik yang ada di loker
const int trigPin2 = 6; //Menginisiasi pin  mikrokontroler sebagai trigger sensor ultrasonik yang ada di area parkir
const int echoPin2 = 7; //Menginisiasi pin mikrokontroler sebagai echo (penerima/receiver) untuk sensor ultrasonik yang ada di area parkir
const int ledPin = 21; //Menginisiasi pin mikrokontroler sebagai pengirim sinyal kepada buzzer jika tandon E7 habis
const int ledPin2 = 26; //Menginisiasi pin mikrokontroler sebagai pengirim sinyal kepada buzzer jika tandon loker habis


#define SOUND_SPEED 0.034 //Pendefinisian kecepatan suara menjadi 0.034 cm/uS (Centimeter / Mikro Second)


long duration; //Pendefinisian durasi dalam bentuk format long sebagai regulasi sensor yang ada di tandon E7
long duration2; // Pendefinisian durasi dalam bentuk format long sebagai regulasi sensor yang ada di tandon loker
float distanceCm; // Pendefinisian jarak dalam bentuk format float sebagai penanda jarak antara sensor dengan air yang ada di tandon E7
float distanceCm2; // Pendefinisian jarak dalam bentuk format float sebagai penanda jarak antara sensor dengan air yang ada di tandon loker
float ketinggian1; // Pendefinisian ketinggian air yang ada di tandon E7
float ketinggian2; // Pendefinisian ketinggian air yang ada di tandon loker
unsigned long lastTimeConditionWasFalse; //Pendefinisian kondisi lama sebagai regulasi sensor yang ada di tandon E7
unsigned long lastTimeConditionWasFalse2; //Pendefinisian kondisi lama sebagai regulasi sensor yang ada di tandon loker

void setup() { //Fungsi yang hanya dilakukan sekali saja ( syntax dasar dari pemrograman arduino )
  //setup pin dan lcd
    Serial.begin(115200); //Menghubungkan LCD ke mikrokontroler ESP (Jika menggunakan esp)
  lcd.init();                      // initialize the lcd 
  lcd.backlight(); //Memberikan backlight pada LCD
  Ethernet.init(33); //Menginisialisasi pin esp32 yang akan digunakan sebagai pin CS pada modul ethernet W5500
  Ethernet.begin(mac, ip); //Menghubungkan mikrokontroler ke alamat ip dan mac yang sudah disetting
  
  pinMode(trigPin, OUTPUT); // Set trigPin sensor tandon E7 sebagai Output saja (Tidak bisa menerima input sinyal)
  pinMode(echoPin, INPUT); // Set echoPin sensor tandon E7 sebagai Input saja (Tidak bisa memberikan sinyal output)
  pinMode(trigPin2, OUTPUT); // Set trigPin sensor tandon loker sebagai Output saja (Tidak bisa menerima input sinyal)
  pinMode(echoPin2, INPUT); // Set echoPin sensor tandon loker sebagai Input saja (Tidak bisa memberikan sinyal output)
  
  pinMode(ledPin, OUTPUT); // Set ledPin sensor tandon E7 sebagai Input saja (Tidak bisa memberikan sinyal output)
  pinMode(ledPin2, OUTPUT); // Set ledPin sensor tandon loker sebagai Output saja (Tidak bisa menerima input sinyal)
}

void loop() { //Fungsi yang akan dilakukan terus menerus berulang - ulang
// Sensor 1 (Area tandon E7)
  digitalWrite(trigPin, LOW); //Mengatur pin pengirim sinyal ultrasonik ke low
  delayMicroseconds(2); //menunggu selama 2 mikrosecond
  digitalWrite(trigPin, HIGH); // Mengubah kondisi pin pengiriman sinyal ke high
  delayMicroseconds(20); //Kondisi ini terjadi selama 20 microseconds
  digitalWrite(trigPin, LOW); //Mengatur pin pengirim sinyal ultrasonik ke low
  duration = pulseIn(echoPin, HIGH); //Menyalakan pin penerima sinyal untuk menangkap lamanya sinyal terkirim, sehingga dapat dihitung sebagai jarak
  
  delay(1000);
  
//Sensor 2 (Area tandon Loker)
  digitalWrite(trigPin2, LOW); //Mengatur pin pengirim sinyal ultrasonik ke 
  delayMicroseconds(2); //menunggu selama 2 mikrosecond
  digitalWrite(trigPin2, HIGH); // Mengubah kondisi pin pengiriman sinyal ke high
  delayMicroseconds(20); //Kondisi ini terjadi selama 20 microseconds
  digitalWrite(trigPin2, LOW); //Mengatur pin pengirim sinyal ultrasonik ke low
  duration2 = pulseIn(echoPin2, HIGH); //Menyalakan pin penerima sinyal untuk menangkap lamanya sinyal terkirim, sehingga dapat dihitung sebagai jarak

  
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2; //Menghitung jarak antara sensor ultrasonik dengan air pada tandon E7
  distanceCm2 = duration2 * SOUND_SPEED/2; //Menghitung jarak antara sensor ultrasonik dengan air pada tandon loker
  ketinggian1 = 120 - distanceCm; //Menghitung ketinggian air pada tandon E7
  ketinggian2 = 120 - distanceCm2; //Menghitung ketinggian air pada tandon loker

  Serial.print("Loker = "); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino
  Serial.print(ketinggian1); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino
  Serial.println("CM   "); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino
  Serial.print("Luar = "); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino
  Serial.print(ketinggian2); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino
  Serial.println("CM   "); // Karakter untuk melihat hasil pembacaan sensor pada serial monitor aplikasi arduino

lcd.setCursor(0,0); //Memposisikan karakter ditulis dari baris pertama dan kolom pertama
lcd.print ("Tandon E7 = "); //Tempat menuliskan karakter
lcd.print(ketinggian1); //Karakter berisi ketinggian air yang berada pada tandon E7
lcd.print(" CM    "); //Tempat menuliskan karakter

lcd.setCursor(0,1); //Memposisikan karakter ditulis dari baris kedua dan kolom pertama
lcd.print ("Loker     = "); //Tempat menuliskan karakter
lcd.print(ketinggian2); //Karakter berisi ketinggian air yang berada pada tandon loker
lcd.print(" CM    "); //Tempat menuliskan karakter

//Pembacaan 1
  if (ketinggian1 > 40.00) { //Membuat kondisi jika ketinggian air yang berada di tandon E7 lebih dari 40.00 cm
   lastTimeConditionWasFalse = millis(); //Maka timer kondisi 'air masih banyak' akan menyala
}

if (millis() - lastTimeConditionWasFalse >= 5000) { //jika timer kondisi 'air masih banyak' tidak menyala selama 5 detik (Seinyal menangkap bahwa air sudah pada titik rendah selama lebih dari 5 detik)
  digitalWrite(ledPin, HIGH); //ledPin akan menyala dan buzzer akan berbunyi
  lcd.setCursor(0,2); //Memposisikan karakter ditulis dari baris ketiga dan kolom pertama
  lcd.print("Tandon E7 Habis !"); //Tempat menuliskan karakter
 }
 else //jika air sudah diisi melebihi ketinggian 40.00 cm
 {
  digitalWrite(ledPin, LOW); //Sinyal pengirim ke buzzer akan berhenti berbunyi
   lcd.setCursor(0,2); //Memposisikan karakter ditulis dari baris ketiga dan kolom pertama
  lcd.print("                    "); //Tempat menuliskan karakter
 }


//Pembacaan 2
if ( ketinggian2 < 40.00) { //Membuat kondisi jika ketinggian air yang berada di tandon loker lebih dari 40.00 cm
   lastTimeConditionWasFalse2 = millis(); //Maka timer kondisi 'air masih banyak' akan menyala
}

if (millis() - lastTimeConditionWasFalse2 >= 5000) { //jika timer kondisi 'air masih banyak' tidak menyala selama 5 detik (Seinyal menangkap bahwa air sudah pada titik rendah selama lebih dari 5 detik)
  digitalWrite(ledPin2, HIGH); //ledPin akan menyala dan buzzer akan berbunyi
  lcd.setCursor(0,3); //Memposisikan karakter ditulis dari baris keempat dan kolom pertama
  lcd.print("Tandon Loker Habis !"); //Tempat menuliskan karakter
 }
 else //jika air sudah diisi melebihi ketinggian 40.00 cm
 {
  digitalWrite(ledPin2, LOW); //Sinyal pengirim ke buzzer akan berhenti berbunyi
  lcd.setCursor(0,3); //Memposisikan karakter ditulis dari baris ketiga dan kolom pertama
 lcd.print("                    "); //Tempat menuliskan karakter
 }
 Sending_To_phpmyadmindatabase(); //Menjalankan fungsi pengiriman ke database
 delay(1000); // delay 1 detik sebelum pengulangan dilakukan kembali
}

void Sending_To_phpmyadmindatabase()   //Fungsi Pengiriman ke database
{ //                                                                                                                         alamat IP    port
  if (client.connect(server, 80)) { //Koneksi mikrokontroler dengan server yang telah diinisialisasi beserta portnya               V       v
                                    //Dalam hal ini PT. Sinko menggunakan port 80 sehingga penyebutannya menjadi 'client connect (server, 80)      '
    Serial.println("sended"); //print terhadap serial monitor bahwa data berhasil terhubung
    //GET from php files localy
    // Make a HTTP request:
    Serial.print("GET /water/level.php?loker="); // Membangkitkan URL yang ditarget dan memasukkan parameter ketinggian air tandon loker dalam URL
    client.print("GET /water/level.php?loker="); // Membangkitkan URL yang ditarget dan memasukkan parameter ketinggian air tandon loker dalam URL
    Serial.println(ketinggian2); //Memasukkan nilai ketinggian air yang ada di tandon area loker
    client.print(ketinggian2); //Memasukkan nilai ketinggian air yang ada di tandon area loker
    client.print("&parkir="); //Menambahkan parameter ketinggian air tandon E7
    Serial.println("&parkir="); //Menambahkan parameter ketinggian air tandon E7
    client.print(ketinggian1); //Memasukkan nilai ketinggian air yang ada di tandon E7
    Serial.println(ketinggian1); //Memasukkan nilai ketinggian air yang ada di tandon E7
    client.print(" ");      //SPACE BEFORE HTTP/1.1
     client.print("HTTP/1.1"); //Memberikan tipe HTTP yang digunakan (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println(); //membuat line baru untuk inisialisasi cient (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println("Host: 192.168.13.2"); //IP host yang digunakan (Gunakan ip address yang sudah dibuat sebelumnya) (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println("Connection: close"); //menutup konektivitas dengan client (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println(); //Memberikan spasi jeda untuk pengiriman selanjutyan (Prosedur bawaan koneksi mikrokontroler ke website)
  } else { // Jika tidak mendapatkan koneksi ke server
   
    Serial.println("connection failed"); //Mengirimkan tulisan koneksi gagal ke serial monitor
    Ethernet.begin(mac, ip); //Jika tidak dapat terhubung ke server maka koneksi dilakukan sekali lagi 
  }
}
