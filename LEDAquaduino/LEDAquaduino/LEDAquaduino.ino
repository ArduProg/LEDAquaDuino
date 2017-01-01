
#include "Thermistor.h"
#include "Time.h"
#include <DHT.h>
#include <Event.h>
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>
#include "LEDFader.h"
#include "SimpleTimer.h"
#include "Timer.h"
#include <math.h>
#include <SoftwareSerial.h>

//#define slowpwm
//#define BT

#ifndef BT
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#define EspSerial Serial
#define ESP8266_BAUD 9600
ESP8266 wifi(&EspSerial);
#endif // !BT
#ifdef BT
#include <BlynkSimpleStream.h>
#endif
//#define TEST
//#define TIMESET
//#define DEMO
/*////////////////////////////////////////////////////////////////////////////*/

#define auth "xxx"
#define wifiauth "xxx"
#define ssid "xxx"
#define pass "xxx"

/*////////////////////////////////////////////////////////////////////////////*/
long SunriseStart = 28800;				//When sunrise mode starts(hour) 8:00

long  DayStart = 36000;					//When day mode starts(hour) 10:00

long SunsetStart = 64800;				//When sunset mode starts(hour) 18:00

long  NightStart = 72000;				//When night mode starts(hour) 20:00

long  OFFstart = 82800;				//When to shut down all acc(hour) 23:00

byte fadeM = 10;
long fadetime = 60000;
bool sended = false;//Fade time(from 0 to max)(ms)
byte Rmor = 255;
byte Gmor = 255;
byte Wmor = 100;
byte BWmor = 0;
byte Bmor = 0;
byte Gday = 100;
byte Wday = 255;
byte Rday = 255;
byte BWday = 220;
byte Bday = 0;
byte Geve = 0;
byte Weve = 200;
byte BWeve = 180;
byte Reve = 255;
byte Beve = 0;
byte Rnig = 100;
byte Gnig = 136;
byte Bnig = 255;
byte Wnig = 50;
byte BWnig = 100;
byte currentmode;
byte power;
byte hour1;
byte minute1;
SimpleTimer t;
#define Rp 5		//Red channelPWM pin
#define Gp 6		//Green channel PWM pin
#define Bp 9		// Blue channel PWM pin
#define Wp 10		//White channel PWM pin
#define BWp 11		//Powerled blue PWM pin
DHT dht(2, 22);
RTC_DS1307 rtc;
byte read = 0;
byte send = 0;
LEDFader BW(BWp);
LEDFader W(Wp);
LEDFader R(Rp);
LEDFader G(Gp);
LEDFader B(Bp);
Thermistor temp(A0);
bool stopped;
long timeg;
long secnow = 0;
void EEPROMsetup() {
	Wmor = EEPROM.read(0);
	Wday = EEPROM.read(1);
	Weve = EEPROM.read(2);
	Wnig = EEPROM.read(3);
	BWmor = EEPROM.read(4);
	BWday = EEPROM.read(5);
	BWeve = EEPROM.read(6);
	BWnig = EEPROM.read(7);
	Rmor = EEPROM.read(8);
	Rday = EEPROM.read(9);
	Reve = EEPROM.read(10);
	Rnig = EEPROM.read(11);
	Gmor = EEPROM.read(12);
	Gday = EEPROM.read(13);
	Geve = EEPROM.read(14);
	Gnig = EEPROM.read(15);
	Bmor = EEPROM.read(16);
	Bday = EEPROM.read(17);
	Beve = EEPROM.read(18);
	Bnig = EEPROM.read(19);
	EEPROM.get(20, SunriseStart);
	EEPROM.get(24, DayStart);
	EEPROM.get(28, SunsetStart);
	EEPROM.get(32, NightStart);
	EEPROM.get(36, OFFstart);
	EEPROM.get(40, fadetime);
}
void push()
{
#ifndef TEST
	DateTime now = rtc.now();
#endif
	bool high = false;
	bool low = false;
	double watertemp = temp.getTemp();
	byte airtemp = dht.readTemperature();
	byte humidity = dht.readHumidity();
#ifndef BT
	if (watertemp > 35 && high == false) {
		Blynk.email("AquaDuino Temp Warning", "Water temperature is too high(>35°C)");
		high = true;
	}
	if (watertemp < 35 && high == true) {
		high = false;
	}
	if (watertemp <22 && low == false) {
		Blynk.email("AquaDuino Temp Warning", "Water temperature is too low(<22°C)");
		low = true;
	}
	if (watertemp >22 && low == true) {
		low = false;
	}
#endif
	Blynk.virtualWrite(V4, watertemp);
	Blynk.virtualWrite(V5, airtemp);
	Blynk.virtualWrite(V6, humidity);

#ifndef TEST
	Blynk.virtualWrite(V14, now.hour());
	Blynk.virtualWrite(V15, now.minute());
	byte MM = now.minute();
	byte HH = now.hour();
	int min = MM * 60;
	long hod = HH * 3600L;
	secnow = hod + min;
#endif
}

