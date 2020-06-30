#pragma once
class OscillatorGenerator
{
public:
	OscillatorGenerator();
	~OscillatorGenerator();
	enum Waveform { sine, saw, tri, square };
	enum Mode {normal, band_limit };

	void setFrequency(float frequency);
	void setWaveform(Waveform wf);
	void setMode(Mode m);
	void setSampleRate(float sr);
	double generate();
	void reset();


private:
	void cookFrequency();

	Waveform selectedWaveform = sine; // default sine wave
	Mode selectedMode = normal; 

	float m_SinArray[1024]; // 1024 point sine wave
	float m_Sawtootharray[1024]; // saw
	float m_TriangleArray[1024]; // triangle
	float m_SquareArray[1024]; // square

	// band limited to 5 partials 
	float m_SawtoothArray_BL5[1024];
	float m_TriangleArray_BL5[1024];
	float m_SquareArray_BL5[1024]; 

	float m_fReadIndex; // current read location 
	float m_f_inc; // incremental value
	float m_fFrequency_Hz = 440.0; // the frequency 
	float m_nSampleRate = 44100;

};

