//--------------------------------------------------------------------------//
// Codigo realizado por Gianfranco Mendez. UCV 2020/2021                    //
// Lector y programador del dispositivo IBUTTON DS1982 de MAXIM             //
// Utilizando servidor WEB de un ESP8266                                    //
//--------------------------------------------------------------------------//

#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <iostream>
#include <string>
//#include<bits/stdc++.h>
//using namespace std;

AsyncWebServer server(80);

const char *ssid = "YOUR SSID";          //Nombre de la RED
const char *password = "YOUR PW"; //Contraseña de la RED

const int GPIO15 = 15; //Puerto de pulso de programacion 12 V
const int GPIO04 = 04; //Puerto auxiliar
const int GPIO00 = 00; //Puerto de comunicacion one wire

const char *PARAM_INPUT_1 = "Programacion"; //Nombre del boton de programación
const char *PARAM_INPUT_2 = "Direccion";    //Nombre del boton de dirección

const char index_html[] PROGMEM = R"rawliteral(  
 <!DOCTYPE HTML>
<html>
	<head>
		<title>iButton DS1982</title>
		<meta name="viewport" content="width=device-width, initial-scale=1">
	</head>
	<body>
    <div style="width: 800px; height: 100px; text-align: justify; margin: auto; overflow: hidden; background-color: #2D446A">
      <img src="" style="height: 100px">
		</div>
		<div style="width: 800px; text-align: justify; margin: auto;">
		<table>
			<tr>
				<td colspan="3">
					<p>Programador/Lector de memoria EPROM DS1982, esta distribuida en 4 paginas de 256bits/32bytes cada una. La escritura/lectura de la memoria EPROM es de 32bytes</p>
				</td>
			</tr>
			<tr>
				<td colspan="3">
					<p>Tome en cuenta que se debe escribir el codigo de 32 bytes completo (64 letras), en caso de copiar una menor cantidad, se completara con ceros, solo los valores FF en la memoria EPROM se pueden programar. Primero se debe escribir el codigo
					de programacion que se desea, seleccionar la direccion inicial y luego darle al boton Write Data y luego al boton Read Data.</p>
				</td>
			</tr>
			<tr>
				<td style="height: 18px;">
					
				</td>
			</tr>
			<tr>
				<td colspan="3">
					<p>Codigo maximo 32 bytes, con el siguiente formato. EJ: 0F0F0F0F0F</p>
					<form action="/get">
					Codigo a cargar:  <input type="text" name="Programacion" style="width: 500px">
					<input type="submit" value="Subir">
					</form>
				</td>
			</tr>
			<tr>
				<td colspan="3">
					<p>Direccion inicial para seleccionar 1 de las 4 posibles paginas de escritura 00, 20, 40 o 60. Valor por defecto 00</p>
					<form action="/get">
					Direccion inicial:  <input type="text" name="Direccion" style="width: 250px">
					<input type="submit" value="Subir">
					</form>
				</td>
			</tr>
			<tr style="height: 28px;">
				<td colspan="3">
					
				</td>
			</tr>
			<tr>
				<td style="text-align: center;">
					<a href="/RR"><button>Read ROM</button></a>
				</td>
				<td style="text-align: center;">
					<a href="/RD"\"><button>Read Data</button></a>
				</td>
				<td style="text-align: center;">
					<a href="/WD"\"><button>Write Data</button></a>
				</td>
			</tr>
			<tr style="height: 28px;">
				<td colspan="3">
					
				</td>
			</tr>
		</table>
		</div>
	</body>
  <div style="width: 800px; height: 100px; text-align: center; margin: auto; overflow: hidden; background-color: #627593">
    <img src="">
	</div>
</html>)rawliteral";

OneWire ds(GPIO00);  //Puerto OneWire en el GPIO 0
byte addr[8];        //Arreglo donde se guarda el ROM unico
byte leemem[12];     //Arreglo para direcciones y instrucciones
byte i;              // Variable para bucles
boolean present = 0; // Boolean para saber si hay o no dispositivo
byte data[32];       // Arreglo donde se guarda la data
byte ccrc;           // Variables para guardar los CRC
byte ccrc_calc;

byte Myid[32];         // Dato a programar max 32 Bytes
byte DirrIni = 0x00;   // Variable donde se almacena la dirección inicial
boolean CRCCHECK = 0;  //Variable para detectar diferencias en el CRC y CRC calculado por el ds1982
boolean ProgCheck = 0; //Variable para detectar si se introdujo por lo menos 1 codigo a programar.

