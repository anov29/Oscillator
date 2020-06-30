#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "IPlug_include_in_plug_hdr.h"
#include "OscillatorGenerator.h"

class Oscillator : public IPlug
{
public:
  Oscillator(IPlugInstanceInfo instanceInfo);
  ~Oscillator();

  OscillatorGenerator generator;

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void createGraphics(); 

private:
  bool m_bNoteon; // on/off message 
};

#endif
