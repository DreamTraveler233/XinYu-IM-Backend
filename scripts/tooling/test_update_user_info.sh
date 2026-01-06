#!/usr/bin/env bash
set -euo pipefail

# Usage: ./test_update_user_info.sh
# This script performs an integration test on UpdateUserInfo binding behaviour.
# It creates a temporary user, generates a JWT for that user id, calls /api/v1/user/detail-update
# with empty nickname/avatar/motto and verifies DB fields become NULL.

ROOT_DIR=$(cd "$(dirname "$0")/../.." && pwd)
CONFIG="$ROOT_DIR/bin/config/gateway_http/system.yaml"
MEDIA_CONFIG="$ROOT_DIR/bin/config/gateway_http/media.yaml"

PY_CONF=$(python3 - "$CONFIG" <<PY
import yaml, sys
conf_file = sys.argv[1]
with open(conf_file, 'r') as fh:
  c = yaml.safe_load(fh)
db = c.get('mysql.dbs', {}).get('default', {}) or c.get('mysql', {}).get('dbs', {}).get('default', {})
out = {
  'host': db.get('host', '127.0.0.1'),
  'port': db.get('port', 3306),
  'user': db.get('user', 'root'),
  'passwd': db.get('passwd', ''),
  'dbname': db.get('dbname', '')
}
for k, v in out.items():
  print(k + '=' + str(v))
PY
)
DB_HOST=$(echo "$PY_CONF" | grep '^host=' | cut -d'=' -f2)
DB_PORT=$(echo "$PY_CONF" | grep '^port=' | cut -d'=' -f2)
DB_USER=$(echo "$PY_CONF" | grep '^user=' | cut -d'=' -f2)
DB_PASS=$(echo "$PY_CONF" | grep '^passwd=' | cut -d'=' -f2)
DB_NAME=$(echo "$PY_CONF" | grep '^dbname=' | cut -d'=' -f2)
JWT_SECRET=$(python3 - "$CONFIG" <<PY
import yaml, sys
with open(sys.argv[1], 'r') as fh:
  c=yaml.safe_load(fh)
print(c.get('auth', {}).get('jwt', {}).get('secret','dev-secret-change-me'))
PY
)

MKTEMP_USER_MOBILE="1880000001$((RANDOM%90+10))"

# Insert a temporary user into database
SQL_INSERT="INSERT INTO im_user (mobile, nickname, gender, online_status, created_at, updated_at) VALUES ('$MKTEMP_USER_MOBILE', 'test_nick_before', 0, 'N', NOW(), NOW()); SELECT LAST_INSERT_ID();"
USER_ID=$(mysql -u "$DB_USER" -p"$DB_PASS" -h "$DB_HOST" -P "$DB_PORT" -sse "$SQL_INSERT" "$DB_NAME")
if [ -z "$USER_ID" ]; then
  echo "Failed to create test user"
  exit 1
fi

echo "Created test user id: $USER_ID mobile: $MKTEMP_USER_MOBILE"

# Create a JWT with HS256 using python
PYTHON_TOKEN=$(python3 - <<PY
import jwt, datetime
payload={'uid': str($USER_ID), 'iss': 'auth-service', 'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=1)}
print(jwt.encode(payload, "$JWT_SECRET", algorithm='HS256'))
PY
)

TOKEN=${PYTHON_TOKEN}

# Call the detail-update with empty nickname/avatar/motto to update DB
API_URL='http://127.0.0.1:8080/api/v1/user/detail-update'
PAYLOAD='{"nickname":"","avatar":"","motto":"","gender":0}'

HTTP_STATUS=$(curl -s -o /tmp/test_update_user_info.out -w "%{http_code}" -X POST -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d "$PAYLOAD" "$API_URL")
if [ "$HTTP_STATUS" != "200" ]; then
  echo "API call failed: status=$HTTP_STATUS"
  cat /tmp/test_update_user_info.out
  exit 1
fi

# Query DB to verify fields are NULL
SQL_CHECK="SELECT IFNULL(nickname,'NULL'), IFNULL(avatar,'NULL'), IFNULL(avatar_media_id,'NULL'), IFNULL(motto,'NULL') FROM im_user WHERE id=$USER_ID LIMIT 1;"
RESULT=$(mysql -u "$DB_USER" -p"$DB_PASS" -h "$DB_HOST" -P "$DB_PORT" -sse "$SQL_CHECK" "$DB_NAME")

# RESULT expected to have 'NULL' for fields we cleared
echo "DB fields after update: $RESULT"
IFS=$'\t' read -r DB_NICK DB_AVATAR DB_AVATAR_MEDIA DB_MOTTO <<< "$RESULT"

PASS=true
# nickname should remain unchanged (test_nick_before) because we use COALESCE(NULLIF(?, ''), nickname)
if [ "$DB_NICK" != "test_nick_before" ]; then
  echo "Failed: nickname changed (expected 'test_nick_before', got: $DB_NICK)"
  PASS=false
fi
if [ "$DB_AVATAR" != "NULL" ]; then
  echo "Failed: avatar is not NULL (value: $DB_AVATAR)"
  PASS=false
fi
if [ "$DB_MOTTO" != "NULL" ]; then
  echo "Failed: motto is not NULL (value: $DB_MOTTO)"
  PASS=false
fi

# Clean up inserted user
mysql -u "$DB_USER" -p"$DB_PASS" -h "$DB_HOST" -P "$DB_PORT" -e "DELETE FROM im_user WHERE id=$USER_ID;" "$DB_NAME"

if $PASS ; then
  echo "PASS: UpdateUserInfo binds NULL for empty nickname/avatar/motto"
  exit 0
else
  echo "FAIL: UpdateUserInfo didn't set expected NULL fields"
  exit 2
fi
