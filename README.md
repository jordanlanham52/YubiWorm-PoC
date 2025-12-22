# YubiWorm

**Turning a Trusted YubiKey into a Propagating Worm**

<img width="1080" height="1080" alt="yubiworm-turning-a-trusted-yubikey-into-a-propaga_lanham_1030572_banner(1)" src="https://github.com/user-attachments/assets/3be17258-999e-4fba-8867-3a7cc2699280" />



---

## Overview

YubiWorm demonstrates how a real YubiKey can be weaponized to silently deliver and propagate a payload across hosts by abusing its HID “static slot” and the `ykman` CLI. Once installed on a host, YubiWorm will:

1. **Monitor** for any YubiKey insertion (`udev-monitor` daemon).  
2. **Reprogram** the key’s Static OTP slot to send a custom HID macro payload (`reprogram_yubi` helper).  
3. **Propagate** itself to the next host via a small downloader script.  
4. **Persist** on infected hosts via cron or scheduled task so future reboots continue propagation.  

---

## Project Structure & Paths

Below is an overview of each YubiWorm component, what it does, and where it resides on Linux and Windows installations:

- Monitor Daemon: A background process that listens for YubiKey insertions and triggers the reprogram helper.
- Reprogram Helper: A small helper binary that overwrites the YubiKey’s Static OTP slot with a HID macro payload.
- Downloader Script: A bootstrapping installer (payload.sh) that hosts use to install binaries and persistence.
- Cron Persistence: Ensures the monitor daemon auto-starts on reboot via cron (or Scheduled Task on Windows).
  
| Component          | Linux Path                                  | Windows Path (WSL or Git Bash)                          |
|--------------------|---------------------------------------------|----------------------------------------------------------|
| Monitor Daemon     | `/usr/local/bin/udev-monitor`               | `C:\Tools\udev-monitor.exe`                              |
| Reprogram Helper   | `/usr/local/bin/reprogram_yubi`             | `C:\Tools\reprogram_yubi.exe`                            |
| Downloader Script  | `http://attacker.example.com/payload.sh`    | `https://attacker.example.com/payload.ps1`               |
| Cron Persistence   | `crontab -l | grep udev-monitor`            | Scheduled Task: `schtasks /query /TN "YubiWormMonitor"`  |

---

## Capabilities & Limitations

| **Can Do**                                                                 | **Can’t Do**                                                             |
|-----------------------------------------------------------------------------|---------------------------------------------------------------------------|
| Detect YubiKey plug-ins via udev netlink events                             | Modify YubiKey firmware or OTP slot beyond slot 1                         |
| Reprogram Static OTP slot to inject HID macros                              | Bypass user-touch requirement completely (needs tap to fire payload)      |
| Open Run dialog or terminal via HID macro keystrokes                        | Guarantee Desktop Environment hotkeys are configured identically           |
| Download and install binaries on fresh Linux hosts                          | Propagate to non-Linux or locked-down hosts without appropriate tools      |
| Persist across reboots via cron or scheduled task                           | Evade all endpoint detection; visible in process lists under disguised name|

---

## To-Do Checklist

| Task                                                      | Status     |
|-----------------------------------------------------------|------------|
| Compile `udev-monitor` daemon for Linux                   | ✅ Done     |
| Compile `reprogram_yubi` helper for Linux                 | ✅ Done     |
| Implement invisible payload via `{ALT}{F2}` macro         | ❌ Pending  |
| Write `payload.sh` downloader script with cron persistence| ✅ Done     |
| **Windows port**: build compatible `udev-monitor.exe`      | ❌ Pending  |
| **Windows port**: build compatible `reprogram_yubi.exe`    | ❌ Pending  |
| Harden persistence (e.g. registry run key)                | ❌ Pending  |
| Add logging/alert suppression                             | ❌ Pending  |
| Automate build & release via CI pipeline                  | ❌ Pending  |

---

## Security & Ethics Disclaimer

> **Purpose:** Educational research / red-team proof-of-concept only.  
> **Do Not** deploy on unauthorized networks or without explicit permission.  
> **Risk:** Demonstrates real hardware abuse; running these tools will compromise YubiKey integrity and host security.

---

*Jordan Lanham – Cyber Saguaros @ University of Arizona*  
*“Trust the process, not the plug.”*  
