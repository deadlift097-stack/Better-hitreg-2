#include "mod/MyMod.h"

#include <filesystem>

#include <link.h>

#include <pl/memory/Signature.hpp>

#include <pl/Mod.hpp>

namespace clange_me {

namespace {

// ASSUMPTION: GameMode::attack(this, Actor&) -> bool. Verify against 1.26.62
// before trusting it; the detour must match the real signature exactly.
using AttackFn = bool (*)(void *self, void *actor);

uintptr_t moduleBase(const char *name) {
    struct Ctx { const char *name; uintptr_t base; } ctx{name, 0};
    dl_iterate_phdr(
        [](dl_phdr_info *info, size_t, void *data) -> int {
            auto *c = static_cast<Ctx *>(data);
            if (info->dlpi_name && std::string_view(info->dlpi_name).find(c->name) != std::string_view::npos) {
                c->base = info->dlpi_addr;
                return 1;
            }
            return 0;
        },
        &ctx);
    return ctx.base;
}

bool attackDetour(void *self, void *actor) {
    auto &mod = ClangeMeMod::instance();
    // Instant client-side feedback (sound / effects) goes here once the
    // matching game functions are located. For now this just passes through.
    return reinterpret_cast<AttackFn>(mod.original())(self, actor);
}

} // namespace

uintptr_t ClangeMeMod::resolveAttackTarget() {
    auto &log = getSelf().getLogger();
    if (!mConfig.attackSignature.empty()) {
        uintptr_t addr = pl::memory::resolveSignature(mConfig.attackSignature, "libminecraftpe.so");
        if (addr == 0)
            log.warn("attackSignature did not match in libminecraftpe.so");
        return addr;
    }
    if (!mConfig.attackRva.empty()) {
        uintptr_t base = moduleBase("libminecraftpe.so");
        if (base == 0) {
            log.warn("Could not find libminecraftpe.so base address");
            return 0;
        }
        return base + static_cast<uintptr_t>(std::stoull(mConfig.attackRva, nullptr, 16));
    }
    return 0;
}

ClangeMeMod &ClangeMeMod::instance() {
    static ClangeMeMod instance;
    return instance;
}

ClangeMeMod::ClangeMeMod() : mSelf(*ll::mod::NativeMod::current()) {}

bool ClangeMeMod::load() {
    auto &self = getSelf();
    self.getLogger().debug("Loading...");

    std::error_code ec;
    std::filesystem::create_directories(self.getDataDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create data directory {}: {}", self.getDataDir().string(),
                               ec.message());
        return false;
    }

    std::filesystem::create_directories(self.getConfigDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create config directory {}: {}",
                               self.getConfigDir().string(), ec.message());
        return false;
    }

    mConfigFile.emplace();
    if (!mConfigFile->load()) {
        self.getLogger().warn("Failed to load typed config");
        return false;
    }
    mConfig = mConfigFile->value();

    self.getLogger().info("Loaded {} from {}", self.getName(), self.getModDir().string());
    return true;
}

bool ClangeMeMod::enable() {
    auto &self = getSelf();
    self.getLogger().debug("Enabling...");
    if (!mConfig.enabled) {
        self.getLogger().info("clange_me is disabled by config");
        return true;
    }

    uintptr_t target = resolveAttackTarget();
    if (target == 0) {
        self.getLogger().warn("No attack target configured; set attackSignature or attackRva in config.json. Mod is idle.");
        return true;
    }

    mAttackHook = pl::memory::HookHandle(reinterpret_cast<void *>(target),
                                         reinterpret_cast<void *>(&attackDetour),
                                         reinterpret_cast<void **>(&mOriginalAttack),
                                         pl::memory::HookPriority::Normal);
    if (!mAttackHook.installed()) {
        self.getLogger().error("Failed to install attack hook");
        return false;
    }
    self.getLogger().info("Attack hook installed");
    return true;
}

bool ClangeMeMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    mAttackHook.reset();
    return true;
}

bool ClangeMeMod::unload() {
    getSelf().getLogger().debug("Unloading...");
    // Release load-time resources here.
    mConfigFile.reset();
    return true;
}

} // namespace clange_me
