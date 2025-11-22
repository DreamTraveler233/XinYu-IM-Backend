#!/usr/bin/env bash
set -euo pipefail

# Setup script to create media directories and set permissions
# Usage:
#   ./setup_media_dir.sh [--upload-dir /var/lib/im/media] [--temp-dir /var/lib/im/media/tmp] [--user user] [--nginx-cache-dir /var/cache/nginx] [--nginx-user nginx]

# Defaults
UPLOAD_DIR="${UPLOAD_DIR:-/var/lib/im/media}"
TEMP_DIR="${TEMP_DIR:-${UPLOAD_DIR}/tmp}"
IM_RUN_USER="${IM_RUN_USER:-$(id -un)}"
# Optional nginx cache dir setup
NGINX_CACHE_DIR="${NGINX_CACHE_DIR:-/var/cache/nginx}"
NGINX_USER="${NGINX_USER:-nginx}"

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --upload-dir)
      UPLOAD_DIR="$2"
      shift 2
      ;;
    --temp-dir)
      TEMP_DIR="$2"
      shift 2
      ;;
    --user)
      IM_RUN_USER="$2"
      shift 2
      ;;
    --nginx-cache-dir)
      NGINX_CACHE_DIR="$2"
      shift 2
      ;;
    --nginx-user)
      NGINX_USER="$2"
      shift 2
      ;;
    *)
      echo "Unknown argument $1" >&2
      exit 1
      ;;
  esac
done

# Create directories
echo "Creating upload dirs: $UPLOAD_DIR and $TEMP_DIR"
mkdir -p "$UPLOAD_DIR"
mkdir -p "$TEMP_DIR"

# Ensure optional nginx cache client_temp exists and is writable by the nginx user
echo "Ensuring nginx cache dir: $NGINX_CACHE_DIR/client_temp"
mkdir -p "${NGINX_CACHE_DIR}/client_temp"

# Set ownership and permissions
if id "$IM_RUN_USER" &>/dev/null; then
  echo "Set owner to $IM_RUN_USER"
  chown -R "$IM_RUN_USER":"$IM_RUN_USER" "$UPLOAD_DIR"
else
  echo "User $IM_RUN_USER does not exist, skipping chown"
fi

chmod -R 755 "$UPLOAD_DIR"

# Fix nginx cache dir permissions if nginx user exists
if id "$NGINX_USER" &>/dev/null; then
  echo "Set owner of ${NGINX_CACHE_DIR} to $NGINX_USER"
  chown -R "$NGINX_USER":"$NGINX_USER" "${NGINX_CACHE_DIR}"
  chmod -R 700 "${NGINX_CACHE_DIR}"
else
  echo "User $NGINX_USER not found; skip nginx cache chown. You may need to set it manually."
fi

echo "Setup complete: $UPLOAD_DIR (temp: $TEMP_DIR)"

exit 0
