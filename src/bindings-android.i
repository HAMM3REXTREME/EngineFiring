%module enginefire

%{
#include "SoundGenerator.h"
#include "AudioContext.h"
#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Car.h"
#include "Damper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SimpleSoundGenerator.h"
#include "SoundBank.h"
#include "TurboWhooshGenerator.h"
#include "Biquad.h"
#include "android/OboeSinePlayer.h"
%}

%include "std_string.i"
%include "std_vector.i"

// Wrap std::vector<T> usage
namespace std {
  %template(vector_int) vector<int>;
  %template(vector_float) vector<float>;
  %template(vector_String) vector<std::string>;
  %template(vector_AudioVector) std::vector<AudioVector>;
}

// Make sure SoundGenerator is included before vector of pointers
%include "SoundGenerator.h"

// Wrap std::vector<SoundGenerator*>
namespace std {
  %template(SoundGeneratorPtrVector) vector<SoundGenerator*>;
}

// Now include the rest of the headers
%include "AudioContext.h"
%include "AudioVector.h"
%include "BackfireSoundGenerator.h"
%include "Car.h"
%include "Damper.h"
%include "Engine.h"
%include "EngineSoundGenerator.h"
%include "SimpleSoundGenerator.h"
%include "SoundBank.h"
%include "TurboWhooshGenerator.h"
%include "Biquad.h"
%include "android/OboeSinePlayer.h"