String Data1 = ""; //Variable donde se guarda la ROM en forma de string para mostrar en el servidor WEB
String Data2 = ""; //Variable donde se guarda la Data de la EPROM en forma de string para mostrar en el servidor WEB

//--------------------------------------------------------------------------//
// Funcion para mostrar en HEX en servidor web //
//--------------------------------------------------------------------------//
int MostrarROM()
{                //Esta función convierte el valor obtenido
  String j = ""; //de la ROM unica y la transforma en una
  int i = 0;     //variable tipo String para poder mandarla
  while (i < 8)
  { //al servidor Web
    char aux[50];
    sprintf(aux, "%X", addr[i]);
    j = j + aux + " ";
    i++;
  }
  Data1 = j;
}

int MostrarDATA()
{                //Esta función convierte el valor obtenido
  String j = ""; //de la memoria EPROM y la transforma en una
  int i = 0;     //variable tipo String para poder mandarla
  while (i < 32)
  { //al servidor Web
    char aux[50];
    sprintf(aux, "%X", data[i]);
    j = j + aux + " ";
    i++;
  }
  Data2 = j;
}

//--------------------------------------------------------------------------//
// Funciones para DS1982 //
//--------------------------------------------------------------------------//

void ReadROM()
{ //Funcion para leer la ROM
  if (!ds.search(addr))
  { //Leo el valor y se lo asigno al arreglo addr
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7])
  { //Compruebo los CRC si no pasa me devuelvo
    CRCCHECK = 1;
    return;
  }
  MostrarROM();
  ds.reset();
}
//--------------------------------------------------------------------------//

