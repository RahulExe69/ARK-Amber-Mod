# ARK Amber Mod — Legacy Mobile

Experimental native C++ mod-menu project for **ARK: Survival Evolved Mobile (legacy), version 2.0.28**, targeting **Unreal Engine 4.17 / Android ARM64**.

This repository is a fork of [Ragekill3377/ARK-mobile-2.0-android-internal](https://github.com/Ragekill3377/ARK-mobile-2.0-android-internal). The original BSD 3-Clause license and copyright notice are retained.

## Current status

- The inherited code contains native game-hook and mod-menu source.
- **Ancient Amber modification is not implemented yet.**
- The Android NDK workflow is being used to verify that the native library can be built for ARM64.
- A successful native build produces `libShandian.so`; it does **not** produce a ready-to-install APK or package the library into the game.

## Development scope

The first goal is to verify a reproducible build, then investigate whether a safe, **local single-player-only** approach is technically viable. No feature should bypass store/payment verification, alter online account/server balances, or be used to interfere with multiplayer servers.

## Build artifact

Open **Actions → Android NDK build** and download the `Shandian-android-arm64` artifact after a successful run. The library is only a build output; it is not independently installable and has not yet been integrated into an APK.

## Compatibility and save safety

The SDK contains hard-coded offsets for the version stated above. Different game builds may crash or behave unpredictably. Back up your single-player save before testing any future experimental changes, and keep testing to local single-player.

## License

BSD 3-Clause. Preserve the upstream copyright and license notice when redistributing source or binaries.
