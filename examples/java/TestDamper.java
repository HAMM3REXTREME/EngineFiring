package org.engine.firing;

public class TestDamper {
    static {
        // Load the JNI shared library
        System.loadLibrary("engineaudio_jni");
    }

    public static void main(String[] args) {
        try {
            // Construct Damper with some int parameter (e.g., 5)
            Damper damper = new Damper(5);

            // Call a method on Damper, e.g., addValue
            damper.addValue(1.23);

            // Print out the average value (assuming getAverage exists)
            double avg = damper.getAverage();
            System.out.println("Damper average value: " + avg);

            System.out.println("Damper constructed and tested successfully.");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
