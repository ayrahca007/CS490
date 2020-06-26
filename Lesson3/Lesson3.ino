#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

unsigned f1;
unsigned f2;
unsigned f3;
unsigned f4;
unsigned f5;
unsigned f6;
float f7;
/************************MQ135sensor************************************/
/************************Hardware Related Macros************************************/
#define         MQ135PIN                       (3)     //define which analog input channel you are going to use
#define         RL_VALUE_MQ135                 (1)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR_MQ135      (3.59)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                       //which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
#define         GAS_CARBON_DIOXIDE           (9)
#define         GAS_CARBON_MONOXIDE          (3)
#define         GAS_ALCOHOL                  (4)
#define         GAS_AMMONIUM                 (10)
#define         GAS_TOLUENE                  (11)
#define         GAS_ACETONE                  (12)
//#define         accuracy                     (0)   //for linearcurves
#define         accuracy                   (1)   //for nonlinearcurves, un comment this line and comment the above line if calculations 
                                                   //are to be done using non linear curve equations
/*****************************Globals***********************************************/
float           Ro = 10;                            //Ro is initialized to 10 kilo ohms

void setup() {
  Serial.begin(9600);                               //UART setup, baudrate = 9600bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ135PIN);                     //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");

  unsigned status;
    
  // default settings
  status = bme.begin(0x76);  
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x");
      Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
    }
  Serial.println("-- Default Test --");
}

void loop() {
  update();
  String command;
  command = "MSG";
  command += "&field1=";
  command += f1;
  command += "&field2=";
  command += f2;
  command += "&field3=";
  command += f3;
  command += "&field4=";
  command += f4;
  command += "&field5=";
  command += f5;
  command += "&field6=";
  command += f6;
  command += "&field7=";
  command += f7;
  Serial.println(command);
  Serial.println();
  delay(2000);
}

void update() {
  f1 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_CARBON_DIOXIDE);
  f2 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_CARBON_MONOXIDE);
  f3 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_ALCOHOL) ;
  f4 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_AMMONIUM) ;
  f5 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_TOLUENE) ;
  f6 = MQGetGasPercentage(MQRead(MQ135PIN)/Ro,GAS_ACETONE) ;
  f7 = bme.readTemperature();


   Serial.print("CARBON_DIOXIDE:"); 
   Serial.print(f1);
   Serial.println( "ppm" );   
   Serial.print("CARBON_MONOXIDE:"); 
   Serial.print(f2);
   Serial.println( "ppm" ); 
   Serial.print("ALCOHOL:"); 
   Serial.print(f3);
   Serial.println( "ppm" ); 
   Serial.print("AMMONIUM:"); 
   Serial.print(f4);
   Serial.println( "ppm" );
   Serial.print("TOLUENE:"); 
   Serial.print(f5);
   Serial.println( "ppm" );
   Serial.print("ACETONE:"); 
   Serial.print(f6);
   Serial.println( "ppm" );
   Serial.print("Temperature = ");
   Serial.print(f7);
   Serial.println(" *C");
   Serial.println();
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE_MQ135*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float RS_AIR_val=0,r0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {                     //take multiple samples
    RS_AIR_val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  RS_AIR_val = RS_AIR_val/CALIBARAION_SAMPLE_TIMES;              //calculate the average value

  r0 = RS_AIR_val/RO_CLEAN_AIR_FACTOR_MQ135;                      //RS_AIR_val divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                                 //according to the chart in the datasheet 

  return r0; 
}

/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function uses different equations representing curves of each gas to 
         calculate the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{ 
  if ( accuracy == 0 ) {
  if ( gas_id == GAS_CARBON_DIOXIDE ) {
    return (pow(10,((-2.890*(log10(rs_ro_ratio))) + 2.055)));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,((-3.891*(log10(rs_ro_ratio))) + 2.750)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-3.181*(log10(rs_ro_ratio))) + 1.895)));
  } else if ( gas_id == GAS_AMMONIUM ) {
    return (pow(10,((-2.469*(log10(rs_ro_ratio))) + 2.005)));
  } else if ( gas_id == GAS_TOLUENE ) {
    return (pow(10,((-3.479*(log10(rs_ro_ratio))) + 1.658)));
  } else if ( gas_id == GAS_ACETONE ) {
    return (pow(10,((-3.452*(log10(rs_ro_ratio))) + 1.542)));
  }   
} 

  else if ( accuracy == 1 ) {
    if ( gas_id == GAS_CARBON_DIOXIDE ) {
    return (pow(10,((-2.890*(log10(rs_ro_ratio))) + 2.055)));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,(1.457*pow((log10(rs_ro_ratio)), 2) - 4.725*(log10(rs_ro_ratio)) + 2.855)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-3.181*(log10(rs_ro_ratio))) + 1.895)));
  } else if ( gas_id == GAS_AMMONIUM ) {
    return (pow(10,((-2.469*(log10(rs_ro_ratio))) + 2.005)));
  } else if ( gas_id == GAS_TOLUENE ) {
    return (pow(10,((-3.479*(log10(rs_ro_ratio))) + 1.658)));
  } else if ( gas_id == GAS_ACETONE ) {
    return (pow(10,(-1.004*pow((log10(rs_ro_ratio)), 2) - 3.525*(log10(rs_ro_ratio)) + 1.553)));
  }
}    
  return 0;
}
