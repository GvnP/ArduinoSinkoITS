s#include <LiquidCrystal_I2C.h> //Penambahan Library Liquid Crystal_I2C
LiquidCrystal_I2C lcd(0x27, 20, 4); //Setting jenis I2C (Angka 20, 4 berarti sekarang LCD yang digunakan memiliki 4 baris dan 20 kolom karakter)
#include <SPI.h> //Penambahan Library SPI untuk penghubungan ESP32 dengan Ethernet
#include <Ethernet.h> //Penambahan Library Ethernet untuk setting syntax penghubung ke ethernet

//Variabel untuk menentukan apakah mikro menyala pertama kali atau tidak
int set = 0; //Variabel ini untuk memulai kondisi mikro yang baru menyala (Kondisi 0)
int MikroStatus; //Variabel untuk menentukan status mikrokontroler

//Konektivitas Ethernet
char server[] = "192.168.13.2"; //IP address disesuaikan dengan target pengiriman (Di sini PT. Sinko memiliki IP 192.168.13.2)
IPAddress ip(192, 168, 13, 33); //IP address mikrokontroler pengirim ( 3 IP pertama harus sama dengan server[] namun IP keempat dirubah antara 1 - 255 namun tidak boleh sama dengan IP server[])
EthernetClient client; //Inisialisasi client Ethernet (Ini adalah syntax bawaan)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //Setting MAC Address ( Tidak perlu dirubah - rubah )

// Variabel penghitungan debit masuk ke dalam tandon
int X; //Inisialisasi sinyal high pada hall sensor debit air untuk air yang masuk ke dalam tandon
int Y; //Inisialisasi sinyal low pada hall sensor debit air untuk air yang masuk ke dalam tandon

float TIME = 0; //inisialisasi waktu lamanya kondisi high (X) berpindah ke low (Y) ataupun sebaliknya
float FREQUENCY = 0;  //Inisialisasi frekuensi untuk air masuk tandon sebagai rumus penghitungan debit
float WATER = 0;  //Inisialisasi debit awal untuk air masuk tandon sebagai rumus penghitungan debit
float TOTAL;  //Inisialisasi volume awal untuk air masuk tandon sebagai rumus penghitungan volume total air yang telah lewat
float LS = 0; //Inisialisasi debit yang lewat per detik untuk rumus volume total air yang telah lewat
const int input = 7; //Menentukan pin mikrokontroler yang digunakan untuk dihubungkan ke kabel sinyal sensor debit (Pastikan pin bertipe adc dengan melihat dokumentasi mikrokontroler)


//penghitungan Kedua
int X2; //Inisialisasi sinyal high pada hall sensor debit air untuk air yang keluar dari dalam tandon
int Y2; //Inisialisasi sinyal low pada hall sensor debit air untuk air yang keluar dari dalam tandon

float TIME2 = 0; //inisialisasi waktu lamanya kondisi high (X) berpindah ke low (Y) ataupun sebaliknya
float FREQUENCY2 = 0; //Inisialisasi frekuensi untuk air masuk tandon sebagai rumus penghitungan debit
float WATER2 = 0; //Inisialisasi debit awal untuk air keluar dari tandon sebagai rumus penghitungan debit
float TOTAL2; //Inisialisasi volume awal untuk air keluar dari tandon sebagai rumus penghitungan volume total air yang telah lewat
float LS2 = 0; //Inisialisasi debit yang lewat per detik untuk rumus volume total air yang telah lewat
const int input2 = 10; //Menentukan pin mikrokontroler yang digunakan untuk dihubungkan ke kabel sinyal sensor debit (Pastikan pin bertipe adc dengan melihat dokumentasi mikrokontroler)


