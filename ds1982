  //////////////////////////////
  // Reading & programming    //
  // for the MAXIM DS1982     //
  // Made by Gianfranco Mendez//
  //////////////////////////////
  
  #include <OneWire.h>

  const int GPIO15 = 15;            //Puerto de pulso de programacion 12 V
  const int GPIO05 = 05;            //puerto de prueba de pulso 
  const int GPIO04 = 04;            //Puerto auxiliar

  OneWire ds(0);                    //Puerto OneWire en el GPIO 0
  byte addr[8];                     //Arreglo donde se guarda el ROM unico 
  byte leemem[12];                  //Arreglo para direcciones y instrucciones
  byte i;                           // Variable para bucles
  boolean present = 0;              // Boolean para saber si hay o no dispositivo
  byte data[32];                    // Arreglo donde se guarda la data
  byte ccrc;                        // Variables para guardar los CRC
  byte ccrc_calc;
  byte Myid[8]= {0x09, 0xD0, 0xC3, 0x32, 0x44, 0x00, 0xB3, 0xD5};

 //--------------------------------------------------------------------------//
 
void setup() {
  Serial.begin (115200);            //Inicio comunicacion serial
  pinMode(GPIO05, INPUT);           //Puerto GPIO05 entrada
  pinMode(GPIO15, OUTPUT);          //Puerto GPIO15 y GPIO04 salidas
  pinMode(GPIO04, OUTPUT);          
  digitalWrite(GPIO15, LOW);        //GPIO15 en bajo y GPIO04 en alto
  digitalWrite(GPIO04, HIGH);       
}

 //--------------------------------------------------------------------------//

void Pulso(){                       //Funcion para ordenar el pulso de programacion
  digitalWrite(GPIO15, HIGH);
  digitalWrite(GPIO04, LOW);
  delayMicroseconds(480);
  digitalWrite(GPIO15,LOW);
  digitalWrite(GPIO04, HIGH);
  }

 //--------------------------------------------------------------------------//

void ProgramData(){
  Serial.println("Programando la memoria EPROM");
  ds.reset();
  ds.skip();

  leemem[0] = 0x0F;
  leemem[1] = 0x00;
  leemem[2] = 0;
  leemem[3] = Myid[0];

  ds.write_bytes(leemem,4,1);

  ccrc = ds.read();
  ccrc_calc = OneWire::crc8(leemem,4);

  if (ccrc_calc != ccrc ){
    Serial.println("Comando invalido");
    Serial.print("Crc calculado: ");
    Serial.println(ccrc_calc,HEX);
    Serial.print("Crc devuelto por el DS1982: ");    
    Serial.println(ccrc, HEX);
    }
  else
    {
      Pulso(); 
      data[0] = ds.read();
    }
  Serial.print("data programada: ");
  Serial.println(data[0],HEX);

  for (i = 1; i < 8; i++){
    leemem[1]++;
    leemem[2]=Myid[i];
    ds.write(leemem[2],1);
    ccrc = ds.read();
    Pulso();
    }
  }

 //--------------------------------------------------------------------------//

void ReadData(){                    //Funcion para leer la data de la memoria
  Serial.println("Leyendo Data en la memoria EPROM");
  
  ds.skip();                        //Obligatorio para acceder a las funciones de la EPROM
  leemem[0] = 0xC3;                 //Instruccion para leer data y generar CRC
  leemem[1] = 0x00;                 //Direcciones en la memoria EPROM
  leemem[2] = 0;
  ds.write(leemem[0],1);            //Mando al DS1982 la instruccion y las direcciones
  ds.write(leemem[1],1);
  ds.write(leemem[2],1);
  
  ccrc = ds.read();                 //DS1982 devuelve el CRC que calculo
  ccrc_calc = OneWire::crc8(leemem,3);  //Calculo el valor del CRC de lo enviado
  
  Serial.println(ccrc,HEX);         //Muestro ambos CRC
  Serial.println(ccrc_calc,HEX);
    
  for(byte i = 0; i<32;i++){        //Imprimo la data en la memoria EPROM
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
    }
  Serial.println();
  }

 //--------------------------------------------------------------------------//
 
void ReadROM(){                     //Funcion para leer la ROM
  Serial.println("Leyendo el valor Unico de la ROM");
  
    if (!ds.search (addr)){         //Leo el valor y se lo asigno al arreglo addr
    ds.reset_search();
    delay(250);
    return;
 }
    Serial.print("ROM= ");          //Muestro la ROM en HEX
    for (byte i = 0; i<8; i++){ 
      Serial.print(addr[i],HEX); 
      Serial.print(" ");
 }
    Serial.println();
    
    if (OneWire::crc8(addr,7)!= addr[7]){ //Compruebo los CRC si no pasa me devuelvo
      Serial.println("CRC no valido");
      return;
 }

  ccrc=addr[7];
  ccrc_calc= OneWire::crc8(addr,7);
  
  Serial.println();
  Serial.println(ccrc,HEX);
  Serial.println(ccrc_calc,HEX);
  Serial.println();
  ds.reset();
  }

 //--------------------------------------------------------------------------//

void loop() {  
  present = ds.reset();                // Operacion obligatoria para saber cuando esta conectado un dispositivo OneWire
  if ( digitalRead(GPIO05) == LOW) {
    ProgramData();
  }
  if (present == true) {
    Serial.println("Dispositivo DS1982 conectado"); 
    ReadROM();
    ReadData();
    }  
  delay(1000);
}
