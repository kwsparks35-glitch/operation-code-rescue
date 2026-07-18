import unreal


ASSET_PATH = "/Game/UrbanZombie4/Mesh/SK_UrbanZombie4_Skeleton"


def log(message):
    unreal.log(f"[cr-urban-skeleton-repair] {message}")


def main():
    skeleton = unreal.load_asset(ASSET_PATH)
    if not skeleton:
        raise RuntimeError(f"Could not load {ASSET_PATH}")

    log(f"loaded {ASSET_PATH}: {skeleton.get_class().get_name()}")
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH, only_if_is_dirty=False)
    log("saved skeleton package with current UE metadata")


if __name__ == "__main__":
    main()
