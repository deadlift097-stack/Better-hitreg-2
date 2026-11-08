#pragma once

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

    [[nodiscard]] const ModConfig &config() const { return mConfig; }
    void *original() const { return mOriginalAttack; }

  private:
    uintptr_t resolveAttackTarget();

    void *mOriginalAttack{};
    pl::memory::HookHandle mAttackHook;
    ll::mod::NativeMod &mSelf;
    ModConfig mConfig;
    std::optional<pl::config::ConfigFile<ModConfig>> mConfigFile;
};

} // namespace clange_me
