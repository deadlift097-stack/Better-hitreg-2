#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>

#include "mod/Config.h"

#include <pl/Mod.hpp>
#include <pl/memory/Hook.hpp>

namespace clange_me {

class ClangeMeMod {
  public:
    static ClangeMeMod &instance();

    ClangeMeMod();

    [[nodiscard]] ll::mod::NativeMod &getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    // Called from the attack hook (game thread).
    void onAttackCall(bool isAttack);
    // Called from the sound hook (log only).
    void onSoundCall(void *self, const void *name, const float *pos, uint64_t a3, uint64_t a4,
                     float volume, float pitch);

  private:
    uintptr_t resolveAttackTarget();
    bool targetLooksValid(uintptr_t addr);
    void reloadConfigIfDue();
    void writeCalibrationFile(bool force);

    pl::memory::HookHandle mAttackHook;
    pl::memory::HookHandle mSoundHook;
    ll::mod::NativeMod &mSelf;
    ModConfig mConfig;
    std::optional<pl::config::ConfigFile<ModConfig>> mConfigFile;

    std::chrono::steady_clock::time_point mLastReload{};
    std::chrono::steady_clock::time_point mLastWrite{};
    std::atomic<uint32_t> mAttackCalls{0};
    std::atomic<uint32_t> mOtherCalls{0};
    std::atomic<int64_t> mLastAttackMs{-100000};
    std::atomic<uint32_t> mSoundSamples{0};
};

} // namespace clange_me
