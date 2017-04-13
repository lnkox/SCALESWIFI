#include <Q2HX711.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Для входу в налаштування перейти в браузері до  http://192.168.4.1 

// Назва та пароль точки доступу для налаштування пристрою
const char *ssid = "ScaleAdmin";
const char *password = "";

const char* host = "uichat.in.ua";

const byte button_pin = D2; // пін для апаратної кнопки зміни режиму
const byte led_pin= D1; // пін для світлодіода індикації
const byte hx711_data_pin = D5; // пін для DATA HX711
const byte hx711_clock_pin = D6; // пін для SCK HX711

int vaga=0;

char modes=0;
int timr=0;
char err=0;
struct eeprom_data_t {
  char eeind[20];
  char eekey[20];
  char eessid[20];
  char eepass[20];
  long eetare;
} eeprom_data;


ESP8266WebServer server(80); // порт веб сервера для налаштування
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

void handleRoot() { // Генерація та відправка сторінки налаштувань
  String strm = "";
  String wflans = "";
  int n = WiFi.scanNetworks(); // пошук wi-fi мереж
  if (n == 0)
  {
    wflans="<h5 align=center>Відсутні мережі</h5>";
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      wflans += "<a href='#' onclick='document.getElementById(\"ssid\").value=this.innerHTML'>"+WiFi.SSID(i)+"</a> ("+WiFi.RSSI(i)+"dbm) <br>";
    }
  }
  strm +=" <html><head>";
  strm +="<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>";
  strm +="<meta http-equiv='Content-Language' content='uk'>";
  strm +="<title>WI-FI SCALES</title>";
  strm +="</head>";
  strm +="<body>";
  strm +="<h1 align='center'>WI-FI SCALES</h1>";
  strm +="<form method='POST' action='/set'>";
  strm +="	<table border='0' width='95%'>";
  strm +="		<tr>";
  strm +="			<td width='155'>SSID(Назва мережі): </td>";
  strm +="			<td width='572'><input type='text' name='ssid' id='ssid' value='"+String(eeprom_data.eessid)+"' size='35'></td>";
  strm +="			<td rowspan='5' width='294' valign='top'>";
  strm +="			<h3 align='center'>WI-FI мережі</h3>"+wflans;
  strm +="			</td>";
  strm +="		</tr>";
  strm +="		<tr>";
  strm +="			<td width='155'>Пароль: </td>";
  strm +="			<td width='572'><input type='text' name='pass' value='"+String(eeprom_data.eepass)+"' size='35'></td>";
  strm +="		</tr>";
  strm +="		<tr>";
  strm +="			<td width='155'>Індифікатор:</td>";
  strm +="			<td width='572'><input type='text' name='ind' value='"+String(eeprom_data.eeind)+"' size='35'></td>";
  strm +="		</tr>";
  strm +="		<tr>";
  strm +="			<td width='155'>Ключ:</td>";
  strm +="			<td width='572'><input type='text' name='key' value='"+String(eeprom_data.eekey)+"' size='35'></td>";
  strm +="		</tr>";
  strm +="		<tr>";
  strm +="			<td width='727' colspan='2' align='center'>";
  strm +="			<input type='submit' value='Зберегти' name='B1'></td>";
  strm +="		</tr>";
  strm +="	</table>";
  strm +="</form>";
  strm +="<br><br><a href='/tare' >Онулити тару</a>";
  strm +="<br><br>Поточний показник тари:"+String(eeprom_data.eetare);
  strm +="</body>";
  strm +="</html>";
  server.send(200, "text/html", strm);
}
void handleSet() // Збереження налаштувань від користувача та генерація і відправка сторінки зі збереженими налаштуваннями
{
  String strm = "";
  server.arg("ssid").toCharArray(eeprom_data.eessid, sizeof(eeprom_data.eessid));
  server.arg("pass").toCharArray(eeprom_data.eepass, sizeof(eeprom_data.eepass));
  server.arg("ind").toCharArray(eeprom_data.eeind, sizeof(eeprom_data.eeind));
  server.arg("key").toCharArray(eeprom_data.eekey, sizeof(eeprom_data.eekey));
  writeSet();  
  strm +=" <html><head>";
  strm +="<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>";
  strm +="<meta http-equiv='Content-Language' content='uk'>";
  strm +="<title>WI-FI SCALES</title>";
  strm +="</head>";
  strm +="<body>";
  strm +="<h1 align='center'>WI-FI SCALES</h1>";
  strm +="<h3 align='center'>Налаштування збережено</h1>";
  strm +="<br>SSID(Назва мережі):"+String(eeprom_data.eessid);
  strm +="<br>Пароль:"+String(eeprom_data.eepass);
  strm +="<br>Індифікатор:"+String(eeprom_data.eeind);
  strm +="<br>Ключ:"+String(eeprom_data.eekey);
  strm +="</body>";
  strm +="</html>";
  server.send(200, "text/html", strm);
}
void handletare()
{
  String strm = "";
  eeprom_data.eetare=get_absweight();
  writeSet();  
  strm +=" <html><head>";
  strm +="<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>";
  strm +="<meta http-equiv='Content-Language' content='uk'>";
  strm +="<title>WI-FI SCALES</title>";
  strm +="</head>";
  strm +="<body>";
  strm +="<h1 align='center'>WI-FI SCALES</h1>";
  strm +="<h3 align='center'>Тара онулена</h1>";
  strm +="<br>Поточний показник:"+String(eeprom_data.eetare);
  strm +="</body>";
  strm +="</html>";
  server.send(200, "text/html", strm);
}
void setup_to_setmode() // Налаштування пристрою в режим  "Налаштувань"
{
  modes=1;  // Режим  "Налаштувань"
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/tare", handletare);
  server.begin();
  readSet();
  digitalWrite(led_pin, HIGH); 
  Serial.println("Server and AP created");
}
void setup_to_workmode() // Налаштування пристрою в робочий режим
{
  modes=0;  // Робочий режим
  readSet();
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(eeprom_data.eessid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(eeprom_data.eessid,eeprom_data.eepass); // підключення до WI-FI мережі заданої в налаштуваннях
  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(led_pin, HIGH); 
    delay(250);
    Serial.print(".");
    digitalWrite(led_pin, LOW); 
    delay(250);
   }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}
