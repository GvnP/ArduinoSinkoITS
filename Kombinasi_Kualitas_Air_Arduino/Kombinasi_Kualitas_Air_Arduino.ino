//Peletakan Kabel Data Sensor
// TDS di A3
// TSS di A0
// pH di A2


#include "GravityTDS.h" //Penambahan library untuk sensor gravity
#include <Wire.h> //Library untuk berkomunikasi dengan I2C pada LCD
#include <LiquidCrystal_I2C.h> //Library untuk menjalankan syntax LCD I2C
LiquidCrystal_I2C lcd(0x27, 20, 4); //Setting jenis I2C (Angka 20, 4 berarti sekarang LCD yang digunakan memiliki 4 baris dan 20 kolom karakter)
#include <SPI.h> //Penambahan Library SPI untuk penghubungan ESP32 dengan Ethernet
#include <Ethernet.h> //Penambahan Library Ethernet untuk setting syntax penghubung ke ethernet

//Variabel untuk menentukan apakah mikro menyala pertama kali atau tidak
int set = 0; //Variabel ini untuk memulai kondisi mikro yang baru menyala (Kondisi 0)
int MikroStatus; //Variabel untuk menentukan status mikrokontroler

//Setup Ethernet

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //Setting MAC Address
char server[] = "192.168.13.2"; //IP address disesuaikan dengan target pengiriman (Di sini PT. Sinko memiliki IP 192.168.13.2)
IPAddress ip(192, 168, 13, 88); //IP address mikrokontroler pengirim ( 3 IP pertama harus sama dengan server[] namun IP keempat dirubah antara 1 - 255 namun tidak boleh sama dengan IP server[])
EthernetClient client; //Inisialisasi client Ethernet (Ini adalah syntax bawaan)

// Setup TDS Sensor

#define TdsSensorPin A3 // Inisialisasi pin pada mikrokontroler sebagai penerima sinyal dari sensor TDS
GravityTDS gravityTds; // Pemrograman untuk menginisialisasi library sensor TDS

//Setup TSS Sensor
int pinTurb = A0; // Inisialisasi pin pada mikrokontroler sebagai penerima sinyal dari sensor TSS
float V; // Inisialisasi variabel voltase untuk konversi tegangan ke kekeruhan
float kekeruhan; // Inisialisasi variabel untuk penghitungan kekeruhan
float VRata2; // Inisialisasi Tegangan rata rata sebagai rumus penghitungan tegangan
float VHasil; // Inisialisasi tegangan hasil penghitungan
float NTU; // Inisialisasi NTU sebagai hasil dari rumus tegangan yang dikonversi sebagai kekeruhan
float pin = analogRead(pinTurb); // Inisiasi variabel pembacaan analog pin A1 mikrokontroler

//Setup pH Sensor
int pinPH = A2; // Inisialisasi pin A2 pada mikrokontroler sebagai penerima sinyal dari sensor pH
float calibration_value = 20.68; //Inisialisasi nilai kalibrasi pada pH
int phval = 0; // Inisiasi awal untuk nilai pH dibuat sama dengan 0
unsigned long int avgval; //Inisiasi variabel nilai rata - rata pH
int buffer_arr[10],temp; //Inisiasi array penghitungan rata - rata pH untuk mendapatkan nilai yang lebih stabil
float ph_act; //Inisiasi nilai akhir pH
float temperature = 25,tdsValue = 0; //Inisiasi temperatur dideklarasikan sebagai 25 derajat celcius

void setup() //Fungsi yang hanya dilakukan sekali saja ( syntax dasar dari pemrograman arduino )
{
  //Setup Ethernet
   Ethernet.init(10); //Menginisialisasi pin esp32 yang akan digunakan sebagai pin CS pada modul ethernet W5500
   Ethernet.begin(mac, ip); //Menghubungkan mikrokontroler ke alamat ip dan mac yang sudah disetting
    lcd.init();
  //Setup TDS
    Serial.begin(115200); //Setting baud rate pada angka 9600
    gravityTds.setPin(TdsSensorPin); 
    gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization
    
  //Setup pH
  lcd.clear();    //Membersihkan karakter pada LCD
  lcd.setCursor(0, 0); //Memposisikan karakter pertama ditulis dari baris pertama dan kolom pertama
  lcd.print("Water Quality Meter"); //Tempat menuliskan karakter
  lcd.setCursor(0, 1); //Memposisikan karakter pertama ditulis dari baris kedua dan kolom pertama
  lcd.print("****************"); //Tempat menuliskan karakter
  lcd.backlight(); //Memberikan backlight pada LCD
  delay(3000); //Memberikan jeda waktu sebesar 3 detik
}

