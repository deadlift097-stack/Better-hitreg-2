# Better Hitreg (LeviLauncher, Bedrock 1.26.62) - scaffold

Build: push to GitHub (Actions builds the .levipack) or run ./scripts/package.ps1 -Abi arm64-v8a

STATUS: the mod loads and installs a hook ONLY if you give it a target.
Set `attackSignature` or `attackRva` in config/config.json (inside the mod folder).
Without one, the mod logs a warning and stays idle (safe).
The detour currently passes through; the instant-feedback code still has to be written
once the attack and sound functions are located in libminecraftpe.so 1.26.62.
