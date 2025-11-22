-- 090_upload.sql
-- 媒体文件 & 分片上传相关表

-- 媒体文件表，存储已完成上传并可访问的文件元信息
CREATE TABLE IF NOT EXISTS `im_media_file` (
  `id` CHAR(32) NOT NULL PRIMARY KEY COMMENT '文件唯一ID，使用 uuid hex',
  `upload_id` VARCHAR(36) DEFAULT NULL COMMENT '关联上传会话ID，multipart 上传成功后回填',
  `user_id` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '上传用户ID',
  `file_name` VARCHAR(255) NOT NULL COMMENT '原始文件名（含后缀）',
  `file_size` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '文件大小，单位字节',
  `mime` VARCHAR(128) DEFAULT NULL COMMENT '文件 MIME 类型，例如 image/png',
  `storage_type` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '存储类型：1=LOCAL,2=S3 等',
  `storage_path` VARCHAR(512) NOT NULL COMMENT '存储路径或对象键：本地相对路径或 S3 Key',
  `url` VARCHAR(1024) DEFAULT NULL COMMENT '外部访问 URL（可包含 CDN 前缀），用于前端直接下载/展示',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '状态：1=可用，0=已删除/不可用',
  `created_at` DATETIME NOT NULL DEFAULT NOW() COMMENT '创建时间',
  `deleted_at` DATETIME DEFAULT NULL COMMENT '删除时间，NULL 表示未删除',
  `updated_at` DATETIME NOT NULL DEFAULT NOW() ON UPDATE NOW() COMMENT '最后更新时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='媒体文件表，存储已完成上传并可访问的文件元信息';

-- 分片上传会话表，记录 multipart 上传的临时状态与统计信息
CREATE TABLE IF NOT EXISTS `im_upload_session` (
  `upload_id` VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '分片上传会话ID（UUID）',
  `user_id` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '发起该上传的用户ID',
  `file_name` VARCHAR(255) NOT NULL COMMENT '原始文件名',
  `file_size` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '文件总大小，单位字节',
  `shard_size` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '每片大小，字节',
  `shard_num` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '分片总数',
  `uploaded_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '已接收的分片数量',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '会话状态：0=上传中，1=已完成，2=失败/取消',
  `temp_path` VARCHAR(512) DEFAULT NULL COMMENT '临时存放分片的目录（服务器本地路径）',
  `created_at` DATETIME NOT NULL DEFAULT NOW() COMMENT '会话创建时间',
  `updated_at` DATETIME NOT NULL DEFAULT NOW() ON UPDATE NOW() COMMENT '最后更新时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='分片上传会话表，记录 multipart 上传的临时状态与统计信息';