void setup() { //Fungsi yang hanya dilakukan sekali saja ( syntax dasar dari pemrograman arduino )
  lcd.begin();  //Menghubungkan LCD ke mikrokontroler ESP (Jika menggunakan esp)
  //lcd.init();   //Menghubungkan LCD ke mikrokontroler arduino (Jika menggunakan arduino)

  lcd.clear();    //Membersihkan karakter pada LCD
  lcd.setCursor(0, 0); //Memposisikan karakter pertama ditulis dari baris pertama dan kolom pertama
  lcd.print("Water Flow Meter"); //Tempat menuliskan karakter
  lcd.setCursor(0, 1); //Memposisikan karakter pertama ditulis dari baris kedua dan kolom pertama
  lcd.print("****************"); //Tempat menuliskan karakter

  pinMode(input, INPUT); //Menginisialisasi pin input agar hanya bisa menerima input saja dan tidak bisa mengeluarkan sinyal
  pinMode(input2, INPUT); //Menginisialisasi pin input2 agar hanya bisa menerima input saja dan tidak bisa mengeluarkan sinyal

  Ethernet.init(33); //Menginisialisasi pin esp32 yang akan digunakan sebagai pin CS pada modul ethernet W5500
  Serial.begin(115200); //Setting baud rate pada angka 115200
  Ethernet.begin(mac, ip); //Menghubungkan mikrokontroler ke alamat ip dan mac yang sudah disetting

  delay(3000); //Menunggu 3 detik sebelum masuk ke fungsi selanjutnya
}


