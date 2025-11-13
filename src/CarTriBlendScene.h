#pragma once
#include "AudioContext.h"
#include "CarBlendScene.h"
#include "CubicClipper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "Map2D.h"
#include "SecondOrderFilter.h"
#include "SimpleConfig.h"
#include "SineClipper.h"
#include "SoundBank.h"
#include <memory>

class CarTriBlendScene : public CarBlendScene {
  public:
    CarTriBlendScene() {}
    ~CarTriBlendScene() {}
    SimpleConfig cfg;

    SoundBank engine_samples;
    std::unique_ptr<Engine> main_engine_def;

    float SAMPLE_RATE = 48000;
    Engine superchargerEngine;

    std::unordered_map<std::string, float> input_values;

    TurboWhooshGenerator whoosh{SAMPLE_RATE};
    std::unique_ptr<Map2D> whoosh_am;
    std::unique_ptr<Map2D> whoosh_intensity;

    std::unique_ptr<EngineSoundGenerator> engine_lo_note;
    std::unique_ptr<EngineSoundGenerator> engine_hi_note;
    std::unique_ptr<EngineSoundGenerator> engine_mech_note;

    std::unique_ptr<EngineSoundGenerator> supercharger;
    std::unique_ptr<EngineSoundGenerator> turbo_shaft;
    std::unique_ptr<Map2D> turbo_shaft_am;
    std::unique_ptr<Map2D> turbo_shaft_rpm;

    // Artistic Amplitude Modulation
    std::unique_ptr<Map2D> engine_lo_am;
    std::unique_ptr<Map2D> engine_hi_am;
    std::unique_ptr<Map2D> engine_mech_am;

    std::unique_ptr<AudioContext> engine_ctx;
    std::unique_ptr<AudioContext> main_ctx;

    std::string base_dir;

    void tick() {
        float rpm = input_values["rpm"];
        float load = input_values["load"];
        float boost = input_values["boost"];
        engine_lo_note->setRPM(rpm);
        engine_hi_note->setRPM(rpm);
        engine_mech_note->setRPM(rpm);

        turbo_shaft->setRPM(turbo_shaft_rpm->getValue(boost, rpm));
        turbo_shaft->setAmplitude(turbo_shaft_am->getValue(boost, rpm));
        whoosh.setIntensity(whoosh_intensity->getValue(boost, rpm));
        whoosh.setAmplitude(whoosh_am->getValue(boost, rpm));

        engine_lo_note->setAmplitude(engine_lo_am->getValue(rpm, load));
        engine_hi_note->setAmplitude(engine_hi_am->getValue(rpm, load));
        engine_mech_note->setAmplitude(engine_mech_am->getValue(rpm, load));
        // Example hardcoded active eq
        auto *biquad = dynamic_cast<Biquad *>(engine_ctx->fx_chain[0]);
        if (biquad) {
            biquad->setPeakGain(8.0f - (load / 20.0f));
        }
    }

    void buildFromCfg(const std::string &cfgFile) {
        std::filesystem::path cfg_path = cfgFile;
        base_dir = cfg_path.parent_path().string();
        std::cout << "base dir: " << base_dir << "\n";
        cfg.load(cfgFile);

        // Main SoundBank
        engine_samples.addFromWavList(cfg.getString("engine_samples_path"));
        // Main Engine
        std::string firing_order = cfg.getString("engine_firing_order", "1-2-3-4-5-6");
        main_engine_def = std::make_unique<Engine>(cfg.getString("engine_name"), Engine::getFiringOrderFromString(firing_order));
        if (cfg.hasKey("engine_firing_interval")) {
            std::string firing_interval = cfg.getString("engine_firing_interval");
            main_engine_def->setIntervalsFromDegrees(Engine::getFiringIntervalFromString(firing_interval));
        }

        // Tri EngineSoundGenerators
        engine_lo_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);
        engine_hi_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);
        engine_mech_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);

        engine_lo_note->setNoteOffset(cfg.getInt("note_offset_engine_lo"));
        engine_hi_note->setNoteOffset(cfg.getInt("note_offset_engine_hi"));
        engine_mech_note->setNoteOffset(cfg.getInt("note_offset_engine_mech"));

        engine_lo_am = std::make_unique<Map2D>(base_dir + "/e_lo_amp.map");
        engine_hi_am = std::make_unique<Map2D>(base_dir + "/e_hi_amp.map");
        engine_mech_am = std::make_unique<Map2D>(base_dir + "/e_mech_amp.map");

        turbo_shaft_am = std::make_unique<Map2D>(base_dir + "/turbo_shaft_amp.map");
        turbo_shaft_rpm = std::make_unique<Map2D>(base_dir + "/turbo_shaft_rpm.map");
        whoosh_am = std::make_unique<Map2D>(base_dir + "/whoosh_amp.map");
        whoosh_intensity = std::make_unique<Map2D>(base_dir + "/whoosh_intensity.map");
        // Supercharger
        superchargerEngine = Engine("supercharger", {0}, 1.0f);
        supercharger = std::make_unique<EngineSoundGenerator>(engine_samples, superchargerEngine, 1000.0f, 0.0f);
        turbo_shaft = std::make_unique<EngineSoundGenerator>(engine_samples, superchargerEngine, 1000.0f, 0.0f);
        turbo_shaft->setNoteOffset(cfg.getInt("note_offset_turbo"));
        std:: cout << turbo_shaft->getInfo(0);
        engine_ctx = std::make_unique<AudioContext>(std::vector<SoundGenerator *>{engine_lo_note.get(), engine_hi_note.get(), engine_mech_note.get()});
        main_ctx = std::make_unique<AudioContext>(std::vector<SoundGenerator *>{ engine_ctx.get(),turbo_shaft.get(), &whoosh});

        // Helper lambda for readability
        auto makeBiquad = [this](int type, float freq, float q, float db) { return std::make_unique<Biquad>(type, freq / SAMPLE_RATE, q, db); };

        // Engine context filters
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 1600.0f, 0.707f, 0.0f)); // Example active filter
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 1500.0f, 1.0f, 3.0f));   // Mid boost
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 22.0f, 4.36f, -36.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 28.0f, 4.36f, -16.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 18100.0f, 4.36f, -26.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 14600.0f, 4.36f, -14.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 2100.0f, 4.36f, 2.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 2600.0f, 4.36f, 2.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 3600.0f, 4.36f, 2.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 4000.0f, 4.36f, 2.0f));

        // Low end
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 120.0f, 0.707f, 4.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 300.0f, 1.0f, 3.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 500.0f, 0.707f, 3.0f));

        // Mid range
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 1700.0f, 0.707f, 4.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 3000.0f, 0.707f, 4.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 8000.0f, 0.707f, -1.0f));
        engine_ctx->addFilter(makeBiquad(bq_type_peak, 9000.0f, 0.707f, -2.0f));
        engine_ctx->addFilter(std::make_unique<SecondOrderFilter>(2950.0f, 0.6f, 1.0f / 48000.0f));
        engine_ctx->addFilter(std::make_unique<SecondOrderFilter>(350.0f, 0.6f, 1.0f / 48000.0f));
        engine_ctx->addFilter(std::make_unique<SecondOrderFilter>(200.0f, 0.3f, 1.0f / 48000.0f));
        engine_ctx->addFilter(std::make_unique<SineClipper>());
        engine_ctx->addFilter(std::make_unique<CubicClipper>());
    }
};