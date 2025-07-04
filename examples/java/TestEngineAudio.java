package com.hammer.engine.firing;

import javax.sound.sampled.*;
import java.util.Arrays;

public class TestEngineAudio {
    static {
        // Load the JNI shared library
        System.loadLibrary("engineaudio_jni");
    }

    public static void main(String[] args) throws Exception {
        // ==== Setup engine ====
        SoundBank mainSamples = new SoundBank();
        vector_String wavVector = new vector_String();
        for (String s : Arrays.asList(
                "assets/audio/thump_library/note_88.wav",
                "assets/audio/thump_library/note_89.wav",
                "assets/audio/thump_library/note_90.wav",
                "assets/audio/thump_library/note_91.wav",
                "assets/audio/thump_library/note_92.wav",
                "assets/audio/thump_library/note_93.wav",
                "assets/audio/thump_library/note_94.wav",
                "assets/audio/thump_library/note_95.wav",
                "assets/audio/thump_library/note_96.wav",
                "assets/audio/thump_library/note_97.wav",
                "assets/audio/thump_library/note_98.wav",
                "assets/audio/thump_library/note_99.wav"
        )) {
            wavVector.add(s);
        }
        mainSamples.addFromWavs(wavVector);

        vector_int firingOrder = new vector_int();
        for (int i : new int[]{0, 5, 4, 9, 1, 6, 2, 7, 3, 8}) {
            firingOrder.add(i);
        }

        vector_float angles = new vector_float();
        for (float f : new float[]{90f, 54f, 90f, 54f, 90f, 54f, 90f, 54f, 90f, 54f}) {
            angles.add(f);
        }

        Engine engineDef = new Engine("Audi V10 FSI", firingOrder, angles, 5.4f);
        EngineSoundGenerator engine = new EngineSoundGenerator(mainSamples, engineDef, 5000.0f, 0.5f);

        // ==== Audio Playback ====
        float sampleRate = 48000.0f;
        int bufferSize = 1024;

        AudioFormat format = new AudioFormat(sampleRate, 16, 1, true, false);
        SourceDataLine line = AudioSystem.getSourceDataLine(format);
        line.open(format, bufferSize * 2);
        line.start();

        byte[] audioBuffer = new byte[bufferSize * 2]; // 16-bit mono
        short[] samples = new short[bufferSize];

        System.out.println("Streaming audio... Press Ctrl+C to stop.");
        //engine.setRPM(1000.0f);

        while (true) {
            for (int i = 0; i < bufferSize; i++) {
                engine.update();
                float sample = engine.getSample();  // returns float [-1.0, 1.0]
                short pcm = (short)(sample * 32767);
                samples[i] = pcm;
            }

            // Convert to byte[]
            for (int i = 0; i < bufferSize; i++) {
                audioBuffer[2 * i] = (byte)(samples[i] & 0xFF);
                audioBuffer[2 * i + 1] = (byte)((samples[i] >> 8) & 0xFF);
            }

            line.write(audioBuffer, 0, audioBuffer.length);
        }
    }
}
