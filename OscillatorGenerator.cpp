#include "OscillatorGenerator.h"
#include "IControl.h"
#include "dLinTerp.h"

OscillatorGenerator::OscillatorGenerator()
{
	// slope and y-intercept values for triangle wave
	// rising edge1:
	float mt1 = 1.0 / 256.0;
	float bt1 = 0.0;

	// rising edge 2
	float mt2 = 1.0 / 256.0;
	float bt2 = -1.0;

	// falling edge 
	float mtf2 = -2.0 / 512.0;
	float btf2 = 1.0;

	// sawtooth
	// rising edge1:
	float ms1 = 1.0 / 512.0;
	float bs1 = 0.0;

	// rising edge2:
	float ms2 = 1.0 / 512.0;
	float bs2 = -1.0;


	// arrays to keep track of max-es for normalization later
	float fMaxTri = 0;
	float fMaxSaw = 0;
	float fMaxSqr = 0; 


	for (int i = 0; i < 1024; i++) {
		// sample the sine wave
		// sin(wnT) = sin(2pi*i/1024)
		m_SinArray[i] = sin(((float)i / 1024.0)*(2 * PI));

		// saw
		m_SawtoothArray[i] = i < 512 ? ms1*i + bs1 : ms2*(i - 511) + bs2;

		// triangle
		if (i < 256) {
			m_TriangleArray[i] = mt1*i + bt1;
		}
		else if (i >= 256 && i < 768) {
			m_TriangleArray[i] = mtf2*(i - 256) + btf2;
		}
		else {
			m_TriangleArray[i] = mt2*(i - 768) + bt2;
		}

		// square
		if (i == 1) {
			m_SquareArray[i] = 0.0;
		}
		else {
			m_SquareArray[i] = i < 512 ? +1.0 : -1.0;
		}


		m_SawtoothArray_BL5[i] = 0.0;
		m_SquareArray_BL5[i] = 0.0; 
		m_TriangleArray_BL5[i] = 0.0; 

		// band limited arrays 
		// sawtooth: +=(-1)^g+1(1/g)sin(wnT)
		for (int g = 1; g <= 6; g++) {
			double n = double(g);
			m_SawtoothArray_BL5[i] += pow((float)-1.0, (float)n) *
				(1.0 / pow((float)(2 * n + 1), (float)2.0)) *
				sin(2.0*PI*(2.0*n + 1)*i / 1024.0); 
		}

		// triangle: += (-1)^g(1/(2g+1+^2)sin(w(2n+1)T)
		// limit is 3 because of the way the sum is constructed
		for (int g = 0; g <= 3; g++) {
			double n = double(g);
			m_TriangleArray_BL5[i] += pow((float)-1.0, (float)n) *
				(1.0 / pow((float)(2 * n + 1),
				(float)2.0)) *
				sin(2.0*PI*(2.0*n + 1)* i / 1024.0); 
		}

		// square: += (1/g)sin(wnT)
		for (int g = 1; g <= 5; g += 2) {
			double n = double(g); 
			m_SquareArray_BL5[i] += (1.0 / n)*sin(2.0*PI*i*n / 1024.0);
		}

		// store the max values 
		if (i == 0) {
			fMaxSaw = m_SawtoothArray_BL5[i];
			fMaxTri = m_TriangleArray_BL5[i];
			fMaxSqr = m_SquareArray_BL5[i];
		}
		else {
			if (m_SawtoothArray_BL5[i] > fMaxSaw) fMaxSaw = m_SawtoothArray_BL5[i];
			if (m_TriangleArray_BL5[i] > fMaxTri) fMaxTri = m_TriangleArray_BL5[i];
			if (m_SquareArray_BL5[i] > fMaxSqr) fMaxSqr = m_SquareArray_BL5[i]; 
		}
	}

	// normalize the bandlimited tables to make sure values are between -1.0 and 1.0
	for (int i = 0; i < 1024; i++) {
		m_SawtoothArray_BL5[i] /= fMaxSaw;
		m_TriangleArray_BL5[i] /= fMaxTri; 
		m_SquareArray_BL5[i] /= fMaxSqr;
	}

	// clear var
	m_fReadIndex = 0;
	m_f_inc = 0.0;

	//initialize inc
	cookFrequency();
}

OscillatorGenerator::~OscillatorGenerator()
{
}