void ReadData()
{                      //Funcion para leer la data de la memoria
  ds.skip();           //Obligatorio para acceder a las funciones de la EPROM
  leemem[0] = 0xC3;    //Instruccion para leer data y generar CRC
  leemem[1] = DirrIni; //Direcciones en la memoria EPROM
  leemem[2] = 0;
  ds.write(leemem[0], 1); //Mando al DS1982 la instruccion y las direcciones
  ds.write(leemem[1], 1);
  ds.write(leemem[2], 1);

  ccrc = ds.read();                     //DS1982 devuelve el CRC que calculo
  ccrc_calc = OneWire::crc8(leemem, 3); //Calculo el valor del CRC de lo enviado

  for (byte i = 0; i < 32; i++)
  { //Imprimo la data de la memoria EPROM
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  MostrarDATA();
}

//--------------------------------------------------------------------------//

void Pulso()
{ //Funcion para ordenar el pulso de programacion
  digitalWrite(GPIO15, HIGH);
  digitalWrite(GPIO04, LOW);
  delayMicroseconds(480); //Tiempo necesario para la programacion
  digitalWrite(GPIO15, LOW);
  digitalWrite(GPIO04, HIGH);
}

//--------------------------------------------------------------------------//

void ProgramData()
{
  ds.reset(); //Mando un pulso de reset
  ds.skip();  //Operación necesaria para poder ingresar los comandos de la memoria EPROM

  leemem[0] = 0x0F; //Comando de escritura, direccion inicial, direccion final y primer dato a programar
  leemem[1] = DirrIni;
  leemem[2] = 0;
  leemem[3] = Myid[0];
  ds.write_bytes(leemem, 4, 1);         //Se escriben los datos almacenados en leemem
  ccrc = ds.read();                     //DS1982 devuelve el CRC que calculo
  ccrc_calc = OneWire::crc8(leemem, 4); //calculamos el CRC

  if (ccrc_calc != ccrc)
  { //Comparamos los 2 CRC, deben ser iguales
  }
  else
  {
    Pulso();             //Si es correcto ocurre el pulso de programacion
    data[0] = ds.read(); //El DS1982 devuelve el dato escrito
  }
  for (i = 1; i < sizeof(Myid); i++)
  { //Ciclo para terminar de realizar las escrituras faltantes
    leemem[1]++;
    leemem[2] = Myid[i];
    ds.write(leemem[2], 1);
    ccrc = ds.read();
    Pulso();
    data[0] = ds.read();
  }
}

//--------------------------------------------------------------------------//
// Funciones para servidor WEB //
//--------------------------------------------------------------------------//

void handleROM(AsyncWebServerRequest *request)
{
  if (present == 1)
  {
    ReadROM();
    request->send(200, "text/html", "La ROM leida fue: " + Data1 + "<br><a href=\"/\">Regresar a la pagina principal</a>");
  }
  else if (CRCCHECK == 1)
  {
    request->send(200, "text/html", "CRC Incorrecto, verifique si es un dispositivo DS1982 <br><a href=\"/\">Regresar a la pagina principal</a>");
    CRCCHECK = 0;
  }
  else
  {
    request->send(200, "text/html", "No hay un dispositivo DS1982 Conectado, Verifique. <br><a href=\"/\">Regresar a la pagina principal</a>");
  }
}
void handleDATA(AsyncWebServerRequest *request)
{
  if (present == 1)
  {
    ReadData();
    request->send(200, "text/html", "La Data almacenada en la Memoria EPROM es: " + Data2 + "<br><a href=\"/\">Regresar a la pagina principal</a>");
  }
  else
  {
    request->send(200, "text/html", "No hay un dispositivo DS1982 Conectado, Verifique. <br><a href=\"/\">Regresar a la pagina principal</a>");
  }
}
void handleWRITE(AsyncWebServerRequest *request)
{
  if (present == 1 && ProgCheck == 1)
  {
    for (int j = 0; j < 10; j++)
    {
      ProgramData();
    }
    if (CRCCHECK == 1)
    {
      request->send(200, "text/html", "CRC Incorrecto, verifique si es un dispositivo DS1982 <br><a href=\"/\">Regresar a la pagina principal</a>");
    }
    else
    {
      request->send(200, "text/html", "La memoria fue programada Correctamente <br><a href=\"/\">Regresar a la pagina principal</a>");
    }
  }
  else
  {
    request->send(200, "text/html", "No hay un dispositivo DS1982 Conectado, Verifique. <br><a href=\"/\">Regresar a la pagina principal</a>");
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Pagina no encontrada");
}

//--------------------------------------------------------------------------//

void setup()
{

  //////////////////////Configuracion de puertos inicial///////////////////////
  Serial.begin(115200);    //Inicio comunicacion serial
  pinMode(GPIO15, OUTPUT); //Puerto GPIO15 y GPIO04 salidas
  pinMode(GPIO04, OUTPUT);
  digitalWrite(GPIO15, LOW); //GPIO15 en bajo y GPIO04 en alto
  digitalWrite(GPIO04, HIGH);

  //////////////////////Configuracion servidor WEB ///////////////////////////

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Envia al servidor web el codigo HTML almacenado en el index
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Si se oprime el boton se guarda lo que escribio el usuario
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1))
    {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      request->send(200, "text/html", "informacion recibida fue  (" + inputParam + ") con el valor: " + inputMessage1 + "<br><a href=\"/\">Return to Home Page</a>");
      ProgCheck = 1;
      char test1[32];
      int num;

      for (int k = 0; k < inputMessage1.length() / 2; k++)
      {                                  //Seccion de codigo que cumple la transformacion del String recibido
        test1[0] = inputMessage1[2 * k]; // y lo transforma en un arreglo de bytes para poder programarlos
        test1[1] = inputMessage1[2 * k + 1];
        test1[2] = '\0';
        num = strtoul(test1, NULL, 16);
        unsigned char *bytePtr = (unsigned char *)&num;
        Myid[k] = bytePtr[0];
        Serial.print(Myid[k]);
        Serial.print(" ");
      }
      Serial.println();
    }
    else if (request->hasParam(PARAM_INPUT_2))
    {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      request->send(200, "text/html", "La informacion recibida fue  (" + inputParam + ") con el valor: " + inputMessage2 + "<br><a href=\"/\">Return to Home Page</a>");
      if (inputMessage2 == "00")
      { //Veo lo recibido en el input de direccion y escribo la direccion inicial
        DirrIni = 0x00;
      }
      if (inputMessage2 == "20")
      {
        DirrIni = 0x20;
      }
      if (inputMessage2 == "40")
      {
        DirrIni = 0x40;
      }
      if (inputMessage2 == "60")
      {
        DirrIni = 0x60;
      }
    }
  });
  server.on("/RR", handleROM);   //Cuando se preciona el boton de READ ROM se realiza la funcion handleROM para mostrarla en el servidor WE
  server.on("/RD", handleDATA);  //Cuando se preciona el boton de READ Data se realiza la funcion handleDATA para mostrarla en el servidor WEB
  server.on("/WD", handleWRITE); //Cuando se preciona el boton de Write Data se realiza la funcion handleWRITE para realizar la programacion del DS1982
  server.onNotFound(notFound);
  server.begin();
}

//--------------------------------------------------------------------------//

void loop()
{
  present = ds.reset(); // Operacion obligatoria para saber cuando esta conectado un dispositivo OneWire
  delay(1000);
}
