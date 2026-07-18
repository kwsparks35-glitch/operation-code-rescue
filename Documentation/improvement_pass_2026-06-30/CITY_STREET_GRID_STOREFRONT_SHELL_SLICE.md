# City Street Grid and Storefront Shell Slice

This slice closes the June 25 P0 world guidance for making playable cities read as actual street environments. The existing major-city urban layer already had roads, sidewalks, dense facades, and lit storefront windows; this pass turns that into an explicit reviewable street/storefront shell with crosswalks, ground-floor shop parts, role labels, and nonblocking world-promotion tags.

## Implementation

- Extended `SpawnMajorCityUrbanIdentityLayer` so its roads, sidewalks, lane paint, facades, district signage, and review sign carry `CityStreetGridStorefrontShell` and `ReadableCityStreetGrid` tags.
- Added readable crosswalk stripes and labels at major grid intersections, tagged with `StreetGridCrosswalkReadable`, `HumanScaleCurbCrossing`, and `RouteClearStreetShell`.
- Added ground-floor storefront shell pieces to dense facade rows: door recesses, ground windows, sign bands, awnings, and shop-role labels such as clinic, radio repair, market, pharmacy, transit info, hardware, community kitchen, and safe route map.
- Tagged storefront shell actors with `StorefrontShellGroundFloor`, `ModularStorefrontShell`, `ParallaxStorefrontReady`, `ImportedWorldAssetPromotionTarget`, `NoAccessBlocker`, and `RouteClearStreetShell`.
- Preserved the old `CITY LANDSCAPE PASS` review text while adding `CITY STREET GRID + STOREFRONT SHELL` so older audit scripts and new visual review both have clear signals.

## Player Result

The first playable city route now reads more like a city block: roads have sidewalks and crosswalks, districts have names, and ground floors show shopfronts with readable public uses. The added shell pieces are nonblocking route dressing and promotion targets, so they improve place readability without trapping the player or replacing future imported modular city kits.

## Validation

Added `Content/CodeRescueData/city_street_grid_storefront_shell_manifest.tsv` and `Scripts/verify_city_street_grid_storefront_shell_slice_pass.py`, then wired the verifier into local CI and full QA. The creative inclusion plan, human QA checklist, visual regression targets, and progress log now name this slice as the P0 street-grid/storefront implementation path.

Manual QA should walk the entry-to-terminal-to-survivor route and inspect downtown, civic, transit, medical, and survivor-search districts. Roads, sidewalks, crosswalks, facades, shop doors, windows, awnings, and labels should be visible, readable, and nonblocking.
