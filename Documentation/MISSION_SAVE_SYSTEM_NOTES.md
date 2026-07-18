# Mission Save System Notes

The upgraded project adds a `USaveGame` implementation named `UCodeRescueSaveGame`.

Runtime state is stored in the GameInstance and saved through:

- `SavePersistentRun()`
- `LoadPersistentRun()`
- `DeletePersistentRun()`
- `RecordTerminalSolved()`
- `RecordConceptAttempt()`

Currently saved values include:

- selected language
- survivors rescued
- zombies neutralized
- coding score
- terminals solved

The save data structures also include room for:

- completed missions
- per-mission best scores
- per-concept mastery
- success/failure counts

Those arrays are in place so future mission designers can expand the campaign without changing the core save type.
