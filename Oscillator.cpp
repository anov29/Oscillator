#include "Oscillator.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "dLinTerp.h"

const int kNumPrograms = 1;

enum EParams {
  FrequencyFaderParam,
  GainFaderParam, 
  DetuneFaderParam, 
  WaveformParam,
  ModeParam,
  PolarityParam,
  OnOffParam,
  kNumParams
};

enum ELayout {
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kIFaderFrequency_L = 120,
  kIFaderFrequency_Vert_X = 78,
  kIFaderFrequency_Vert_Y = 148,
  kIFaderGain_L = 120,
  kIFaderGain_Vert_X = 13,
  kIFaderGain_Vert_Y = 148,
  kIFaderDetune_L = 120,
  kIFaderDetune_Vert_X = 148,
  kIFaderDetune_Vert_Y = 147, 
  kICC_Freq_X = 71, 
  kICC_Freq_Y = 270,
  kICC_Gain_X = 1,
  kICC_Gain_Y = 270, 
  kICC_Detune_X = 135,
  kICC_Detune_Y = 270,
  kICC_W = 70, 
  kICC_H = 30,
  kIRadioButtonsControl_N = 2,
  kIRBC_W = 96,  // width of bitmap
  kIRBC_H = 26,  // height of one of the bitmap images + 2 for image border 
  kIRadioButtonsControl_V_X_WF = 223,
  kIRadioButtonsControl_V_Y_WF = 148,
  kIRadioButtonsControl_V_X_M = 345,
  kIRadioButtonsControl_V_Y_M = 245,
  kIRadioButtonsControl_V_X_P = 223,
  kIRadioButtonsControl_V_Y_P = 58, 
  kIRBC_VN = 4,  // number of vertical buttons for waveform
  kIRBC_VN_M = 2,  // number of vertical buttons for mode
  kIRBC_VN_P = 2,  // number of vertical buttons for polarity
  //ITextControl
  kITC_X = 115,  // fext for waveform 
  kITC_Y = 155,
  kITC_W = 100,
  kITC_H = 20,
  kITC_X_M = 235, // text for mode
  kITC_X_P = 350, // text for polarity
  kISwitchControl_2_N = 2,  // # of sub-bitmaps.
  kISwitchControl_2_X = 75,  // position of left side of control
  kISwitchControl_2_Y = 55,  // position of top of control

};

Oscillator::Oscillator(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo) {
  TRACE;

  createGraphics(); 
  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);

  // begin muted
  m_bNoteon = false;
}

Oscillator::~Oscillator() {}

void Oscillator::createGraphics() {
	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->AttachBackground(BG_ID, BG_FN);

	// Attach the ISwitchControl 2 image "switch" for on/off button
	IBitmap bitmap = pGraphics->LoadIBitmap(ISWITCHCONTROL_2_ID, ISWITCHCONTROL_2_FN, kISwitchControl_2_N);
	pGraphics->AttachControl(new ISwitchControl(this, kISwitchControl_2_X, kISwitchControl_2_Y, OnOffParam, &bitmap));

	// Attach the vertical fader controls 
	// frequency
	GetParam(FrequencyFaderParam)->InitDouble("IFaderControl Vert", 440, 25, 4200, .1, "Hz");
	bitmap = pGraphics->LoadIBitmap(IFADERCONTROL_VERT_ID, IFADERCONTROL_VERT_FN);
	pGraphics->AttachControl(new IFaderControl(this, kIFaderFrequency_Vert_X, kIFaderFrequency_Vert_Y, kIFaderFrequency_L, FrequencyFaderParam, &bitmap)); // kVertical is default
	IText text = IText(14);
	ICaptionControl *captionControl = new ICaptionControl(this, IRECT(kICC_Freq_X, kICC_Freq_Y, (kICC_Freq_X + kICC_W), (kICC_Freq_Y + kICC_H)), FrequencyFaderParam, &text);
	captionControl->DisablePrompt(false);
	pGraphics->AttachControl(captionControl);

	// gain
	GetParam(GainFaderParam)->InitDouble("IFaderControl Gain", .5, 0, 1, .1, "");
	pGraphics->AttachControl(new IFaderControl(this, kIFaderGain_Vert_X, kIFaderGain_Vert_Y, kIFaderGain_L, GainFaderParam, &bitmap));
	ICaptionControl *captionControlGain = new ICaptionControl(this, IRECT(kICC_Gain_X, kICC_Gain_Y, (kICC_Gain_X + kICC_W), (kICC_Gain_Y + kICC_H)), GainFaderParam, &text);
	captionControlGain->DisablePrompt(false);
	pGraphics->AttachControl(captionControlGain);

	// detune 
	GetParam(DetuneFaderParam)->InitDouble("IFaderControl Detune", 0, 0, 500, .1, "Hz");
	pGraphics->AttachControl(new IFaderControl(this, kIFaderDetune_Vert_X, kIFaderDetune_Vert_Y, kIFaderDetune_L, DetuneFaderParam, &bitmap));
	ICaptionControl *captionControlDetune = new ICaptionControl(this, IRECT(kICC_Detune_X, kICC_Detune_Y, (kICC_Detune_X + kICC_W), (kICC_Detune_Y + kICC_H)), DetuneFaderParam, &text);
	captionControlDetune->DisablePrompt(false);
	pGraphics->AttachControl(captionControlDetune);

	//text.mStyle = text.kStyleBold;
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X, kITC_Y, (kITC_X + kITC_W), (kITC_Y + kITC_H)), &text, "Sine"));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X, kITC_Y + 27, (kITC_X + kITC_W), (kITC_Y + 27 + kITC_H)), &text, "Saw"));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X - 2, kITC_Y + 54, (kITC_X - 2 + kITC_W), (kITC_Y + 54 + kITC_H)), &text, "Tri"));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X, kITC_Y + 80, (kITC_X + kITC_W), (kITC_Y + 80 + kITC_H)), &text, "Square"));

	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X_M, kITC_Y, (kITC_X_M + kITC_W), (kITC_Y + kITC_H)), &text, "Normal"));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X_M, kITC_Y + 27, (kITC_X_M + kITC_W), (kITC_Y + 27 + kITC_H)), &text, "Band-Limit"));

	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X_P, kITC_Y, (kITC_X_P + kITC_W), (kITC_Y + kITC_H)), &text, "Bipolar"));
	//pGraphics->AttachControl(new ITextControl(this, IRECT(kITC_X_P, kITC_Y + 27, (kITC_X_P + kITC_W), (kITC_Y + 27 + kITC_H)), &text, "Unipolar"));

	GetParam(WaveformParam)->InitInt("IRadioButtonsControl Vert", 0, 0, 3, "button"); // needed if using more than 2 radio buttons 
	//Attach the vertical IRadioButtonsControl
	bitmap = pGraphics->LoadIBitmap(IRADIOBUTTONSCONTROL_ID, IRADIOBUTTONSCONTROL_FN, kIRadioButtonsControl_N);
	pGraphics->AttachControl(new IRadioButtonsControl(this, IRECT(kIRadioButtonsControl_V_X_WF, kIRadioButtonsControl_V_Y_WF, kIRadioButtonsControl_V_X_WF + (kIRBC_W*kIRBC_VN), kIRadioButtonsControl_V_Y_WF + (kIRBC_H*kIRBC_VN)), WaveformParam, kIRBC_VN, &bitmap));
	pGraphics->AttachControl(new IRadioButtonsControl(this, IRECT(kIRadioButtonsControl_V_X_M, kIRadioButtonsControl_V_Y_WF, kIRadioButtonsControl_V_X_M + (kIRBC_W*kIRBC_VN_M), kIRadioButtonsControl_V_Y_WF + (kIRBC_H*kIRBC_VN_M)), ModeParam, kIRBC_VN_M, &bitmap));
	pGraphics->AttachControl(new IRadioButtonsControl(this, IRECT(kIRadioButtonsControl_V_X_P, kIRadioButtonsControl_V_Y_P, kIRadioButtonsControl_V_X_P + (kIRBC_W*kIRBC_VN_P), kIRadioButtonsControl_V_Y_P + (kIRBC_H*kIRBC_VN_P)), PolarityParam, kIRBC_VN_M, &bitmap));

	AttachGraphics(pGraphics);
}


