#pragma once
class OscillatorGenerator
{
public:
	OscillatorGenerator();
	~OscillatorGenerator();

	enum Waveform { sine, saw, tri, square };
	enum Mode { normal, band_limit };
	enum Polarity { bipolar, unipolar };

	void setFrequency(float frequency);
	void setWaveform(Waveform wf);
	void setMode(Mode m);
	void setPolarity(Polarity p); 
	void setSampleRate(float sr);
	void generate(double* pYn, double* pYqn);
	void reset();

private:
	void cookFrequency();

	Waveform selectedWaveform = sine; // default sine wave
	Mode selectedMode = normal; 
	Polarity selectedPolarity = bipolar; 

	float m_SinArray[1024]; // 1024 point sine wave
	float m_SawtoothArray[1024]; // saw
	float m_TriangleArray[1024]; // triangle
	float m_SquareArray[1024]; // square

	// band limited to 5 partials 
	float m_SawtoothArray_BL5[1024];
	float m_TriangleArray_BL5[1024];
	float m_SquareArray_BL5[1024]; 

	float m_fReadIndex; // current read location 
	float m_fQuadPhaseReadIndex;
	float m_f_inc; // incremental value
	float m_fFrequency_Hz = 440.0; // the frequency 
	float m_nSampleRate = 44100;

	bool m_bInvert; 
};

