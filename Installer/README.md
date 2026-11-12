# PRO flash installer

This PSP-only installer uses LibPspExploit for kernel access. It installs and
verifies the PRO files in flash0 and automatically reboots into PRO after a
successful installation. It is not intended for Vita, Adrenaline, or other 
emulators.

Flashing can brick a PSP. Use a known-good NAND backup and recovery method,
connect AC power, charge the battery to at least 50%, and do not interrupt the
installer. If a write fails, flash0 may be partially updated; the installer
reports the error rather than claiming success. Existing files are overwritten
and are not backed up by the installer.

Place `EBOOT.PBP` and the package matching the installed firmware together in
`PSP/GAME/PROINSTALLER/` on the storage device. The package names are
`FLASH620.PRO`, `FLASH639.PRO`, and `FLASH660.PRO`.
6.60 and 6.61 use different firmware-specific builds; they are not
interchangeable. Only packages actually built and distributed are available.
The installer rejects unsupported firmware/model combinations and incomplete
or malformed packages before opening flash0 for writing. The archive format
does not authenticate its contents: only use packages from a trusted source.

From the project root, `make dist TARGET_FW=660` builds the 6.60 package and
installer under `dist/PSP/GAME/PROINSTALLER/`. The build also accepts 620 and 639. 
Clean the project before switching `TARGET_FW`, because the existing component 
makefiles reuse object files across firmware builds. Install the PSP toolchain 
and psp-cfw-sdk (including LibPspExploit) first.
