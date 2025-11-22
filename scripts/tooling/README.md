# Tooling scripts for IM-backend

## setup_media_dir.sh

This script ensures the media directory (`upload_base_dir`) and temporary directory are created and have correct permissions for the IM server to write uploaded files. It can also optionally create and set ownership for the Nginx `client_temp` cache directory so that Nginx can write request bodies (used for `multipart/form-data` uploads).

Usage:

```
# create upload dirs, set owner to current user (default)
sudo ./setup_media_dir.sh

# specify upload and temp dirs and IM runtime user (user that runs the im_server)
sudo ./setup_media_dir.sh --upload-dir /var/lib/im/media --temp-dir /var/lib/im/media/tmp --user im

# also create nginx cache directories and set owner
sudo ./setup_media_dir.sh --nginx-cache-dir /var/cache/nginx --nginx-user nginx
```

Why nginx cache dir matters

Nginx stores client request bodies in a temporary location (default `/var/cache/nginx/client_temp`). If the directory does not exist or Nginx does not have permission to write to it, Nginx will return a 500 error (see error logs with message "open() \"/var/cache/nginx/client_temp/...\" failed (13: Permission denied)"). When you run this script with `--nginx-cache-dir` and `--nginx-user`, it ensures the directory exists and will attempt to set the owner to `nginx`.

If you're running nginx inside Docker, consider creating and mounting a host directory and ensuring the container's nginx user has access to it. You may also run this command inside the container as part of the Docker entrypoint configuration.

Notes
- chown operations require root privileges; run commands with `sudo` if necessary.
- The script will detect whether the specified IM user or Nginx user exists, and skip chown if they don't.

Quick reproduction & fix steps (when you see 500 on upload):

1. Reproduce with curl and inspect verbose output:
```
curl -v -F 'file=@/path/to/test.png' http://localhost:9000/api/v1/upload/media-file
```

2. Tail nginx error log (this repo includes `scripts/docker/nginx.error.log` for example config):
```
tail -f scripts/docker/nginx.error.log
```

If you see lines like:
```
open() "/var/cache/nginx/client_temp/0000000001" failed (13: Permission denied)
```
then nginx cannot write client request bodies — create the cache dir and set ownership to the nginx user:

```
# find the nginx user (one of these)
grep -E '^[[:space:]]*user[[:space:]]+' /etc/nginx/nginx.conf || echo 'user nginx;'
ps aux | grep 'nginx: worker' | awk '{print $1}' | head -n1

# create and set ownership
sudo mkdir -p /var/cache/nginx/client_temp
sudo chown -R nginx:nginx /var/cache/nginx  # or use www-data for ubuntu/debian
sudo chmod -R 700 /var/cache/nginx

# restart nginx to pick up changes
sudo systemctl restart nginx
```

3. Re-run tests and re-run the upload command; you should observe an actual backend error if something else is wrong, or you will get a 200 with JSON response `{ id, src }` if success.

If you run Nginx in Docker, run `docker exec -it <nginx-container> bash` and run the commands inside the container or change your docker-compose volume and init statements.

