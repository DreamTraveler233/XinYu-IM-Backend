-- 010_user_auth.sql
-- 用户 & 认证 & 验证相关表

-- 用户主档
CREATE TABLE IF NOT EXISTS `im_user` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '用户ID（由数据库自增）',
  `mobile` VARCHAR(20) NOT NULL COMMENT '手机号（唯一，用于登录/找回）',
  `email` VARCHAR(100) NULL COMMENT '邮箱（唯一，可选，用于找回/通知）',
  `nickname` VARCHAR(64) NOT NULL COMMENT '昵称',
  `avatar` VARCHAR(255) NULL COMMENT '头像URL',
  `avatar_media_id` CHAR(32) NULL COMMENT '头像文件ID',
  `gender` TINYINT NOT NULL DEFAULT 0 COMMENT '性别：0=未知 1=男 2=女',
  `motto` VARCHAR(255) NULL COMMENT '个性签名',
  `birthday` DATE NULL COMMENT '生日 YYYY-MM-DD',
  `online_status` VARCHAR(1) NOT NULL DEFAULT 'N' COMMENT '在线状态：N:离线 Y:在线',
  `last_online_at` DATETIME NULL COMMENT '最后在线时间',
  `is_qiye` TINYINT NOT NULL DEFAULT 0 COMMENT '是否企业用户：0=否 1=是（用于 UserSettingResponse.user_info）',
  `is_robot` TINYINT NOT NULL DEFAULT 0 COMMENT '是否机器人账号：0=否 1=是',
  `is_disabled` TINYINT NOT NULL DEFAULT 0 COMMENT '是否禁用：0=否 1=是（冻结/封禁）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_mobile` (`mobile`),
  UNIQUE KEY `uk_email` (`email`),
  KEY `idx_nickname` (`nickname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户主档';

-- 本地账号密码
CREATE TABLE IF NOT EXISTS `im_user_auth` (
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '用户ID（im_user.id）',
  `password_hash` VARCHAR(255) NOT NULL COMMENT '密码哈希（bcrypt/argon2等）',
  `password_algo` VARCHAR(255) NOT NULL DEFAULT 'PBKDF2-HMAC-SHA256' COMMENT '哈希算法（bcrypt/argon2等）',
  `password_version` SMALLINT NOT NULL DEFAULT 1 COMMENT '密码版本（升级算法时便于区分）',
  `last_reset_at` DATETIME NULL COMMENT '最近重置时间（找回/修改）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `fk_auth_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户本地认证信息（密码等）';

-- 用户个性化设置（与前端类型对齐）
CREATE TABLE IF NOT EXISTS `im_user_setting` (
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '用户ID（im_user.id）',
  `theme_mode` VARCHAR(16) NOT NULL DEFAULT 'auto' COMMENT '主题模式：light/dark/auto',
  `theme_bag_img` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '聊天背景图片URL',
  `theme_color` VARCHAR(32) NOT NULL DEFAULT '' COMMENT '主题主色调（#RRGGBB）',
  `notify_cue_tone` VARCHAR(16) NOT NULL DEFAULT "N" COMMENT '通知提示音开关：Y是 N否',
  `keyboard_event_notify` VARCHAR(16) NOT NULL DEFAULT "N" COMMENT '键盘事件通知：Y是 N否',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `fk_setting_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户个性化设置';

-- 登录日志（驱动登录提醒消息；便于风控）
CREATE TABLE IF NOT EXISTS `im_auth_login_log` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `user_id` BIGINT UNSIGNED NULL COMMENT '用户ID（登录前失败场景可能为空）',
  `mobile` VARCHAR(20) NULL COMMENT '尝试登录的手机号（便于失败审计）',
  `platform` VARCHAR(32) NOT NULL COMMENT '登录平台（来自请求）',
  `ip` VARCHAR(45) NOT NULL COMMENT '登录IP（IPv4/IPv6）',
  `address` VARCHAR(128) NULL COMMENT 'IP归属地（解析后）',
  `user_agent` VARCHAR(255) NULL COMMENT 'UA/设备指纹',
  `success` TINYINT NOT NULL DEFAULT 0 COMMENT '是否成功：1=成功 0=失败',
  `reason` VARCHAR(128) NULL COMMENT '失败原因（密码错误/验证码错误/账号禁用等）',
  `created_at` DATETIME NOT NULL COMMENT '记录时间',
  PRIMARY KEY (`id`),
  KEY `idx_user_time` (`user_id`,`created_at`),
  KEY `idx_mobile_time` (`mobile`,`created_at`),
  CONSTRAINT `fk_loginlog_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='认证登录日志';

-- 短信验证码记录
CREATE TABLE IF NOT EXISTS `im_sms_verify_code` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `mobile` VARCHAR(20) NOT NULL COMMENT '手机号（目标）',
  `channel` VARCHAR(255) NOT NULL COMMENT '场景：register/login/forget/update_mobile/update_email/oauth_bind 等',
  `code` VARCHAR(12) NOT NULL COMMENT '验证码',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=未使用 2=已使用 3=已过期',
  `sent_ip` VARCHAR(45) NULL COMMENT '发送请求IP',
  `sent_at` DATETIME NOT NULL COMMENT '发送时间',
  `expire_at` DATETIME NOT NULL COMMENT '过期时间',
  `used_at` DATETIME NULL COMMENT '使用时间',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_target_scene` (`mobile`,`channel`,`status`),
  KEY `idx_expire` (`expire_at`),
  KEY `idx_sent_at` (`sent_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='短信验证码记录';

-- 邮箱验证码表
CREATE TABLE IF NOT EXISTS `im_email_verify_code` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `email` VARCHAR(100) NULL COMMENT '邮箱（目标）',
  `channel` VARCHAR(255) NOT NULL COMMENT '场景：register/login/forget/update_mobile/update_email/oauth_bind 等',
  `code` VARCHAR(12) NOT NULL COMMENT '验证码',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=未使用 2=已使用 3=已过期',
  `sent_ip` VARCHAR(45) NULL COMMENT '发送请求IP',
  `sent_at` DATETIME NOT NULL COMMENT '发送时间',
  `expire_at` DATETIME NOT NULL COMMENT '过期时间',
  `used_at` DATETIME NULL COMMENT '使用时间',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `idx_target_scene` (`email`,`channel`,`status`),
  KEY `idx_expire` (`expire_at`),
  KEY `idx_sent_at` (`sent_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='邮箱验证码记录';