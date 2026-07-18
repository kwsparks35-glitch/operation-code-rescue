# #65 — Mod loader runs at GameInstance::Init

`UCodeRescueModLoader::LoadAllMods()` (#60) was static and never called.
`UCodeRescueGameInstance::Init()` now invokes it after the persistent
run loads, then logs the count of detected mods.

```cpp
const TArray<FCodeRescueModManifest> Mods = UCodeRescueModLoader::LoadAllMods();
if (Mods.Num() > 0)
{
    UE_LOG(LogTemp, Display, TEXT("[Mods] Loaded %d user mod(s) from Saved/Mods/"), Mods.Num());
}
```

The custom-challenge merge into the active pool happens inside
`UCodeRunnerLibrary::LoadCustomChallenges` (item #49), which is called
on demand. This Init-time call simply primes the manifest cache and
logs a count so the player can see "mods detected" without opening a UI.

## Why log instead of toast on screen
At Init time there's no viewport yet (the GameInstance starts before
any world). On-screen messages from before viewport-ready are dropped;
log is reliable.

## Files
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp` — new include +
  Init() addition.
