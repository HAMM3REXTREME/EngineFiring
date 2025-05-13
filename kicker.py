import numpy as np
from scipy.io.wavfile import write

# Settings
duration = 0.5  # seconds
sample_rate = 44100
t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)

# Frequency envelope: pitch drops from 150Hz to 40Hz
start_freq = 150
end_freq = 40
freq_env = np.logspace(np.log10(start_freq), np.log10(end_freq), len(t))

# Amplitude envelope: exponential decay
amp_env = np.exp(-40 * t)

# Generate the sine wave kick
kick = np.sin(2 * np.pi * freq_env * t) * amp_env

# Normalize to int16 range
kick = kick / np.max(np.abs(kick))
kick_int16 = np.int16(kick * 32767)

# Save to file
write("synth_kick.wav", sample_rate, kick_int16)
