# TMOP Bulk Lightmap UV — Unreal Editor Python, UE 5.x / 5.8
# Run in Unreal Editor Python console: exec(open(r'C:\\path\\TMOP_BulkLightmapUV.py', encoding='utf-8').read())
# IMPORTANT: Start with DRY_RUN=True. Backup / source control before applying.
import unreal

# ===== CONFIGURE =====
CONTENT_FOLDERS = [
    '/Game/enviorment/region1',
    '/Game/enviorment/region2',
    '/Game/enviorment/region3',
    '/Game/enviorment/region4',
    '/Game/enviorment/region5',
    '/Game/enviorment/region6',
    '/Game/enviorment/region7',
    '/Game/enviorment/region9',
    '/Game/enviorment/region8Afk',
    '/Game/enviorment/region9Johannes',
]  # Exact folder names from Content Browser; includes subfolders
DRY_RUN = False                      # Set False only after reviewing the report
SAVE_ASSETS = True                 # Set True to save after successful changes
SKIP_EXISTING_UV1 = True            # Preserve existing second UV channel
MIN_LIGHTMAP_RESOLUTION = 64
LIGHTMAP_RESOLUTION = None          # None = keep existing value; e.g. 128 for testing
# =====================


def log(msg):
    unreal.log('[TMOP LightmapUV] ' + str(msg))


def warn(msg):
    unreal.log_warning('[TMOP LightmapUV] ' + str(msg))


def uv_channel_count(mesh):
    # LOD0 count via mesh editor subsystem, if supported by installed version.
    try:
        return int(editor.get_num_uv_channels(mesh, 0))
    except Exception as e:
        warn('Could not inspect UV channels for {}: {}'.format(mesh.get_name(), e))
        return None


def process(mesh):
    name = mesh.get_path_name()
    count = uv_channel_count(mesh)
    if count is None:
        return 'skipped'
    if count < 1:
        warn('SKIP no UV0: ' + name)
        return 'skipped'
    if SKIP_EXISTING_UV1 and count >= 2:
        log('SKIP already has UV1 ({} channels): {}'.format(count, name))
        return 'skipped'
    try:
        settings = editor.get_lod_build_settings(mesh, 0)
    except Exception as e:
        warn('SKIP cannot read LOD0 build settings {}: {}'.format(name, e))
        return 'skipped'

    log(('WOULD UPDATE ' if DRY_RUN else 'UPDATING ') + name + ' (UV channels: {})'.format(count))
    if DRY_RUN:
        return 'planned'

    try:
        settings.set_editor_property('generate_lightmap_u_vs', True)
        settings.set_editor_property('src_lightmap_index', 0)
        settings.set_editor_property('dst_lightmap_index', 1)
        settings.set_editor_property('min_lightmap_resolution', MIN_LIGHTMAP_RESOLUTION)
        editor.set_lod_build_settings(mesh, 0, settings)
        # UV1 is generated on mesh rebuild. Only switch index if it exists afterwards.
        updated_count = uv_channel_count(mesh)
        if updated_count is not None and updated_count >= 2:
            mesh.set_editor_property('light_map_coordinate_index', 1)
        else:
            warn('UV1 not verified after rebuild; index left unchanged: ' + name)
        if LIGHTMAP_RESOLUTION is not None:
            mesh.set_editor_property('light_map_resolution', LIGHTMAP_RESOLUTION)
        if SAVE_ASSETS:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        return 'updated'
    except Exception as e:
        warn('FAILED {}: {}'.format(name, e))
        return 'failed'


editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
registry = unreal.AssetRegistryHelpers.get_asset_registry()
assets_by_path = {}
for folder in CONTENT_FOLDERS:
    if not unreal.EditorAssetLibrary.does_directory_exist(folder):
        warn('Folder not found: ' + folder)
        continue
    folder_assets = registry.get_assets_by_path(unreal.Name(folder), recursive=True)
    log('Found {} assets under {}'.format(len(folder_assets), folder))
    for data in folder_assets:
        assets_by_path[str(data.package_name)] = data
assets = list(assets_by_path.values())
counts = {'planned': 0, 'updated': 0, 'skipped': 0, 'failed': 0}
log('Folders={} | dry_run={} | save={} | unique assets found={}'.format(len(CONTENT_FOLDERS), DRY_RUN, SAVE_ASSETS, len(assets)))
for data in assets:
    if str(data.asset_class_path.asset_name) != 'StaticMesh':
        continue
    mesh = data.get_asset()
    if mesh is None:
        continue
    result = process(mesh)
    counts[result] += 1
log('DONE: ' + str(counts))
if DRY_RUN:
    log('TEST ONLY: No assets modified. Set DRY_RUN=False to apply.')
elif not SAVE_ASSETS:
    log('Assets were changed in memory but NOT saved. Review, then save in editor or rerun with SAVE_ASSETS=True.')
