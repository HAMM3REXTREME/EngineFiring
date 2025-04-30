import librosa
import soundfile as sf
import os

# Load your sample (at C6, MIDI note 84)
sample_path = 'C6.wav'
y, sr = librosa.load(sample_path, sr=None)

# Create output directory
output_dir = 'piano_keys'
os.makedirs(output_dir, exist_ok=True)

# MIDI note numbers for piano range (A0 = 21 to C8 = 108)
for midi_note in range(21, 109):
    semitone_shift = midi_note - 84  # C6 = 84
    y_shifted = librosa.effects.pitch_shift(y=y, sr=sr, n_steps=semitone_shift)
    out_path = os.path.join(output_dir, f'note_{midi_note}.wav')
    sf.write(out_path, y_shifted, sr)

print("Done generating piano key set.")