void OscillatorGenerator::generate(double* pYn, double* pYqn) {
	float fOutSample = 0; // output value for this cycle
	float fQuadPhaseOutSample = 0; 

	int nReadIndex = (int)m_fReadIndex;
	int nQuadPhaseReadIndex = (int)m_fQuadPhaseReadIndex;

	float fFrac = m_fReadIndex - nReadIndex;

	int nReadIndexNext = nReadIndex + 1 > 1023 ? 0 : nReadIndex + 1; // read index of second index, wrap around buffer if needed
	int nQuadPhaseReadIndexNext = nQuadPhaseReadIndex + 1 > 1023 ? 0 : nQuadPhaseReadIndex + 1; 

	switch (selectedWaveform) {
	case sine:
		fOutSample = dLinInterp::dLinTerp(0, 1, m_SinArray[nReadIndex], m_SinArray[nReadIndexNext], fFrac); // interpolate frac value 
		fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SinArray[nQuadPhaseReadIndex], m_SinArray[nQuadPhaseReadIndexNext], fFrac); 
		break;
	case saw:
		if (selectedMode == normal) {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_SawtoothArray[nReadIndex], m_SawtoothArray[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SawtoothArray[nQuadPhaseReadIndex], m_SawtoothArray[nQuadPhaseReadIndexNext], fFrac);
		}
		else {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_SawtoothArray_BL5[nReadIndex], m_SawtoothArray_BL5[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SawtoothArray_BL5[nQuadPhaseReadIndex], m_SawtoothArray_BL5[nQuadPhaseReadIndexNext], fFrac);
		}
		break;
	case tri:
		if (selectedMode == normal) {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_TriangleArray[nReadIndex], m_TriangleArray[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_TriangleArray[nQuadPhaseReadIndex], m_TriangleArray[nQuadPhaseReadIndexNext], fFrac);
		}
		else {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_TriangleArray_BL5[nReadIndex], m_TriangleArray_BL5[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_TriangleArray_BL5[nQuadPhaseReadIndex], m_TriangleArray_BL5[nQuadPhaseReadIndexNext], fFrac);
		}
		break;
	case square:
		if (selectedMode == normal) {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_SquareArray[nReadIndex], m_SquareArray[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SquareArray[nQuadPhaseReadIndex], m_SquareArray[nQuadPhaseReadIndexNext], fFrac);
		}
		else {
			fOutSample = dLinInterp::dLinTerp(0, 1, m_SquareArray_BL5[nReadIndex], m_SquareArray_BL5[nReadIndexNext], fFrac);
			fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SquareArray_BL5[nQuadPhaseReadIndex], m_SquareArray_BL5[nQuadPhaseReadIndexNext], fFrac);
		}
		break;
	default:
		fOutSample = dLinInterp::dLinTerp(0, 1, m_SinArray[nReadIndex], m_SinArray[nReadIndexNext], fFrac);
		fQuadPhaseOutSample = dLinInterp::dLinTerp(0, 1, m_SinArray[nQuadPhaseReadIndex], m_SinArray[nQuadPhaseReadIndexNext], fFrac);
		break;
	}


	m_fReadIndex += m_f_inc; // add increment for next time 
	m_fQuadPhaseReadIndex += m_f_inc;

	if (m_fReadIndex > 1024) {
		m_fReadIndex = m_fReadIndex - 1024;
	}
	if (m_fQuadPhaseReadIndex > 1024) {
		m_fQuadPhaseReadIndex = m_fQuadPhaseReadIndex - 1024;
	}
	// write out 
	*pYn = fOutSample;
	*pYqn = fQuadPhaseOutSample; 

	// create unipolar; div 2 then shift up 0.5 
	if (selectedPolarity == unipolar) {
		*pYn /= 2.0;
		*pYn += 0.5;

		*pYqn /= 2.0;
		*pYqn += 0.5; 
	}
}


void OscillatorGenerator::cookFrequency() {
	m_f_inc = 1024.0 * m_fFrequency_Hz / m_nSampleRate;
}

void OscillatorGenerator::setSampleRate(float sr) {
	sr = m_nSampleRate;
}

void OscillatorGenerator::setMode(Mode m) {
	selectedMode = m; 
}

void OscillatorGenerator::setFrequency(float frequency) {
	m_fFrequency_Hz = frequency;
	cookFrequency();
}

void OscillatorGenerator::setWaveform(Waveform wf) {
	selectedWaveform = wf;  
}

void OscillatorGenerator::setPolarity(Polarity p) {
	selectedPolarity = p; 
}

void OscillatorGenerator::reset() {
	m_fReadIndex = 0;
	m_fQuadPhaseReadIndex = 0; 
	cookFrequency();
}