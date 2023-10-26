#include <Arduino.h>
#include <DS3231-RTC.h>

void showTimeFormated(time_t t) {
#if defined (__AVR__)
    t -= 946684800UL;
#endif
  char buffer[50];
  struct tm *ptm;
  ptm = gmtime(&t);
  const char * timeformat {"%a %F %X - weekday %w; CW %W"};
  strftime(buffer, sizeof(buffer), timeformat, ptm);
  Serial.print(buffer);
  Serial.print("\n");
}

constexpr time_t tstmp {1660644000UL};

RTClib myRTC;
DS3231 Clock;

void setup () {
    Serial.begin(115200);
    while(!Serial){
        yield();
    }
    Serial.println("\n\n\nDS3231 - DateTime Constructor Test()\n");

#if defined (__AVR__)
#warning using AVR platform
    Serial.println("\n\nAVR Microcontroller Ready!\n\n");
    Wire.begin();

#elif defined (__SAMD21G18A__)
#warning using SAMD21 platform
    Serial.println("\n\nSAMD21 Microcontroller Ready!\n\n");
    Wire.begin();

#elif defined (ESP8266)
#warning using espressif platform
    Serial.println("\n\nESP8266 Microcontroller Ready!\n\n");
    // SDA = 0, SCL = 2
    Wire.begin(4U, 5U);
#endif

    // set the Ds3131 with a specific UnixTimestamp
    // ==>    Tue Aug 16 2022 10:00:00 GMT+0000 - weekday 2 (0 = Sunday); CW 33; yearday 227
    // ==>    1660644000
    
    Serial.println("Input Data:");
    Serial.println("\tTue Aug 16 2022 10:00:00 GMT+0000 - weekday 2 (0 = Sunday); CW 33, yearday 227");
    Serial.print("\tUnixTimestamp - ");
    Serial.println(tstmp);

    // feed UnixTimeStamp and don' t use localtime
    Clock.setEpoch(tstmp, false);
    // set to 24h
    Clock.setClockMode(false);

    // Just for verification of DS3231 Data
    // check now the data from ESP8266 and DS3231
    // get year
    bool century = false;
    bool h12Flag;
    bool pmFlag;
    
    // hole die Werte direkt aus der Uhr(DS3231)
    Serial.print("\n\n");
    Serial.println("Get Data from DS3231 clock:");
    Serial.print("\tDateTime of DS3231:     ");
    Serial.print(Clock.getYear(), DEC);
    Serial.print("-");
    Serial.print(Clock.getMonth(century), DEC);
    Serial.print("-");
    Serial.print(Clock.getDate(), DEC);
    Serial.print(" ");
    Serial.print(Clock.getHour(h12Flag, pmFlag), DEC);
    Serial.print(":");
    Serial.print(Clock.getMinute(), DEC);
    Serial.print(":");
    Serial.print(Clock.getSecond(), DEC);
    Serial.print("  -  weekday ");
    Serial.print(Clock.getDoW(), DEC);
    Serial.println();

    Serial.flush();

    // Hole die Zeit aus der DateTime Class
    //auto tic{micros()};
    DateTime datetime = myRTC.now();
    //auto toc {micros()};
    //Serial.print("\n\nDateTime Class instatiation duration: ");
    //Serial.print(toc-tic);
    //Serial.println(" µs");

    Serial.print("\n\n");
    Serial.println("Print data via myRTC.now() constructor");
    Serial.println("Get Data of Struct tm");
    Serial.print("\tDateTime of RTC:        ");
    Serial.print(datetime.getYear(), DEC);
    Serial.print("-");
    Serial.print(datetime.getMonth(), DEC);
    Serial.print("-");
    Serial.print(datetime.getDay(), DEC);
    Serial.print(" ");
    Serial.print(datetime.getHour(), DEC);
    Serial.print(":");
    Serial.print(datetime.getMinute(), DEC);
    Serial.print(":");
    Serial.print(datetime.getSecond(), DEC);
    Serial.print("  -  weekday ");
    Serial.print(datetime.getWeekDay(), DEC);
    Serial.print("  -  yearday ");
    Serial.print(datetime.getYearDay(), DEC);
    Serial.println();
    Serial.print("\tUnixtime: ");
    Serial.println(datetime.getUnixTime());
    Serial.print("\tY2k-Time: ");
    Serial.println(datetime.getY2kTime());


    Serial.print("\n\nPrint Data via function <showTimeFormated> of Struct tm:\n\t");
    showTimeFormated(tstmp);

    Serial.print("\nPrint Data via DataTime function <show_DateTime>:\n\t");
    char buffer[80];
    datetime.show_DateTime(buffer, sizeof(buffer));
    Serial.println(buffer);

    //Serial.print("\nPrint __DATE__ and __TIME__:\n");
    //Serial.print(__DATE__);
    //Serial.print(" ");
    //Serial.println(__TIME__);

    //datetime = DateTime(__DATE__, __TIME__);
    datetime = DateTime("Aug 16 2022", "10:00:00");
    Serial.print("\n\n");
    Serial.println("Print data via __Date, __TIME__ constructor");
    Serial.print("Data of Struct tm\n");
    Serial.print("\tDateTime of RTC:        ");
    Serial.print(datetime.getYear(), DEC);
    Serial.print("-");
    Serial.print(datetime.getMonth(), DEC);
    Serial.print("-");
    Serial.print(datetime.getDay(), DEC);
    Serial.print(" ");
    Serial.print(datetime.getHour(), DEC);
    Serial.print(":");
    Serial.print(datetime.getMinute(), DEC);
    Serial.print(":");
    Serial.print(datetime.getSecond(), DEC);
    Serial.print("  -  weekday ");
    Serial.print(datetime.getWeekDay(), DEC);
    Serial.print("  -  yearday ");
    Serial.print(datetime.getYearDay(), DEC);
    Serial.println();
    Serial.print("\tUnixtime: ");
    Serial.println(datetime.getUnixTime());
    Serial.print("\tY2k-Time: ");
    Serial.println(datetime.getY2kTime());

}

void loop () {
}