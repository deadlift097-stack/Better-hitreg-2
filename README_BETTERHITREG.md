# Better Hitreg (LeviLauncher, Minecraft Bedrock 1.26.62.1)

Build: push to GitHub (Actions builds the .levipack) or run ./scripts/package.ps1 -Abi arm64-v8a

## Works now
- In-game menu module "Better Hitreg" (on/off and settings entries).
- Settings are re-read from the mod's config.json every 2 seconds.
- Attack hook (RVA 0x1009d290, libminecraftpe.so 1.26.62.1). Bytes are checked before hooking.

## Log files (in the mod's data folder)
- hit_calibration.txt: attack_calls rises when you hit something.
- sound_samples.txt: sounds played within 1.5 s after an attack (log only, max 80 lines).
  Hook: SoundEngine play-style function at RVA 0xbce0498 (bytes checked first).

## Not done yet
The instant hit sound and hit flash. The sound call is located but its two extra
arguments are not confirmed (sound_samples.txt shows real values). The hurt-animation
function has not been found.
