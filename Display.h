/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 */
 
#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include "Nextion.h"
#include "Constants.h"
#include "MeanSmooth.h"


class Display {
	public:
		Display();
		// initialize screen
		void init();

		// startup sequence 
		void start();

		// nexLoop to listen for button events and update values
		void listen();

		// update setting values based on user input
		void updateValues();

		// show alarm
		void showAlarm(const char *buffer, int priority);
		void stopAlarm();

		// update graphs
		void updateFlowWave(float currentFlow);
		void updatePressureWave(float currentPressure);

		// funcitons to write live values to screen
		void writePeak(float peak);
		void writePlateau(float plat);
		void writePeep(float peep);
		void writeVolumeInsp(float Vinsp);
		void writeVolumeExp(float Vexp);
		void writeMinuteVolume(float mv);
		void writeBPM(float bpm);
		void writeO2(int o2);

		// reset inspiratory hold on screen after end of cycle
		void setInspHold() { settings.inspHold = true; }
		void resetInspHold();

		// returns true if user clicked on standby and confirmed 
		bool isTurnedOff() { return turnOff; }

		// getters for user settings
		int oxygen() const { return settings.o2; }
		float sensitivity() const { return settings.sensitivity; }

		// Volume Control only 
		bool inspHold() const { return settings.inspHold; }
		int volume() const { return settings.volume; }
		int bpm() const { return settings.bpm; } 
		unsigned inspPercent() const { return settings.ie[0]*100 / (settings.ie[0] + settings.ie[1]); } 

		// Pressure Support only
		int peak() const { return settings.peak; }
		float cycleOff() const { return settings.cycleOff; }
		float riseTime() const { return settings.riseTime; }

		bool locked;

	private:
		bool turnOff;
		
		struct userSettings {
			int   o2;          // O2 concentration
			float sensitivity; // pressure sensitivity
			int   bpm;         // Respiratory rate 
			int   ie[2]; 			 // I:E ratio 
			int   volume;      // Tidal volume (VC only)
			bool  inspHold;    // inspiratory hold is on (VC only)
			int   peak;        // peak pressure above peep (PS only)
			int   apnea;       // apnea backup time (PS only)
			float cycleOff;    // % peak flow at which we switch to expiration (PS only)
			float riseTime;    // time to peak pressure (PS only)
		} settings;

		// hold button
		NexButton hold = NexButton( 6, 4, "b2" );

		// switch
		NexButton lock = NexButton( 6, 63, "sw0");

		// waveform
		NexWaveform flowWave = NexWaveform( 6, 12, "s0" );
		NexWaveform pressureWave = NexWaveform( 6, 37, "s1" );

		// Alarm stuff
		NexText banner = NexText( 6, 1, "t1");
		NexButton bell = NexButton( 6, 67, "b6");

		// settings
		NexText VTText  = NexText( 6, 68, "t40" );
		NexText RRText  = NexText( 6, 69, "t42" );
		NexText O2Text  = NexText( 6, 70, "t44" );
		NexText IEText  = NexText( 6, 71, "t3" );
		NexText SenText = NexText( 6, 72, "t46" );

		// patient data
		NexText pip  = NexText( 6, 39, "t12" );
		NexText plat = NexText( 6, 40, "t13" );
		NexText peep = NexText( 6, 41, "t14" );
		NexText VTi  = NexText( 6, 42, "t16" );
		NexText VTe  = NexText( 6, 44, "t18" );
		NexText mv   = NexText( 6, 45, "t19" );
		NexText rr   = NexText( 6, 46, "t28" );
		NexText o2   = NexText( 6, 43, "t17" );

		NexTouch *nex_listen_list[3];

		MeanSmooth flowSmoother;
    MeanSmooth pressureSmoother;
};

void holdPopCallback(void *ptr);
void lockPopCallback(void *ptr);
void bellPushCallback(void *ptr);

extern Display display;

#endif