void setup() {
  pinMode(button_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  delay(1000);
  Serial.begin(9600);
  if (digitalRead(button_pin)==1)
  {
    delay(500);
    if (digitalRead(button_pin)==1){ setup_to_setmode();}
  }
  if (digitalRead(button_pin)!=1) {setup_to_workmode();}
}
void loop()
{
  if (modes==1)
  {
     server.handleClient();
  }
  else
  {
    delay(100);
    timr++;
    if (timr==5){timr=0; send_to_server();}
    if (err>5) {ESP.reset();} // При більше як 5 помилок поспіль  - пристрій перезавантажується
  }
}
void send_to_server()// Відправка даних на сервер
{
  //Serial.print("connecting to ");
  //Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    digitalWrite(led_pin, HIGH);
    err++; // Лічильник помилок звязку
    return;
  }
  vaga++;
  String url = "/add.php?ind="+String(eeprom_data.eeind)+"&vaga="+String(get_weight()); //  Генерація URL для запиту
  //Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n"); // Відправка запиту
  delay(10);
  //while(client.available())
  //{
  //  String line = client.readStringUntil('\r');
  //  Serial.print(line);
  //  err=0;  // Скидання лічильника помилок звязку
  //}
  err=0;  // Скидання лічильника помилок звязку (тимчасово)
  digitalWrite(led_pin, HIGH); // Індикація відправленого запиту
  delay(20);
  digitalWrite(led_pin, LOW);
  //Serial.println();
  //Serial.println("closing connection");
}
long get_absweight()
{
  int u=0;
  long sumw=0;
  for (int nr=1; nr<=10;nr++)
  {
    sumw=sumw+hx711.read();
    delay(5);
  }
  sumw=sumw/10;
  //Serial.println(sumw);
  return sumw;
}
long get_weight()
{
  return (get_absweight()-eeprom_data.eetare)/255;
}
void readSet() //Читання налаштувань з енергонезалежної памяті в оперативну
{
  int i;
  byte eeprom_data_tmp[sizeof(eeprom_data)];
  EEPROM.begin(sizeof(eeprom_data));
  for (i =0; i <sizeof(eeprom_data); i++)
  {
    eeprom_data_tmp[i] = EEPROM.read(i);
  }
  memcpy(&eeprom_data, eeprom_data_tmp,  sizeof(eeprom_data));
}

void writeSet() //Запис налаштувань з оперативної памяті в енергонезалежну
{
  int i;
  byte eeprom_data_tmp[sizeof(eeprom_data)];
  EEPROM.begin(sizeof(eeprom_data));
  memcpy(eeprom_data_tmp, &eeprom_data, sizeof(eeprom_data));
  for (i = 0; i < sizeof(eeprom_data); i++)
  {
    EEPROM.write(i, eeprom_data_tmp[i]);
  }
  EEPROM.commit();
}