////////////////////////////////////
/////////////////////////////////////
////////////////////////////////////

void setup() {
	dht.begin();
	Serial.begin(9600);
	delay(10);
	t.setInterval(9000L, push);
#ifdef BT
	Blynk.begin(auth, Serial);
#endif
#ifndef BT 
	Blynk.begin(wifiauth, wifi, ssid, pass, "arduprog.ddns.net");
#endif // !BT 

#ifdef slowpwm
	pinMode(11, OUTPUT);
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS22);
	OCR2A = 180;
	OCR2B = 50;
	bitSet(TCCR1B, WGM12);

#endif
	rtc.begin();
#ifndef TEST
	DateTime now = rtc.now();
#endif
	power = 1;
	digitalWrite(BWp, LOW);
	digitalWrite(Wp, LOW);
	digitalWrite(Rp, LOW);
	digitalWrite(Gp, LOW);
	digitalWrite(Bp, LOW);
#ifdef TIMESET
	if (!rtc.isrunning()) {
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
#endif
}
void loop() {

	BW.update();
	B.update();
	R.update();
	G.update();
	W.update();
	Blynk.run();
	t.run();
	/**********************************************
	******************Misc. things****************
	**********************************************/

	/****************************************************/
	/****************************************************
	*****************Light section**********************
	****************************************************/
#ifndef TEST 
#ifndef DEMO

	if (power == 1) {
		stopped = false;
		if (secnow >= SunriseStart && secnow < DayStart) {
			currentmode = 1;
			if (BW.is_fading() == false) {
				BW.fade(BWmor, fadetime);
			}
			if (W.is_fading() == false) {
				W.fade(Wmor, fadetime);
			}
			if (R.is_fading() == false) {
				R.fade(Rmor, fadetime);
			}
			if (G.is_fading() == false) {
				G.fade(Gmor, fadetime);
			}
			if (B.is_fading() == false) {
				B.fade(Bmor, fadetime);
			}
		}
		if (secnow >= DayStart && secnow < SunsetStart) {
			currentmode = 2;
			if (BW.is_fading() == false) {
				BW.fade(BWday, fadetime);
			}
			if (W.is_fading() == false) {
				W.fade(Wday, fadetime);
			}
			if (R.is_fading() == false) {
				R.fade(Rday, fadetime);
			}
			if (G.is_fading() == false) {
				G.fade(Gday, fadetime);
			}
			if (B.is_fading() == false) {
				B.fade(Bday, fadetime);
			}
		}
		if (secnow >= SunsetStart && secnow < NightStart) {
			currentmode = 3;
			if (BW.is_fading() == false) {
				BW.fade(BWeve, fadetime);
			}
			if (W.is_fading() == false) {
				W.fade(Weve, fadetime);
			}
			if (R.is_fading() == false) {
				R.fade(Reve, fadetime);
			}
			if (G.is_fading() == false) {
				G.fade(Geve, fadetime);
			}
			if (B.is_fading() == false) {
				B.fade(Beve, fadetime);
			}
		}
		if (secnow >= NightStart && secnow < OFFstart) {
			currentmode = 4;
			if (BW.is_fading() == false) {
				BW.fade(BWnig, fadetime);
			}
			if (W.is_fading() == false) {
				W.fade(Wnig, fadetime);
			}
			if (R.is_fading() == false) {
				R.fade(Rnig, fadetime);
			}
			if (G.is_fading() == false) {
				G.fade(Gnig, fadetime);
			}
			if (B.is_fading() == false) {
				B.fade(Bnig, fadetime);
			}
		}
		if (secnow >= OFFstart || secnow < SunriseStart) {
			currentmode = 5;
			if (BW.is_fading() == false) {
				BW.fade(0, fadetime);
			}
			if (W.is_fading() == false) {
				W.fade(0, fadetime);
			}
			if (R.is_fading() == false) {
				R.fade(0, fadetime);
			}
			if (G.is_fading() == false) {
				G.fade(0, fadetime);
			}
			if (B.is_fading() == false) {
				B.fade(0, fadetime);
			}
		}
	}
	if (power == 0) {
		if (stopped == false) {
			BW.stop_fade();
			W.stop_fade();
			R.stop_fade();
			G.stop_fade();
			B.stop_fade();
			stopped = true;
		}
		if (BW.is_fading() == false) {
			BW.fade(0, 2000);
		}
		if (W.is_fading() == false) {
			W.fade(0, 2000);
		}
		if (R.is_fading() == false) {
			R.fade(0, 2000);
		}
		if (G.is_fading() == false) {
			G.fade(0, 2000);
		}
		if (B.is_fading() == false) {
			B.fade(0, 2000);
		}
	}
#endif
#endif
#ifdef TEST
	if (BW.is_fading() == false) {
		BW.fade(100, fadetime);
	}
	if (W.is_fading() == false) {
		W.fade(100, fadetime);
	}
	if (R.is_fading() == false) {
		R.fade(100, fadetime);
	}
	if (G.is_fading() == false) {
		G.fade(100, fadetime);
	}
	if (B.is_fading() == false) {
		B.fade(100, fadetime);
	}
#endif
}

