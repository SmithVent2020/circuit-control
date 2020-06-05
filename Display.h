/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 */
 
#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include "Nextion.h"
#include "Constants.h"


class Display {
  public:
		Display();
		// startup sequence 
		void start();

		// nexLoop to listen for button events and update values
		void loop();

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
		float ie() const { return (double)settings.ie[0]/ settings.ie[1]; } 

		// Pressure Support only
		int peak() const { return settings.peak; }
		float cycleOff() const { return settings.cycleOff; }
		float riseTime() const { return settings.riseTime; }

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

		// waveform
		NexWaveform flowWave = NexWaveform(7, 11, "s0");
		NexWaveform pressureWave = NexWaveform(7, 38, "s1");

		// sliders
		NexSlider VTSlider  = NexSlider( 7, 68, "h0" );
		NexSlider RRSlider  = NexSlider( 7, 69, "h1" );
		NexSlider O2Slider  = NexSlider( 7, 70, "h2" );
		NexSlider IESlider  = NexSlider( 7, 71, "h3" );
		NexSlider SenSlider = NexSlider( 7, 72, "h4" );

		// patient data
		NexText pip  = NexText( 7, 43, "t12" );
		NexText plat = NexText( 7, 44, "t13" );
		NexText peep = NexText( 7, 45, "t14" );
		NexText VTi  = NexText( 7, 46, "t16" );
		NexText VTe  = NexText( 7, 48, "t18" );
		NexText mv   = NexText( 7, 49, "t19" );
		NexText rr   = NexText( 7, 50, "t28" );
		NexText o2   = NexText( 7, 47, "t17" );
};

extern Display display;

#endif
