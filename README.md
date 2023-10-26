# DS3231-RTC Library
The **great** C++ Library for the DS3231 real-time clock (RTC) module.  

## Description
The library provides easy-to-use methods to:

* set and read the current date and time;
* set, clear, and detect two, independent, Time-of-Day alarms;
* perform certain conversions and calculations with time data;
* manage certain hardware inside the DS3231 RTC module.

This document explains the installation and usage of the Library with the Arduino IDE. 

You do have to install the Library in your Arduino IDE environment before you can use it. Installation instructions are provided, below.

**REMARK**:<br>
This library was based on the master branch of [NorthernWidget/DS3231](https://github.com/NorthernWidget/DS3231) Library in Oct/2023. It was reworked and refractured with respect of the following main topics:
* using standardized functions of the `time.h` library.
* introduce a `struct tm` which holds all relevant date and time values.
* restrucure comments, so that syntax highlighting works fine.
* add a `show_DateTime()` function with can be used to print a user specific(self defined) DateTime string easily.

## Contents

* [Summary](#summary)
* [About the DS3231](#about-the-ds3231-module)
    * [The DS3231 Battery Problem](#the-ds3231-battery-problem)
* [How to Install the Library](#installation)
* [Functions Provided in the Library](#functions)
* [Examples of Using the Library](#examples-of-use)
* [Helpful Resources](#additional-resources-and-references)
* [Contributing, Credits and License](#contributing)
* [To-Do List](#to-do)


<hr>

## Summary

After installing the Library in your Arduino IDE, using it in a program starts with three, simple steps:

<ol start="1"> 
  <li>Import the Library into the program code:</li>
</ol>


```
#include <DS3231-RTC.h>
```

<ol start="2">
  <li>Declare a DS3231 object, for example:</li>
</ol>


```
DS3231 myRTC;
```

<ol start="3">
  <li>Start the Wire library to enable I2C communications with the DS3231 hardware, typically in the setup() code block:</li>
</ol>


```
Wire.begin();
```
or for the **ESP8266** like:
```
Wire.begin(SDA, SCL);
```

Then, Library functions are typically accessed through the DS3231 object.

For example, to read the current date of the month (1...31), depending on the month and the year:

```
byte theDate = myRTC.getDate();
```

The Library incorporates two other classes to assist with managing `date` and `time` data:

* `DateTime` class enables a object for managing date and time data.
* `RTClib` class institutes a convenient `RTClib::now()` function for receiving a date/time snapshot, as a DateTime object, from the DS3231 device.

The `DateTime` class can be instanciated by a specific date and time in three different ways:

* 1.) by distinct values for:<br>
year, month, day, hour, minute and second

    or
    
* 2.) by a single, `time_t` unix timestamp.<br>

* 3.) by giving a separate `const char *` string for Date and Time like:<br>
`"Feb 16 2022"` and `"14:05:00"`<br>
This can be also achived by usage on the precomoiler Tags `__DATE__` and `__TIME__`.



## About the DS3231 Module
The DS3231 module is a low-cost integrated circuit (IC) providing a highly accurate, real time clock for use with Arduino, ESP8266, ESP32, Raspberry Pi and other popular small computing devices. 

The IC is typically mounted on a circuit board, along with other hardware, such as header pins, supportive electrical components, and even EEPROM memory chips, for convenient attachment to a breadboard or an Arduino. 

Several different modules are available from a number of competing vendors. This Library aspires to work with any DS3231 module that supports I2C communications with the IC.

DS3231 runs independently and can be kept running for a considerable length of time by a small, backup battery, even if power to the Arduino is turned off.

According to the [DS3231-datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231-DS3231S.pdf), the DS3231 hardware "completely manages all timekeeping functions (including):

* Seconds, 
* Minutes, 
* Hours
    * 12-hour format with AM/PM indication, or
    * 24-hour format,
* Day of the Week,
* Date of the Month, 
* Month,  and
* Year, with Leap-Year Compensation Valid Up to 2100"

Data for the time and date are stored in registers on the DS3231. Each, distinct value is stored separately. This means the seconds are in one register, the minutes in another, and so on. The DS3231 updates the values in the date and time registers every second.

The device keeps track of time by operating its own 32.768 kHz crystal oscillator, similar to the timekeeper in an electronic watch. Temperature can affect oscillator speed. Accordingly, the DS3231 takes further steps to maintain accuracy. It senses the temperature around the crystal and adjusts the speed of the oscillator.<br>
The oscillator can be accessed directly, independent of the date and time registers, for use as an external timer or source of interrupts.

The temperature can be read from the DS3231 using a class member function. The data sheet declares it to be accurate to within 3 degrees, Celsius. 


### Power Supply and Battery Backup
The DS3231 can run in a range between 2.3 V and 5.5 V. The device actually has two power supply pins:<br>
* the primary source V<sub>CC</sub>
* a secondary, backup source V<sub>BAT</sub>

Some popular DS3231 modules mounting, provide a receptacle for a coin battery (CR2032), attaching it to the V<sub>BAT</sub> pin. If a sufficiently-charged battery is present, the DS3231 will switch automatically to the battery after detecting a drop in V<sub>CC</sub> voltage below a certain "power-fail" level.

It will switch back to V<sub>CC</sub> automatically, if and when that voltage rises back up above both the power-fail and the battery voltage level. 

**REMARK**:<br>
One point regarding the choice of battery may deserve consideration:<br>
The question of whether to install a **rechargeable** coin battery, or to disable the charging circuit if a **nonrechargeable** battery provided on the module being used.<br>


#### The DS3231 Battery Problem:<br>
It is highly recommended to check which battery/accumulator is placed on the DS3231 module!<br>
If the module is connected to 5V it normally charges the accumulator, and when a normal battery is used this can cause issues.<br>
Like:
* battery look inflated
* battery can swell

For the use with a battery e.g.: `CR2032`, please desolder the **200 Ohm resistor** to deactivate the charging mechanism.<br>
See the corresponding link for the problem description in detail:<br>
[DS3231-battery-problem](https://320volt.com/en/ds3231-cr2032-battery-problem-and-solution/)

[back to top](#ds3231-rtc-library)
<hr>


## Installation

### First Method

1. In the Arduino IDE, navigate to Sketch > Include Library > Manage Libraries
1. Then the Library Manager will open and you will find a list of libraries that are already installed or ready for installation.
1. Then search for DS3231 using the search bar.
1. Click on the text area and then select the specific version and install it.

### Second Method

1. Navigate to the [Releases page](https://github.com/hasenradball/DS3231-RTC/releases).
1. Download the latest release.
1. Extract the zip file
1. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library

### Dependencies

The user must also ensure that two, other, required libraries are available to the Arduino IDE. This DS3231 library takes care to `#include`  the following in a program, but it does not install them in your Arduino IDE:

* `Wire.h` : a widely-used Arduino library for I2C twowire communications
* `time.h` : a modified C language header file to the corresponding microcontroller based time lib functions

**Remark**:<br>
the AVR lib `time.h` is based on year 2000 against the c-standard libs are based on year 1970.<br>
See [AVR-libc Time](https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html)

Note:<br>
At the time of writing, both of these libraries were included by default with a standard installation of the 1.8.x version of Arduino IDE for AVR-based devices.

A simple way to check for the availability of the two libraries is to compile the following, blank Arduino sketch. If the IDE does not complain that anything is missing, then the required libraries are available for use with this DS3231 library.

```
#include <Wire.h>
#include <time.h>
void setup() {}
void loop() {}
```

[back to top](#ds3231-rtc-library)
<hr>

## Functions
Readers are encouraged to visit the [Documentation folder](https://github.com/hasenradball/DS3231-RTC/tree/master/Documentation) for detailed information about the functions in this Library. Additional information is available in the [Examples of Use](#examples-of-use) described below, and in the code source files of this repository:

* [DS3231-RTC.h](https://github.com/hasenradball/DS3231-RTC/blob/master/src/DS3231-RTC.h)
* [DS3231-RTC.cpp](https://github.com/hasenradball/DS3231-RTC/blob/master/src/DS3231-RTC.cpp)

### [Read the Date or Time](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md)
- [RTClib::now() <sup>\*</sup>](https://github.com/hasenradball/DS3231-RTC#the-special-rtclibnow-function-)
- [getSecond()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getsecond)
- [getMinute()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getminute)
- [getHour(bool, bool)](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#gethour)
- [getDoW()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getdow)
- [getDate()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getdate)
- [getMonth(bool)](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getmonth)
- [getYear()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Retrieval.md#getyear)

\* The *RTClib::now()* function is not accessed through the DS3231 object. Rather, it has a very specific syntax as described below in <a href="#RTClib_now_function">The Special RTClib::now() Function</a>.


### [Set the Date or Time](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md)
We emphasize here and elsewhere that the code writer bears responsibility to ensure that the values passed into the following functions fall within the valid range, as specified in the documentation for each function.

Unexpected values in the DS3231 hardware registers may follow from the insertion of an invalid parameter into any one of these functions.

- [setEpoch()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setepochtime_t-epoch--0-bool-flag_localtime--false)
- [setSecond()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setsecondbyte-second)
- [setMinute()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setminutebyte-minute)
- [setHour()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-sethourbyte-hour)
- [setDoW()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setdowbyte-dow)
- [setDate()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setdatebyte-date)
- [setMonth()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setmonthbyte-month)
- [setYear()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setyearbyte-year)
- [setClockMode()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Time-Set.md#void-setclockmodebool-h12)

### [Set, Clear and Check Alarms](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md)
The following functions set and retrieve time and date values in the DS3231 hardware alarm registers. 

Parameters include a special 8-bit value named "AlarmBits". Readers may find additional information about it at the following links: [Alarm Bits Quick Reference](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#alarm-bits-quick-reference), and [Alarm Bits in Detail](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#alarm-bits-in-detail).

- [getA1Time()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#geta1time)
- [getA1Time() with Option](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#geta1time-with-option)
- [getA2Time()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#geta2time)
- [getA2Time() with Option](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#geta2time-with-option)
- [setA1Time()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#seta1time)
- [setA2Time()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#seta2time)

The remaining functions in this group set and retrieve certain flags in the DS3231 hardware that govern or report the operation of the alarms.

- [turnOnAlarm()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#turnonalarm)
- [turnOffAlarm()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#turnoffalarm)
- [checkAlarmEnabled()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#checkalarmenabled)
- [checkIfAlarm()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#checkifalarm)
- [checkIfAlarm() with Option](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Alarms.md#checkifalarm-with-option)

### [Manage DS3231 Hardware](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Utilities.md)
The functions in this group support uses for a DS3231 other than as an alarm clock.

- [getTemperature()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Utilities.md#gettemperature)
- [enableOscillator()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Utilities.md#enableoscillator)
- [enable32kHz()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Utilities.md#enable32khz)
- [oscillatorCheck()](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/Utilities.md#oscillatorcheck)

### [DateTime Object](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/DateTime.md)
A DateTime class is defined in this DS3231-RTC.h library. The link, above, provides more information about the class. 

[Retrieving Date and Time Data](https://github.com/hasenradball/DS3231-RTC/blob/master/Documentation/DateTime.md#retrieving-date-and-time-data)<br>
further lists the DateTime class methods (for Documentation look into the src file): 

- getYear()
- getMonth()
- getDay()
- getHour()
- getMinute()
- getSecond()
- getWeekDay()
- getDST()
- show_DateTime()
- getUnixTime()
- getY2KTime()

<h3 id="RTClib_now_function">The <code>RTClib::now()</code> Function </h3>

`RTClib::now()` is the precise, complete name for a special function that returns a `DateTime` object from the DS3231. Always write it just so: `RTClib::now()`.

The function returns a DateTime object. To use it in your program, declare a DateTime type of variable to receive the value. For example: 

`DateTime currentMoment = RTClib::now();`

The value of `currentMoment` can then be accessed as either:
* getting an unsigned integer containing the number of seconds since a certain reference date (Unix-Time or Y2K-Time)
* distinct values out of the `struct tm` for Year, Month, Day, Date, Hour, Minute, Second etc...<br>
see [Definiton of struct tm](https://en.cppreference.com/w/c/chrono/tm).

[back to the list of functions](#functions)<br>
[back to top](#ds3231-rtc-library)
<hr>

## Examples of Use

There are many examples provided in the [examples](https://github.com/hasenradball/DS3231-RTC/tree/master/examples) folder of this repository.<br>
At the time of writing the examples include:

* `set`: demonstrates selected time-setting functions
* `test`: demonstrates selected time-reading functions
* `echo`: demonstrates setting the time and date then reading it back
* `echo_time`: similar to *echo*, demonstrates setting and reading time/date data
* `oscillator_test`: demonstrates advanced techniques for managing and using the DS3231 device as a pulse generator

Future development plans include updating these examples and adding more of them.

See also [Working with the DS3231 libraries and interrupts](https://github.com/IowaDave/RTC-DS3231-Arduino-Interrupt), a tutorial provided by [IowaDave](https://github.com/IowaDave).

[back to top](#ds3231-rtc-library)
<hr>

## Additional Resources and References

* [Maxim DS3231 Data Sheet](https://datasheets.maximintegrated.com/en/ds/DS3231-DS3231S.pdf)

[back to top](#ds3231-rtc-library)
<hr>

## Contributing

If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell others about this library
- Contribute new protocols

Please read [CONTRIBUTING.md](https://github.com/hasenradball/DS3231-RTC/blob/master/CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

[back to top](#ds3231-rtc-library)
<hr>

## Credits
This library is in the first shot based on [A. Wickerts'](https://github.com/NorthernWidget/DS3231) library and then refractured.

The author of this library is F. Häfele <mail@frankhaefele.de>

Based on previous work by:

- A. Wickert

[back to top](#ds3231-rtc-library)
<hr>

## License

DS3231 is licensed under [MIT License](https://github.com/hasenradball/DS3231-RTC/blob/master/LICENSE).

[back to top](#ds3231-rtc-library)
<hr>

## To Do
A project is underway to update the library's documentation.

