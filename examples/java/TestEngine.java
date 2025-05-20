package org.engine.firing;

import java.util.Arrays;

public class TestEngine {
    static {
        // Load the JNI shared library
        System.loadLibrary("engineaudio_jni");
    }

    public static void main(String[] args) {
        // Create SoundBank object
        SoundBank mainSamples = new SoundBank();

        // Prepare an array of WAV filenames (as std::vector<std::string>)
        // SWIG converts Java String[] to std::vector<std::string> automatically if wrapped
        java.util.List<String> wavs = Arrays.asList(
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
        );

        // Convert Java List to SWIG std::vector<std::string>
        vector_String wavVector = new vector_String();
        for (String s : wavs) {
            wavVector.add(s);
        }

        // Call addFromWavs(std::vector<std::string>)
        mainSamples.addFromWavs(wavVector);

        // Create Engine firing order vector (std::vector<int>)
        int[] firingOrderArray = {0, 5, 4, 9, 1, 6, 2, 7, 3, 8};
        vector_int firingOrder = new vector_int();
        for (int i : firingOrderArray) {
            firingOrder.add(i);
        }

        // Create Engine angles vector (std::vector<float>)
        float[] anglesArray = {90f, 54f, 90f, 54f, 90f, 54f, 90f, 54f, 90f, 54f};
        vector_float angles = new vector_float();
        for (float f : anglesArray) {
            angles.add(f);
        }

        // Create Engine object
        Engine engineDef = new Engine("Audi V10 FSI", firingOrder, angles, 5.4f);

        // Create EngineSoundGenerator (assuming constructor: SoundBank, Engine, float, float)
        EngineSoundGenerator engine = new EngineSoundGenerator(mainSamples, engineDef, 1000.0f, 0.5f);

        // Test: maybe print some info or just confirm creation
        System.out.println("EngineSoundGenerator created successfully!");
        
        while (True){
        
        }
    }
}
