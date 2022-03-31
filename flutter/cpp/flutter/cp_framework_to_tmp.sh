#!/usr/bin/env sh

mkdir -p /tmp/ios_backend_fw_static_archive-root/
archive_root_path=${1}
framework_path=$(find ${archive_root_path} -type d -name '*.framework')
cp -a ${framework_path} /tmp/ios_backend_fw_static_archive-root/
