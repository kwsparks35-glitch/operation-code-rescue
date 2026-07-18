import unreal
for d in ("/Game/DiagReimport",):
    if unreal.EditorAssetLibrary.does_directory_exist(d):
        ok = unreal.EditorAssetLibrary.delete_directory(d)
        unreal.log_error("[DiagCleanup] deleted {0} ok={1}".format(d, ok))
    else:
        unreal.log_error("[DiagCleanup] absent {0}".format(d))
