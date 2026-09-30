const int pinV1 = A0;   // Voltaje de entrada
const int pinV2 = A1;   // Punto 1 del shunt de entrada

const float R1 = 3.3;   // Shunt de entrada en ohmios

const float VREF = 5.0; //voltaje de Referrencia soportado

const int NUM_MUESTRAS = 1000;//Para tener buenos valores de muestra

// ===============================
// CONTROL DE CARGAS
// ===============================

const int primerPinCarga = 2;  //Configuración de los pines dgitales con el pin2 como la primera carga
const int ultimoPinCarga = 5; //configuración de los pines digitales con el pin5 como la cuarta y ultima carga

const int NUM_CARGAS = 4;     //Número de Cargas

// Vector de estado de las cargas
int cargas[NUM_CARGAS] = {0, 0, 0, 0};  //Vector que me permite guardar los estados de las 4 cargas con un bit por carga


void setup() {

  Serial.begin(9600);

  // Configurar pines 2 a 5 como salidas
  for (int pin = primerPinCarga; pin <= ultimoPinCarga; pin++) {  //Recorre los diferentes niveles de de pin para asignarles como salida y apagarlos. Usando la variable pin

    pinMode(pin, OUTPUT); //Desgina cada  variable pin desde el 2 al 5 como salida

    digitalWrite(pin, LOW); // Todas las cargas "pin" comienzan apagadas
  }
}


void loop() {

  // =================================
  // RECEPCIÓN DE VECTOR DESDE MATLAB
  // =================================

  if (Serial.available() >= NUM_CARGAS) { //¿La cantidad de datos que entran como vector son exactamente igual al npumero de cargas?

    bool ordenValida = true;  //Preguntamos mediante el tipo de variable bool si el vector es de caracter binario

    // Leer los 4 elementos del vector
    for (int i = 0; i < NUM_CARGAS; i++) {  //Leemos los valores del vector binario de 1 bit en un bit

      char dato = Serial.read();  //Revisa y forma el vector en base a lo enviado armando los datos que llegaron como 1110 a [1,1,1,0]

      if (dato == '0') {  //El primer valor que estamos leendo y pregunta si es u caracter '0' para desginar en el valor de esa carga i el estado logico 0
        cargas[i] = 0;
      }
      else if (dato == '1') { //Cuando no cumple la condición anterior entonces preguntamos si es '1' y designamos valor booleano 1
        cargas[i] = 1;
      }
      else {
        ordenValida = false;  //Protección contra ordenes invalidas
      }
    }

    // =================================
    // APLICAR VECTOR A LAS CARGAS
    // =================================

    if (ordenValida) {  //Si todo fue correcto, entramos. Si no, Arduino simplemente no modifica las cargas.

      for (int i = 0; i < NUM_CARGAS; i++) {  //Recorremos de nuevo las 4 posiciones pero ahora estamos viendo el vector formado no el numero binario enviado por MATLAB

        int pin = primerPinCarga + i; //Estamos haciendo que cada valor del vector se corresponda con un pin. pin_i=2+i. [0,1,2,4]->[2,3,4,5]

        if (cargas[i] == 1) { //Leemos el vector formado posición por posición y preguntamos si la actual posición es 1
          digitalWrite(pin, HIGH);  //Entonces sera 1 booleano
        }
        else {
          digitalWrite(pin, LOW); //Si no es 1 entonces como 0 booleano
        }
      }
    }
  }


  // =================================
  // ADQUISICIÓN DE DATOS
  // =================================

  long sumaV1 = 0;  //Variables de tipo Long
  long sumaV2 = 0;  //Variables de tipo Long

  // Promediar muchas mediciones
  for (int i = 0; i < NUM_MUESTRAS; i++) {  //Hacemos 1000 muestras antes de definir un valor

    sumaV1 += analogRead(pinV1);  //Vamos acumulando las 1000 muestras en una variable "sumaV1" mientras leemos el pin analogico
    sumaV2 += analogRead(pinV2);  //Vamos acumulando las 1000 muestras en una variable "sumaV2" mientras leemos el pin analogico
  }
  //La operación += es lo mismo en abreviado que decir a=a+a, osea digamos vamos incrementando los diferentes valores obtenidos uno detras de otro


  // =================================
  // PROMEDIOS ADC
  // =================================

  float lecturaV1 = sumaV1 / (float)NUM_MUESTRAS; //Promediamos los valores obtenidos
  float lecturaV2 = sumaV2 / (float)NUM_MUESTRAS; //Promediamos los valores obtenidos


  // =================================
  // CONVERSIÓN ADC → VOLTAJE
  // =================================

  float voltajeV1 = lecturaV1 * (VREF / 1023.0);  //El vaslor de un ADC de arduino es de 1023 su resolución entonces hagarramos el valor y lo ponemos en un de esos posibles estados
  float voltajeV2 = lecturaV2 * (VREF / 1023.0);  //El vaslor de un ADC de arduino es de 1023 su resolución entonces hagarramos el valor y lo ponemos en un de esos posibles estados


  // =================================
  // VALIDACIÓN DEL SHUNT
  // =================================

  if (voltajeV1 > voltajeV2) {  //Teoricamente siempre la tensión de cae pero al leer con arduino se puede dar supuestos inversos lo que es falso en la teoria entonces ponemos condición 

    float voltajeShunt = voltajeV1 - voltajeV2;    // Caída de tensión en el shunt

    float corriente1 = voltajeShunt / R1;    // Corriente en amperios

    float corriente1_mA = corriente1 * 1000.0;    // Corriente en mA


    // =================================
    // ENVÍO A MATLAB
    // =================================

    // Formato:
    // VoltajeEntrada, Corriente_mA

    Serial.print(voltajeV1, 8);
    Serial.print(",");
    Serial.println(corriente1_mA, 8);
  }


  // =================================
  // PEQUEÑA PAUSA
  // =================================

  delay(100);
}