/* Infinite Loop */
void loop() { //Fungsi yang akan dilakukan terus menerus berulang - ulang

  if (set == 0) //Memeriksa jika variabel set masih pada angka 0
  {
    set = 1;          //Mengubah variabel set menjadi 1 jika kondisi terpenuhi
    MikroStatus = 0;  //Mengubah kondisi MikroStatus menjadi 0 (Kondisi dimana mikrokontroler baru dinyalakan)
  }
  else { //Memeriksa jika kondisi if tidak terpenuhi
    MikroStatus = 1; //Mengubah kondisi MikroStatus menjadi 1 ( Kondisi dimana mikrokontroler sudah dinyalakan selama satu kali sequence program )
  }

  //Penghitung Debit Air
  X = pulseIn(input, HIGH); //Menghitung berapa lama waktu yang dibutuhkan hall sensor pendeteksi air masuk tandon dari kondisi awal (low) ke kondisi high
  Y = pulseIn(input, LOW); //Menghitung berapa lama waktu yang dibutuhkan hall sensor pendeteksi air masuk tandon dari kondisi high (setelah inisialisasi X di atas) ke kondisi low
  TIME = X + Y; //Menghitung total waktu yang dibutuhkan untuk perpindahan kondisi hall sensor pendeteksi air masuk tandon
  FREQUENCY = 1000000 / TIME; //Penghitungan frekuensi untuk menentukan berapa lama hall sensor berputar (100000 microsecond = 1 second) dengan menggunakan rumus frekuensi (f = 1/T)
  WATER = FREQUENCY / 4.8; //Kalibrasi sensor penghitungan debit dengan frekuensi (sensor F400A memiliki karakteristik aliran pulsa dengan rumus debit = F / 4.8)
  LS = WATER / 60; //Penghitungan debit air yang lewat tiap detik
  
  // PENGHITUNGAN DEBIT MASUK
  if (FREQUENCY >= 0) //Memulai kondisi sensor dialiri oleh air (Jika frekuensi lebih dari dan sama dengan 0)
  {
    if (isinf(FREQUENCY)) //Jika frequency tidak terdefinisikan (Ketika sensor tidak dialiri air yang masuk ke tandon)
    {
      lcd.clear(); //Membersihkan LCD
      lcd.home(); //Melakukan setting agar karakter dimulai dari bagian pojok kiri atas LCD
      lcd.setCursor(0, 0); //Memposisikan karakter pertama ditulis dari baris pertama dan kolom pertama
      lcd.print("DEBIT IN : 0.00 L/M"); //Tempat menuliskan karakter (Ditulis 0.00 karena tidak ada air yang lewat)
      lcd.setCursor(0, 1); //Memposisikan karakter pertama ditulis dari baris kedua dan kolom pertama
      lcd.print("TOTAL IN : "); //Tempat menuliskan karakter
      lcd.print(TOTAL); //Karakter berisi volume total yang telah lewat
      lcd.print(" L  "); //Tempat menuliskan karakter
    }
    else //Jika kondisi tidak terpenuhi (Terdapat air yang masuk ke dalam tandon dan melewati sensor)
    {
       if (LS > 1000){
        LS = 0;}
      TOTAL = TOTAL + LS; //Menambahkan air yang lewat tiap detik untuk diakumulasi menjadi volume total air yang telah lewat
      lcd.setCursor(0, 0); //Memposisikan karakter ditulis dari baris pertama dan kolom pertama
      lcd.print("DEBIT IN : "); //Tempat menuliskan karakter
      lcd.print(WATER); //Karakter berisi debit yang lewat
      lcd.print(" L/M"); //Tempat menuliskan karakter
      lcd.setCursor(0, 1); //Memposisikan karakter ditulis dari baris kedua dan kolom pertama
      lcd.print("TOTAL IN : "); //Tempat menuliskan karakter
      lcd.print(TOTAL); //karakter berisi total volume yang telah melewati sensor
      lcd.print(" L"); //Tempat menuliskan karakter
          }
  }

  X2 = pulseIn(input2, HIGH); //Menghitung berapa lama waktu yang dibutuhkan sensor pendeteksi air keluar dari tandon dari kondisi awal (low) ke kondisi high
  Y2 = pulseIn(input2, LOW); //Menghitung berapa lama waktu yang dibutuhkan hall sensor pendeteksi air keluar dari tandon dari kondisi high (setelah inisialisasi X di atas) ke kondisi low
  TIME2 = X2 + Y2; //Menghitung total waktu yang dibutuhkan untuk perpindahan kondisi hall sensor pendeteksi air keluar dari tandon
  FREQUENCY2 = 1000000 / TIME2; //Penghitungan frekuensi untuk menentukan berapa lama hall sensor berputar (100000 microsecond = 1 second) dengan menggunakan rumus frekuensi (f = 1/T)
  WATER2 = FREQUENCY2 / 4.8; //Kalibrasi sensor penghitungan debit dengan frekuensi (sensor F400A memiliki karakteristik aliran pulsa dengan rumus debit = F / 4.8)
  LS2 = WATER2 / 60; //Penghitungan debit air yang lewat tiap detik

  //PENGHITUNGAN DEBIT KELUAR
  if (FREQUENCY2 >= 0) //Memulai kondisi sensor dialiri oleh air yang keluar dari dalam tandon (Jika frekuensi lebih dari dan sama dengan 0)
  {
    if (isinf(FREQUENCY2)) //Jika frequency tidak terdefinisikan (Ketika sensor tidak dialiri air yang keluar dari dalam tandon)
    {
      lcd.setCursor(0, 2); //Memposisikan karakter untuk ditulis dari baris ketiga kolom pertama
      lcd.print("DEBIT OUT: 0.00 L/M "); //Tempat Menuliskan Karakter (Ditulis 0.00 karena sensor sedang tidak dialiri oleh air
      lcd.setCursor(0, 3); //Memposisikan karakter untuk ditulis dari baris ketiga kolom pertama
      lcd.print("TOTAL OUT: "); //Tempat menuliskan karakter
      lcd.print(TOTAL2); //Karakter berisi total volume air yang telah melewati sensor
      lcd.print(" L"); //Tempat menuliskan karakter
    }
    else //Jika kondisi tidak terpenuhi (Terdapat air yang keluar dari dalamtandon dan melewati sensor)
    {
       if (LS2 > 1000){
        LS2 = 0;}
        
      TOTAL2 = TOTAL2 + LS2; //Menambahkan air yang lewat tiap detik untuk diakumulasi menjadi volume total air yang telah lewat

      if (isinf(WATER2)) { //Kondisi jika tidak ada air yang mengalir keluar dari tandon dan melewati sensor
        lcd.setCursor(0, 2); //Memposisikan karakter untuk ditulis dari baris ketiga kolom pertama
        lcd.print("DEBIT OUT: 0.00"); //Tempat Menuliskan Karakter (Ditulis 0.00 karena sensor sedang tidak dialiri oleh air
        lcd.print(" L/M"); //Tempat menuliskan karakter
        lcd.setCursor(0, 3); //Memposisikan karakter untuk ditulis dari baris keempat kolom pertama
        lcd.print("TOTAL OUT: "); //Tempat menuliskan karakter
        lcd.print(TOTAL2); //Karakter berisi total volume air yang telah melewati sensor
        lcd.print(" L"); //Tempat menuliskan karakter
      }
      else { //
        lcd.setCursor(0, 2); //Memposisikan karakter untuk ditulis dari baris ketiga kolom pertama
        lcd.print("DEBIT OUT: "); //Tempat Menuliskan Karakter
        lcd.print(WATER2); //Karakter berisi debit air yang telah melewati sensor
        lcd.print(" L/M"); //Tempat Menuliskan Karakter
        lcd.setCursor(0, 3); //Memposisikan karakter untuk ditulis dari baris keempat kolom pertama
        lcd.print("TOTAL OUT: "); //Tempat menuliskan karakter
        lcd.print(TOTAL2); //Karakter berisi total volume air yang telah melewati sensor
        lcd.print(" L"); //Tempat menuliskan karakter
      }
    }
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
         Serial.print("GET /water/dht.php?debit=");
        client.print("GET /water/dht.php?debit="); 
//    Serial.print("GET /adminstrap/dht.php?debit="); // Membangkitkan URL yang ditarget dan Memasukkan parameter debit ke dalam URL
//    client.print("GET /adminstrap/dht.php?debit="); // Membangkitkan URL yang ditarget dan Memasukkan parameter debit ke dalam URL
    Serial.print(WATER); //Memasukkan nilai debit air masuk ke dalam tandon yang telah didapatkan oleh sensor dan mikrokontroler
    client.print(WATER); //Memasukkan nilai debit air masuk ke dalam tandon yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&volume="); //Memasukkan parameter volume air yang masuk ke dalam tandon ke dalam URL
    Serial.print("&volume="); //Memasukkan parameter volume air yang masuk ke dalam tandon ke dalam URL
    client.print(TOTAL); //Memasukkan nilai volume total air masuk tandon yang telah didapatkan oleh sensor dan mikrokontroler
    Serial.print(TOTAL); //Memasukkan nilai volume total air masuk tandon yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&debit2=");  //Memasukkan parameter debit air yang keluar dari dalam tandon ke dalam URL
    Serial.print("&debit2="); //Memasukkan parameter debit air yang keluar dari dalam tandon ke dalam URL
    client.print(WATER2); //Memasukkan nilai debit air keluar dari tandon yang telah didapatkan oleh sensor dan mikrokontroler
    Serial.print(WATER2); //Memasukkan nilai debit air keluar dari tandon yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&volume2="); //Memasukkan parameter volume total air yang keluar dari dalam tandon ke dalam URL
    Serial.print("&volume2="); //Memasukkan parameter volume total air yang keluar dari dalam tandon ke dalam URL 
    client.print(TOTAL2); //Memasukkan nilai volume total air keluar dari tandon yang telah didapatkan oleh sensor dan mikrokontroler
    Serial.print(TOTAL2); //Memasukkan nilai volume total air keluar dari tandon yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&mikro="); //Memasukkan kondisi mikrokontroler (apakah beru menyala atau tidak) ke dalam URL
    Serial.print("&mikro="); //Memasukkan kondisi mikrokontroler (apakah beru menyala atau tidak) ke dalam URL
    client.print(MikroStatus); //Memasukkan nilai kondisi mikrokontroler apakah baru menyala atau tidak
    Serial.print(MikroStatus); //Memasukkan nilai kondisi mikrokontroler apakah baru menyala atau tidak
    client.print(" ");      //memberikan SPACE sebelum HTTP/1.1 (Prosedur bawaan koneksi mikrokontroler ke website)
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
