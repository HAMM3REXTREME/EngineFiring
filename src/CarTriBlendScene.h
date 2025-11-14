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

    float sample_rate = 48000;
    Engine superchargerEngine;

    std::unordered_map<std::string, float> input_values;

    TurboWhooshGenerator whoosh{sample_rate};
    BackfireSoundGenerator backfire{sample_rate};
    std::unique_ptr<Map2D> whoosh_am;
    std::unique_ptr<Map2D> whoosh_intensity;
    std::unique_ptr<Map2D> backfire_intensity;
    std::unique_ptr<Map2D> backfire_intensity_falloff;

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
    std::unique_ptr<AudioContext> backfire_ctx;
    std::unique_ptr<AudioContext> main_ctx;

    std::string base_dir;

    int tick_count = 0;
    int ticks_since_backfire_start = 0.0;
    bool is_backfiring = false;
    bool is_loaded = false;

    void tick() {
        tick_count++;
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

        // Backfire on logic
        if (is_backfiring) {
            ticks_since_backfire_start++;
            backfire.setIntensity(backfire_intensity->getValue(rpm, load) * backfire_intensity_falloff->getValue(ticks_since_backfire_start, 0.0f));
        }
        // Backfire cut logic
        if (ticks_since_backfire_start >= cfg.getFloat("backfire_duration_ticks", 250.0f)) {
            is_backfiring = false;
            ticks_since_backfire_start = 0.0f;
            //backfire.setIntensity(0.0f);
        }
        // Trigger new backfire timer, only if we've gone on the gas before letting off again (to prevent constant restarting of the backfiring timer)
        if (load <= cfg.getFloat("backfire_trigger_load", 10.0f) && !is_backfiring && is_loaded) {
            is_backfiring = true;
            is_loaded = false;
            ticks_since_backfire_start = 0.0f;
        }
        if (load >= cfg.getFloat("backfire_reset_load", 25.0f)) {
            is_loaded = true;
        }

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

        soundBanksInit();
        enginesInit();
        generatorsInit();
        map2dInit();
        audioCtxInit();
        filtersInit();
    }

  private:
    void soundBanksInit() {
        // Main SoundBank
        engine_samples.addFromWavList(cfg.getString("engine_samples_path"));
    }
    void enginesInit() {
        // Main Engine
        std::string firing_order = cfg.getString("engine_firing_order", "1");
        main_engine_def = std::make_unique<Engine>(cfg.getString("engine_name"), Engine::getFiringOrderFromString(firing_order));
        if (cfg.hasKey("engine_firing_interval")) {
            std::string firing_interval = cfg.getString("engine_firing_interval");
            main_engine_def->setIntervalsFromDegrees(Engine::getFiringIntervalFromString(firing_interval));
        }

        // Supercharger
        superchargerEngine = Engine("supercharger", {0}, 1.0f);
    }
    void generatorsInit() {
        // Tri EngineSoundGenerators
        engine_lo_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);
        engine_hi_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);
        engine_mech_note = std::make_unique<EngineSoundGenerator>(engine_samples, *main_engine_def, 1000.0f, 0.0f);
        // Note offsets
        engine_lo_note->setNoteOffset(cfg.getInt("note_offset_engine_lo"));
        engine_hi_note->setNoteOffset(cfg.getInt("note_offset_engine_hi"));
        engine_mech_note->setNoteOffset(cfg.getInt("note_offset_engine_mech"));

        supercharger = std::make_unique<EngineSoundGenerator>(engine_samples, superchargerEngine, 1000.0f, 0.0f);
        // Turbo
        turbo_shaft = std::make_unique<EngineSoundGenerator>(engine_samples, superchargerEngine, 1000.0f, 0.0f);
        turbo_shaft->setNoteOffset(cfg.getInt("note_offset_turbo"));
        // Backfire fixed amplitude for now
        backfire.setAmplitude(0.4f);
    }
    void map2dInit() {
        // Tri engines mapping
        engine_lo_am = std::make_unique<Map2D>(base_dir + "/e_lo_amp.map");
        engine_hi_am = std::make_unique<Map2D>(base_dir + "/e_hi_amp.map");
        engine_mech_am = std::make_unique<Map2D>(base_dir + "/e_mech_amp.map");

        // TurboShaft mapping
        turbo_shaft_am = std::make_unique<Map2D>(base_dir + "/turbo_shaft_amp.map");
        turbo_shaft_rpm = std::make_unique<Map2D>(base_dir + "/turbo_shaft_rpm.map");
        whoosh_am = std::make_unique<Map2D>(base_dir + "/whoosh_amp.map");
        whoosh_intensity = std::make_unique<Map2D>(base_dir + "/whoosh_intensity.map");
        backfire_intensity = std::make_unique<Map2D>(base_dir + "/backfire_intensity.map");
        backfire_intensity_falloff = std::make_unique<Map2D>(base_dir + "/backfire_intensity_falloff.map");
    }
    void audioCtxInit() {
        engine_ctx = std::make_unique<AudioContext>(std::vector<SoundGenerator *>{engine_lo_note.get(), engine_hi_note.get(), engine_mech_note.get()});
        backfire_ctx = std::make_unique<AudioContext>(std::vector<SoundGenerator *>{&backfire});
        main_ctx = std::make_unique<AudioContext>(std::vector<SoundGenerator *>{engine_ctx.get(), backfire_ctx.get()});
        if (cfg.getBool("enable_turbo", false)) {
            main_ctx->addGenerator(turbo_shaft.get());
            main_ctx->addGenerator(&whoosh);
        }
    }

    void filtersInit() {
        // Helper lambda for readability
        auto makeBiquad = [this](int type, float freq, float q, float db) { return std::make_unique<Biquad>(type, freq / sample_rate, q, db); };

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

        // Backfire Context
        backfire_ctx->addFilter(makeBiquad(bq_type_lowshelf, 150.0f, 0.707f, 12.0f));
        backfire_ctx->addFilter(makeBiquad(bq_type_highshelf, 3000.0f, 0.707f, -12.0f));
        // backfire_ctx->addFilter(makeBiquad(bq_type_peak, 4500.0f, 0.3f, -12.0f));
        backfire_ctx->addFilter(makeBiquad(bq_type_peak, 2100.0f, 4.36f, 2.0f));
        backfire_ctx->addFilter(makeBiquad(bq_type_peak, 1500.0f, 1.0f, 3.0f));
        backfire_ctx->addFilter(makeBiquad(bq_type_peak, 14600.0f, 4.36f, -14.0f));
        backfire_ctx->addFilter(makeBiquad(bq_type_lowshelf, 54.0f, 0.707f, 5.0f)); // GT-R
        backfire_ctx->addFilter(std::make_unique<SecondOrderFilter>(350.0f, 0.5f, 1.0f / 48000.0f));
        backfire_ctx->addFilter(std::make_unique<SecondOrderFilter>(2050.0f, 0.3f, 1.0f / 48000.0f));
    }
};