BLYNK_WRITE(V0) {
	power = param.asInt();
	switch (currentmode) {
	case 1:
		Blynk.virtualWrite(V1, Wmor);
		Blynk.virtualWrite(V2, BWmor);
		Blynk.virtualWrite(V7, Rmor);
		Blynk.virtualWrite(V8, Gmor);
		Blynk.virtualWrite(V9, Bmor);
		break;
	case 2:
		Blynk.virtualWrite(V1, Wday);
		Blynk.virtualWrite(V2, BWday);
		Blynk.virtualWrite(V7, Rday);
		Blynk.virtualWrite(V8, Gday);
		Blynk.virtualWrite(V9, Bday);
		break;
	case 3:
		Blynk.virtualWrite(V1, Weve);
		Blynk.virtualWrite(V2, BWeve);
		Blynk.virtualWrite(V7, Reve);
		Blynk.virtualWrite(V8, Geve);
		Blynk.virtualWrite(V9, Beve);
		break;
	case 4:
		Blynk.virtualWrite(V1, Wnig);
		Blynk.virtualWrite(V2, BWnig);
		Blynk.virtualWrite(V7, Rnig);
		Blynk.virtualWrite(V8, Gnig);
		Blynk.virtualWrite(V9, Bnig);
		break;
	case 5:
		Blynk.virtualWrite(V1, 0);
		Blynk.virtualWrite(V2, 0);
		Blynk.virtualWrite(V7, 0);
		Blynk.virtualWrite(V8, 0);
		Blynk.virtualWrite(V9, 0);
		//power == 0;
		break;
	}
	Blynk.virtualWrite(V16, fadetime / 6000);
	Blynk.virtualWrite(V0, power);
}
BLYNK_WRITE(V1) {
	W.stop_fade();
	switch (currentmode) {
	case 1:
		Wmor = param.asInt();
		break;
	case 2:
		Wday = param.asInt();
		break;
	case 3:
		Weve = param.asInt();
		break;
	case 4:
		Wnig = param.asInt();
		break;
	}
}
BLYNK_WRITE(V2) {
	BW.stop_fade();
	switch (currentmode) {
	case 1:
		BWmor = param.asInt();
		break;
	case 2:
		BWday = param.asInt();
		break;
	case 3:
		BWeve = param.asInt();
		break;
	case 4:
		BWnig = param.asInt();
		break;
	}
}
BLYNK_WRITE(V3) {
	timeg = param.asLong();
	hour1 = timeg / 3600;
	minute1 = (timeg / 60) - (hour1 * 60);
	rtc.adjust(DateTime(2016, 4, 11, hour1, minute1, 0));
}
BLYNK_WRITE(V7) {
	R.stop_fade();
	switch (currentmode) {
	case 1:
		Rmor = param.asInt();
		break;
	case 2:
		Rday = param.asInt();
		break;
	case 3:
		Reve = param.asInt();
		break;
	case 4:
		Rnig = param.asInt();
		break;
	}
}
BLYNK_WRITE(V8) {
	G.stop_fade();
	switch (currentmode) {
	case 1:
		Gmor = param.asInt();
		break;
	case 2:
		Gday = param.asInt();
		break;
	case 3:
		Geve = param.asInt();
		break;
	case 4:
		Gnig = param.asInt();
		break;
	}
}
BLYNK_WRITE(V9) {
	B.stop_fade();
	switch (currentmode) {
	case 1:
		Bmor = param.asInt();
		break;
	case 2:
		Bday = param.asInt();
		break;
	case 3:
		Beve = param.asInt();
		break;
	case 4:
		Bnig = param.asInt();
		break;
	}
}
BLYNK_WRITE(V10) {
	SunriseStart = param.asLong();
}
BLYNK_WRITE(V11) {
	DayStart = param.asLong();
}
BLYNK_WRITE(V12) {
	send = param.asInt();
	if (send == 1) {
		EEPROM.update(0, Wmor);
		EEPROM.update(1, Wday);
		EEPROM.update(2, Weve);
		EEPROM.update(3, Wnig);
		EEPROM.update(4, BWmor);
		EEPROM.update(5, BWday);
		EEPROM.update(6, BWeve);
		EEPROM.update(7, BWnig);
		EEPROM.update(8, Rmor);
		EEPROM.update(9, Rday);
		EEPROM.update(10, Reve);
		EEPROM.update(11, Rnig);
		EEPROM.update(12, Gmor);
		EEPROM.update(13, Gday);
		EEPROM.update(14, Geve);
		EEPROM.update(15, Gnig);
		EEPROM.update(16, Bmor);
		EEPROM.update(17, Bday);
		EEPROM.update(18, Beve);
		EEPROM.update(19, Bnig);
		EEPROM.put(20, SunriseStart);
		EEPROM.put(24, DayStart);
		EEPROM.put(28, SunsetStart);
		EEPROM.put(32, NightStart);
		EEPROM.put(36, OFFstart);
		EEPROM.put(40, fadetime);
		send = 0;
	}
}
BLYNK_WRITE(V13) {
	read = param.asInt();
	if (read == 1) {
		EEPROMsetup();
	}


}
BLYNK_WRITE(V16) {
	fadeM = param.asInt();
	fadetime = fadeM * 6000L;
}
BLYNK_WRITE(V17) {
	switch (currentmode) {
	case 1:
		Blynk.virtualWrite(V1, Wmor);
		Blynk.virtualWrite(V2, BWmor);
		Blynk.virtualWrite(V7, Rmor);
		Blynk.virtualWrite(V8, Gmor);
		Blynk.virtualWrite(V9, Bmor);
		break;
	case 2:
		Blynk.virtualWrite(V1, Wday);
		Blynk.virtualWrite(V2, BWday);
		Blynk.virtualWrite(V7, Rday);
		Blynk.virtualWrite(V8, Gday);
		Blynk.virtualWrite(V9, Bday);
		break;
	case 3:
		Blynk.virtualWrite(V1, Weve);
		Blynk.virtualWrite(V2, BWeve);
		Blynk.virtualWrite(V7, Reve);
		Blynk.virtualWrite(V8, Geve);
		Blynk.virtualWrite(V9, Beve);
		break;
	case 4:
		Blynk.virtualWrite(V1, Wnig);
		Blynk.virtualWrite(V2, BWnig);
		Blynk.virtualWrite(V7, Rnig);
		Blynk.virtualWrite(V8, Gnig);
		Blynk.virtualWrite(V9, Bnig);
		break;
	case 5:
		Blynk.virtualWrite(V1, 0);
		Blynk.virtualWrite(V2, 0);
		Blynk.virtualWrite(V7, 0);
		Blynk.virtualWrite(V8, 0);
		Blynk.virtualWrite(V9, 0);
		//power == 0;
		break;
	}
	Blynk.virtualWrite(V16, fadetime / 6000);
	Blynk.virtualWrite(V0, power);

}
BLYNK_WRITE(V18) {
	SunsetStart = param.asLong();
}
BLYNK_WRITE(V19) {
	NightStart = param.asLong();
}
BLYNK_WRITE(V20) {
	OFFstart = param.asLong();
}
