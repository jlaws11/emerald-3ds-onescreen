# Payload recipe for the single-screen build

`emerald3ds.recipe` lets the Emerald 3DS Builder make an `emerald3ds.pak` from
the player's own ROM (Pokémon Emerald, USA/Europe, SHA-1
f3ae088181bf583e55daf962a92bb46f4f1d07b7) for the `Emerald3DS.3dsx` of release
`single-screen-6826e1e` (data ABI 9f33cc4a). It holds no game data: made with
`tools/gen_recipe.py` within its literal limits, and audited like an official
payload (`tools/release_audit.py --zip ... --strict --rom`), with no run of
the ROM's bytes in it.

The builder AppImage (`.github/workflows/builder-appimage.yml`) bundles it with
that release's executable. A new engine build needs a new recipe.