void Oscillator::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  // Mutex is already locked for us.

  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2) {
	  double one = 0; 
	  double two = 0;
	  double oneD = 0;
	  double twoD = 0;
	  double *normalOut1 = &one; 
	  double *normalOut2 = &two;
	  double *detuneOut1 = &oneD; 
	  double *detuneOut2 = &twoD; 

	  if (!m_bNoteon) {
		  *out1 = 0;
		  *out2 = 0;
	  } else {
		  generator.generate(normalOut1, normalOut2);
		  detuneGenerator.generate(detuneOut1, detuneOut2);
		  // *out2 = *out1; // mono out, no quad phase 
	  }
	  *out1 = gain * (*normalOut1 + *detuneOut1);
	  *out2 = gain * (*normalOut2 + *detuneOut2);
  }
}

void Oscillator::Reset() {
  TRACE;
  IMutexLock lock(this);
  generator.reset();
}

void Oscillator::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
  switch (paramIdx)
  {
  case FrequencyFaderParam:
	  if (GetGUI()) {
		  GetGUI()->SetParameterFromPlug(FrequencyFaderParam, GetParam(paramIdx)->Value(), false);
	  }
	  generator.setFrequency(GetParam(paramIdx)->Value());
	  detuneGenerator.setFrequency(GetParam(paramIdx)->Value() + detune);
	  break;
  case GainFaderParam:
	  gain = GetParam(paramIdx)->Value();
	  if (GetGUI()) {
		  GetGUI()->SetParameterFromPlug(GainFaderParam, GetParam(paramIdx)->Value(), false);
	  }
	  break;
  case DetuneFaderParam:
	  if (GetGUI()) {
		  GetGUI()->SetParameterFromPlug(DetuneFaderParam, GetParam(paramIdx)->Value(), false);
	  }
	  detune = GetParam(paramIdx)->Value();
	  detuneGenerator.setFrequency(generator.getFrequency() + detune);
	  break; 
  case OnOffParam:
	  m_bNoteon = GetParam(paramIdx)->Value();
	  if (m_bNoteon) {
		  generator.reset();
		  detuneGenerator.reset(); 
	  }
	  break;
  case WaveformParam: {
	  int wf = GetParam(paramIdx)->Value();
	  generator.setWaveform((OscillatorGenerator::Waveform)wf);
	  detuneGenerator.setWaveform((OscillatorGenerator::Waveform)wf);
	  break;
  }
  case ModeParam: {
	  int m = GetParam(paramIdx)->Value();
	  generator.setMode((OscillatorGenerator::Mode)m);
	  detuneGenerator.setMode((OscillatorGenerator::Mode)m);
	  break;
  }
  case PolarityParam: {
	  int p = GetParam(paramIdx)->Value();
	  generator.setPolarity((OscillatorGenerator::Polarity)p);
	  detuneGenerator.setPolarity((OscillatorGenerator::Polarity)p);
	  break;
  }
  default:
      break;
  }
}