void loop() //Fungsi yang akan dilakukan terus menerus berulang - ulang
{

  //Sensor TDS
    //temperature = readTemperature();  //Jika ada sensor temperatur, bisa dimasukkan di sini
    gravityTds.setTemperature(temperature);  // Eksekusi kompensasi temperatur yang telah diinisialisasi sebelumnya
    gravityTds.update();  //Kalkulasi tds berdasarkan sinyal yang diterima dari sensor
    tdsValue = gravityTds.getTdsValue();  // Mendapatkan nilai pembacaan sensor tds


    
  //Sensor TSS
   V = 0; //Inisiasi awal tegangan nol
  for(int i=0; i<800; i++) //Pengambilan 800 data tegangan yang diterima
  {
    V += ((float)analogRead(pinTurb)/1023)*5.0; //Mendapatkan masing - masing nilai tegangan yang diterima dari data TSS yang diambil
  }

  VRata2 = (V)/800; //Menghitung rata rata nilai tegangan yang diterima dengan membagi 800 data
  VHasil = roundf(VRata2*10.0f)/10.0f; //Membulatkan nilai rata - rata yang ada agar hanya ada satu angka di belakang koma

  if(VHasil < 2.5) //Membuat kondisi jika nilai tegangan analog di bawah 2.5 V
  {
    kekeruhan = 3000; //Langsung menulis bahwa kekeruhan adalah 3000, di mana itu sangat jauh dari standard kekeruhan normal
  }
  else //Jika kondisi pertama tidak terpenuhi
  {
    kekeruhan = -1120.4*VHasil*VHasil+5742.3*VHasil-4353.8; //Mendapatkan jumlah TSS berdasarkan data yang diterima melalui rumus TSS
     
  }

if(kekeruhan < 0){  //Kondisi jika nilai kekeruhan di bawah nol (Bernilai negatif)
  kekeruhan = 0;  //Kekeruhan langsung dianggap nol
}

  //Sensor pH
 for(int i=0;i<10;i++)  //Program untuk melakukan pengulangan sampai 10 kali
 { 
 buffer_arr[i]=analogRead(pinPH); //Membaca tegangan yang diterima dari pin analog setiap perulangan
 delay(30); //Delay 30 mikrodetik sebelum masuk ke baris program sebelumnya
 }
 for(int i=0;i<9;i++) //Program untuk melakukan pengulangan sampai 9 kali
 {
 for(int j=i+1;j<10;j++) //Program untuk melakukan pengulangan sampai 10 kali
 {
 if(buffer_arr[i]>buffer_arr[j]) //Pengondisian ketika array pengulangan i lebih besar dari array pengulangan j
 {
 temp=buffer_arr[i]; //Pendeklarasian bahwa temp = pembacaan analog dari sensor ph
 buffer_arr[i]=buffer_arr[j];//Pendeklarasian agar buffer_arr[i] (Berisi pembacaan sensor) dibuat sama dengan buffer_arr[j]
 buffer_arr[j]=temp; //Pendeklarasian bahwa buffer_arr[j] (Sekarang membawa analog read dari sensor ph = temp
 }
 }
 }
 avgval=0; //Pendeklarasian rata rata nilai yang dihasilkan dari pembacaan sensor = 0
 for(int i=2;i<8;i++) //Pengulangan brogram sampai 5 kali
 avgval+=buffer_arr[i]; //Setiap ada nilai baru, maka akan ditambahkan dengan nilai pembacaan lama

 
 float volt=(float)avgval*5.0/1024/6; //Rumus nilai pembacaan pH dengan sensor analog
 ph_act = -5.70 * volt + calibration_value; //Rumus untuk menentukan nilai akhir pH dengan tambahan nilai kalibrasi


//Print Serial Kualitas Air

 Serial.println("Kualitas Air"); // Menampilkan tulisan pada serial monitor
 Serial.print("1. "); // Menampilkan tulisan pada serial monitor
 Serial.print("pH Value : "); // Menampilkan tulisan pada serial monitor
  Serial.println(ph_act); // Menampilkan jumlah pH berdasarkan hasil perhitungan sensor

 Serial.print("2. "); // Menampilkan tulisan pada serial monitor
 Serial.print("Tegangan :"); // Menampilkan tulisan pada serial monitor
 Serial.print(VHasil); // Menampilkan jumlah tegangan analog yang diterima oleh pembacaan TSS sensor
 Serial.print(" V"); //Menampilkan tulisan pada serial monitor

 Serial.print("\t Kekeruhan :"); // Menampilkan tulisan pada serial monitor
 Serial.println(kekeruhan); // Menampilkan jumlah zat tersuspensi yang dibaca oleh sensor ph

 Serial.print("3. "); // Menampilkan tulisan pada serial monitor
 Serial.print("Jumlah Zat Terlarut : "); // Menampilkan tulisan pada serial monitor
 Serial.print(tdsValue,0); // Menampilkan jumlah zat terlarut yang dibaca oleh sensor tds
 Serial.println("ppm"); // Menampilkan tulisan pada serial monitor

lcd.clear(); // Membersihkan karakter pada lcd
 lcd.setCursor(0, 0); //Memposisikan karakter pertama ditulis dari baris pertama dan kolom pertama
 lcd.print ("TDS = "); //Tempat menuliskan karakter pada lcd
 lcd.print(tdsValue,0); //Tempat menuliskan karakter pada lcd berisi pembacaan sensor TDS
 lcd.print(" ppm"); //Tempat menuliskan karakter pada lcd

 lcd.setCursor(0, 1); //Memposisikan karakter pertama ditulis dari baris kedua dan kolom pertama
 lcd.print ("TSS = "); //Tempat menuliskan karakter pada lcd
 lcd.print(kekeruhan); //Tempat menuliskan karakter pada lcd berisi pembacaan sensor TSS
 lcd.print(" NTU"); //Tempat menuliskan karakter pada lcd

 lcd.setCursor(0, 2); //Memposisikan karakter pertama ditulis dari baris ketiga dan kolom pertama
 lcd.print ("pH  = "); //Tempat menuliskan karakter pada lcd
 lcd.print(ph_act); //Tempat menuliskan karakter pada lcd berisi pembacaan sensor pH
 
 Sending_To_phpmyadmindatabase(); //Pengaktifan fungsi untuk mengirimkan data ke database
  delay(1000); //Menunggu satu detik sebelum seluruh program diulang kembali

}


  
void Sending_To_phpmyadmindatabase()   //Fungsi untuk melakukan pengiriman ke database
{//                                                                                                                          alamat IP    port
  if (client.connect(server, 80)) { //Koneksi mikrokontroler dengan server yang telah diinisialisasi beserta portnya               V       v
                                    //Dalam hal ini PT. Sinko menggunakan port 80 sehingga penyebutannya menjadi 'client connect (server, 80)      '
    Serial.println("sended"); //print terhadap serial monitor bahwa data berhasil terhubung
    // GET from php files localy
    // Make a HTTP request:
  Serial.print("GET /water/kualitas.php?tds="); // Membangkitkan URL yang ditarget dan Memasukkan parameter TDS ke dalam URL
  client.print("GET /water/kualitas.php?tds="); // Membangkitkan URL yang ditarget dan Memasukkan parameter TDS ke dalam URL
    Serial.print(tdsValue,0); //Memasukkan nilai TDS yang telah didapatkan oleh sensor dan mikrokontroler
    client.print(tdsValue,0); //Memasukkan nilai TDS yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&tss="); //Memasukkan parameter TSS ke dalam URL
    Serial.print("&tss="); //Memasukkan parameter TSS ke dalam URL
    client.print(kekeruhan); //Memasukkan nilai TSS yang telah didapatkan oleh sensor dan mikrokontroler
    Serial.print(kekeruhan); //Memasukkan nilai TSS yang telah didapatkan oleh sensor dan mikrokontroler
    client.print("&ph="); //Memasukkan parameter pH ke dalam URL
    Serial.print("&ph="); //Memasukkan parameter pH ke dalam URL
    client.print(ph_act); //Memasukkan nilai pH yang telah didapatkan oleh sensor dan mikrokontroler
    Serial.print(ph_act); //Memasukkan nilai pH yang telah didapatkan oleh sensor dan mikrokontroler
    client.print(" ");      //memberikan SPACE sebelum HTTP/1.1 (Prosedur bawaan koneksi mikrokontroler ke website)
    client.print("HTTP/1.1"); //Memberikan tipe HTTP yang digunakan (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println(); //membuat line baru untuk inisialisasi client (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println("Host: 192.168.13.2"); //IP host yang digunakan (Gunakan ip address yang sudah dibuat sebelumnya) (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println("Connection: close"); //menutup konektivitas dengan client (Prosedur bawaan koneksi mikrokontroler ke website)
    client.println(); //Memberikan spasi jeda untuk pengiriman selanjutyan (Prosedur bawaan koneksi mikrokontroler ke website)
  
  } else { // Jika tidak mendapatkan koneksi ke server
    
    Serial.println("connection failed"); //Mengirimkan tulisan koneksi gagal ke serial monitor
    Ethernet.begin(mac, ip); //Jika tidak dapat terhubung ke server maka koneksi dilakukan sekali lagi 
  }
}
