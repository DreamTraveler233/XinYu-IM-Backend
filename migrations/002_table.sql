-- 创建数据库 im_db
CREATE DATABASE IF NOT EXISTS im_db
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
USE im_db;

--- 媒体文件表，存储已完成上传并可访问的文件元信息
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='媒体文件表，存储已完成上传并可访问的文件元信息';

--- 分片上传会话表，记录 multipart 上传的临时状态与统计信息
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='分片上传会话表，记录 multipart 上传的临时状态与统计信息';

-- 用户主档
CREATE TABLE IF NOT EXISTS `im_user` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '用户ID（由数据库自增）',
  `mobile` VARCHAR(20) NOT NULL COMMENT '手机号（唯一，用于登录/找回）',
  `email` VARCHAR(100) NULL COMMENT '邮箱（唯一，可选，用于找回/通知）',
  `nickname` VARCHAR(64) NOT NULL COMMENT '昵称',
  `avatar_media_id` CHAR(32) NOT NULL COMMENT '头像媒体文件ID（im_media_file.id）',
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
  KEY `idx_nickname` (`nickname`),
  CONSTRAINT `fk_user_avatar` FOREIGN KEY (`avatar_media_id`) REFERENCES `im_media_file` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
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

-- 登录日志
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

-- 联系人分组（每个用户自定义）
CREATE TABLE IF NOT EXISTS `im_contact_group` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '分组所属用户ID',
  `name` VARCHAR(32) NOT NULL COMMENT '分组名称',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序值（越小越靠前）',
  `contact_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '分组下联系人数量',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_user_name` (`user_id`,`name`) COMMENT '同用户下分组名唯一',
  KEY `idx_user_sort` (`user_id`,`sort`),
  CONSTRAINT `fk_cg_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='联系人分组（用户自定义）';

-- 联系人关系（定向：owner -> friend）
CREATE TABLE IF NOT EXISTS `im_contact` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `owner_user_id` BIGINT UNSIGNED NOT NULL COMMENT '联系人拥有者（发起展示视角的用户）',
  `friend_user_id` BIGINT UNSIGNED NOT NULL COMMENT '好友用户ID',
  `group_id` BIGINT UNSIGNED NULL COMMENT '联系人分组ID（im_contact_group.id）',
  `remark` VARCHAR(64) NULL COMMENT '我对该联系人的备注名',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '状态：1正常 2删除',
  `relation` TINYINT NOT NULL DEFAULT 1 COMMENT '我视角的关系：1=陌生人 2=好友 3=企业同事 4=本人',
  `is_block` TINYINT NOT NULL DEFAULT 0 COMMENT '是否拉黑：0=否 1=是（仅影响owner侧通信与展示）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间（成为好友时间）',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_owner_friend` (`owner_user_id`,`friend_user_id`) COMMENT '同一视角下唯一联系人',
  KEY `idx_owner_group` (`owner_user_id`,`group_id`),
  KEY `idx_friend` (`friend_user_id`),
  CONSTRAINT `fk_contact_group` FOREIGN KEY (`group_id`) REFERENCES `im_contact_group` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_contact_owner` FOREIGN KEY (`owner_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_contact_friend` FOREIGN KEY (`friend_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='联系人关系（按 owner 定向存备注/分组/黑名单）';

-- 好友申请
CREATE TABLE IF NOT EXISTS `im_contact_apply` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `apply_user_id` BIGINT UNSIGNED NOT NULL COMMENT '申请人用户ID',
  `target_user_id` BIGINT UNSIGNED NOT NULL COMMENT '被申请用户ID',
  `remark` VARCHAR(128) NULL COMMENT '申请附言（打招呼）',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=待处理 2=已同意 3=已拒绝 4=已撤回/删除',
  `handler_user_id` BIGINT UNSIGNED NULL COMMENT '处理人用户ID（通常等于 target_user_id）',
  `handle_remark` VARCHAR(128) NULL COMMENT '处理备注（可选）',
  `handled_at` DATETIME NULL COMMENT '处理时间（同意/拒绝）',
  `created_at` DATETIME NOT NULL COMMENT '申请时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_target_status_read` (`target_user_id`,`status`),
  KEY `idx_apply_time` (`apply_user_id`,`created_at`),
  KEY `idx_target_time` (`target_user_id`,`created_at`),
  CONSTRAINT `fk_capply_apply_user` FOREIGN KEY (`apply_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_capply_target_user` FOREIGN KEY (`target_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_capply_handler_user` FOREIGN KEY (`handler_user_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='联系人申请记录（申请/同意/拒绝/未读）';

-- 群基础信息
CREATE TABLE IF NOT EXISTS `im_group` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '群ID（由数据库自增）',
  `group_name` VARCHAR(64) NOT NULL COMMENT '群名称',
  `avatar` VARCHAR(255) NULL COMMENT '群头像URL',
  `profile` VARCHAR(255) NULL COMMENT '群简介/公告摘要（非正式公告）',
  `leader_id` BIGINT UNSIGNED NOT NULL COMMENT '群主用户ID（owner）',
  `creator_id` BIGINT UNSIGNED NOT NULL COMMENT '创建者用户ID（通常=leader_id）',
  `is_mute` TINYINT NOT NULL DEFAULT 2 COMMENT '全员禁言：1=开启 2=关闭',
  `is_overt` TINYINT NOT NULL DEFAULT 1 COMMENT '是否公开：1=关闭 2=开启（配合公开列表）',
  `overt_type` TINYINT NULL COMMENT '公开类型/分类（公开列表的 type）',
  `max_num` INT NOT NULL DEFAULT 500 COMMENT '最大成员数（公开列表展示用）',
  `member_num` INT NOT NULL DEFAULT 1 COMMENT '当前成员数（可冗余维护，或运行时统计）',
  `is_dismissed` TINYINT NOT NULL DEFAULT 0 COMMENT '是否已解散：0=否 1=是',
  `dismissed_at` DATETIME NULL COMMENT '解散时间（解散后不可用）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_leader` (`leader_id`),
  KEY `idx_overt_time` (`is_overt`,`created_at`),
  KEY `idx_name` (`group_name`),
  CONSTRAINT `fk_group_leader` FOREIGN KEY (`leader_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_group_creator` FOREIGN KEY (`creator_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群基础信息（详情/公开/禁言/成员数）';

-- 群成员
CREATE TABLE IF NOT EXISTS `im_group_member` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `group_id` BIGINT UNSIGNED NOT NULL COMMENT '群ID（im_group.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '成员用户ID',
  `role` TINYINT NOT NULL DEFAULT 1 COMMENT '角色：1=成员 2=管理员 3=群主',
  `visit_card` VARCHAR(64) NULL COMMENT '群名片（用户在该群内的昵称）',
  `no_speak_until` DATETIME NULL COMMENT '禁言到期时间（>NOW() 视为禁言中）',
  `joined_at` DATETIME NOT NULL COMMENT '加入时间',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间（退群/移除）',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_group_user` (`group_id`,`user_id`) COMMENT '群内成员唯一',
  KEY `idx_group_role` (`group_id`,`role`),
  KEY `idx_user_group` (`user_id`,`group_id`),
  CONSTRAINT `fk_member_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_member_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群成员（角色/群名片/成员禁言）';

-- 群公告
CREATE TABLE IF NOT EXISTS `im_group_notice` (
  `group_id` BIGINT UNSIGNED NOT NULL COMMENT '群ID（im_group.id）',
  `content` MEDIUMTEXT NOT NULL COMMENT '公告内容（富文本/纯文本）',
  `modify_user_id` BIGINT UNSIGNED NOT NULL COMMENT '最后修改人用户ID（群主/管理员）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`group_id`),
  CONSTRAINT `fk_notice_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_notice_user` FOREIGN KEY (`modify_user_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群公告（单条当前生效）';

-- 群申请
CREATE TABLE IF NOT EXISTS `im_group_apply` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `group_id` BIGINT UNSIGNED NOT NULL COMMENT '群ID（im_group.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '申请人用户ID',
  `remark` VARCHAR(128) NULL COMMENT '申请附言',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=待处理 2=已同意 3=已拒绝 4=已撤回/删除',
  `is_read` TINYINT NOT NULL DEFAULT 0 COMMENT '审核方是否已读：0=未读 1=已读（用于未读数）',
  `handler_user_id` BIGINT UNSIGNED NULL COMMENT '处理人用户ID（管理员/群主）',
  `handled_at` DATETIME NULL COMMENT '处理时间',
  `created_at` DATETIME NOT NULL COMMENT '申请时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_group_status_read` (`group_id`,`status`,`is_read`),
  KEY `idx_user_time` (`user_id`,`created_at`),
  CONSTRAINT `fk_gapply_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_gapply_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_gapply_handler` FOREIGN KEY (`handler_user_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群申请记录（待处理/同意/拒绝/未读）';

-- 群邀请审计
CREATE TABLE IF NOT EXISTS `im_group_invite` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `group_id` BIGINT UNSIGNED NOT NULL COMMENT '群ID',
  `inviter_id` BIGINT UNSIGNED NOT NULL COMMENT '邀请人用户ID',
  `invitee_id` BIGINT UNSIGNED NOT NULL COMMENT '被邀请用户ID',
  `status` TINYINT NOT NULL DEFAULT 2 COMMENT '状态：1=待接受 2=已加入 3=已拒绝 4=已取消（直接拉人可设为2）',
  `created_at` DATETIME NOT NULL COMMENT '邀请时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_group_invitee` (`group_id`,`invitee_id`),
  KEY `idx_inviter_time` (`inviter_id`,`created_at`),
  CONSTRAINT `fk_ginvite_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ginvite_inviter` FOREIGN KEY (`inviter_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ginvite_invitee` FOREIGN KEY (`invitee_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群邀请记录（可选：便于审计/风控）';

-- 群投票主表
CREATE TABLE IF NOT EXISTS `im_group_vote` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '投票ID（由数据库自增）',
  `group_id` BIGINT UNSIGNED NOT NULL COMMENT '群ID',
  `title` VARCHAR(128) NOT NULL COMMENT '投票标题',
  `answer_mode` TINYINT NOT NULL DEFAULT 1 COMMENT '回答模式：1=单选 2=多选（或扩展）',
  `is_anonymous` TINYINT NOT NULL DEFAULT 0 COMMENT '是否匿名：0=否 1=是',
  `created_by` BIGINT UNSIGNED NOT NULL COMMENT '创建者用户ID',
  `deadline_at` DATETIME NULL COMMENT '截止时间（可选）',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=进行中 2=已结束',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `idx_group_status_time` (`group_id`,`status`,`created_at` DESC),
  CONSTRAINT `fk_gvote_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_gvote_creator` FOREIGN KEY (`created_by`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群投票（标题/模式/匿名/状态）';

CREATE TABLE IF NOT EXISTS `im_group_vote_option` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `vote_id` BIGINT UNSIGNED NOT NULL COMMENT '投票ID（im_group_vote.id）',
  `opt_key` VARCHAR(32) NOT NULL COMMENT '选项键（作为提交时的选项ID）',
  `opt_value` VARCHAR(255) NOT NULL COMMENT '选项内容',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_vote_key` (`vote_id`,`opt_key`),
  KEY `idx_vote_sort` (`vote_id`,`sort`),
  CONSTRAINT `fk_gvoteopt_vote` FOREIGN KEY (`vote_id`) REFERENCES `im_group_vote` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群投票选项';

CREATE TABLE IF NOT EXISTS `im_group_vote_answer` (
  `vote_id` BIGINT UNSIGNED NOT NULL COMMENT '投票ID（im_group_vote.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '回答者用户ID',
  `opt_key` VARCHAR(32) NOT NULL COMMENT '选择的选项键',
  `answered_at` DATETIME NOT NULL COMMENT '回答时间',
  PRIMARY KEY (`vote_id`,`user_id`,`opt_key`),
  KEY `idx_vote_user` (`vote_id`,`user_id`),
  CONSTRAINT `fk_gvoteans_vote` FOREIGN KEY (`vote_id`) REFERENCES `im_group_vote` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_gvoteans_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群投票回答（多选多行）';

CREATE TABLE IF NOT EXISTS `im_talk` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '会话ID（由数据库自增）',
  `talk_mode` TINYINT NOT NULL COMMENT '会话类型: 1=单聊 2=群聊（对应 TalkModeEnum）',
  `user_min_id` BIGINT UNSIGNED NULL COMMENT '单聊: 两个用户ID中的较小者（用于唯一性约束）',
  `user_max_id` BIGINT UNSIGNED NULL COMMENT '单聊: 两个用户ID中的较大者（用于唯一性约束）',
  `group_id` BIGINT UNSIGNED NULL COMMENT '群聊: 群ID（用于唯一性约束）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_single` (`talk_mode`,`user_min_id`,`user_max_id`),
  UNIQUE KEY `uk_group`  (`talk_mode`,`group_id`),
  KEY `idx_mode` (`talk_mode`),
  CONSTRAINT `fk_talk_user_min` FOREIGN KEY (`user_min_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_talk_user_max` FOREIGN KEY (`user_max_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_talk_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='会话主实体（单聊/群聊唯一）';

CREATE TABLE IF NOT EXISTS `im_message` (
  `id` CHAR(32) NOT NULL COMMENT '消息ID（UUID/HEX32，无自增）',
  `talk_id` BIGINT UNSIGNED NOT NULL COMMENT '所属会话ID（im_talk.id）',
  `sequence` BIGINT NOT NULL COMMENT '会话内消息序号（严格递增，用于排序/分页）',
  `talk_mode` TINYINT NOT NULL COMMENT '会话类型: 1=单聊 2=群聊',
  `msg_type` SMALLINT NOT NULL COMMENT '消息类型（文本/图片/音视频/文件/位置/名片/转发/登录/投票/混合/公告/系统）',
  `sender_id` BIGINT UNSIGNED NOT NULL COMMENT '发送者用户ID',
  `receiver_id` BIGINT UNSIGNED NULL COMMENT '单聊: 对端用户ID；群聊: NULL',
  `group_id` BIGINT UNSIGNED NULL COMMENT '群聊: 群ID；单聊: NULL',
  `content_text` MEDIUMTEXT NULL COMMENT '文本/系统文本内容；非文本消息留空（由 extra 承载）',
  `extra` JSON NULL COMMENT '扩展内容：各类型专属字段',
  `quote_msg_id` CHAR(32) NULL COMMENT '被引用消息ID（用于回复/引用）',
  `is_revoked` TINYINT NOT NULL DEFAULT 2 COMMENT '撤回状态: 1=已撤回 2=正常',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '发送状态: 1=成功 2=发送中 3=失败',
  `revoke_by` BIGINT UNSIGNED NULL COMMENT '撤回操作人用户ID（可为空）',
  `revoke_time` DATETIME NULL COMMENT '撤回时间',
  `created_at` DATETIME NOT NULL COMMENT '发送时间（创建时间）',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_talk_seq` (`talk_id`,`sequence`) COMMENT '同一会话序号唯一',
  KEY `idx_talk_seq_desc` (`talk_id`,`sequence` DESC) COMMENT '按序号倒序分页',
  KEY `idx_sender_time` (`sender_id`,`created_at`) COMMENT '按发送者与时间检索',
  KEY `idx_receiver_time` (`receiver_id`,`created_at`) COMMENT '按单聊接收者与时间检索',
  KEY `idx_group_time` (`group_id`,`created_at`) COMMENT '按群与时间检索',
  KEY `idx_quote` (`quote_msg_id`) COMMENT '引用消息反查',
  CONSTRAINT `fk_msg_talk` FOREIGN KEY (`talk_id`) REFERENCES `im_talk` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_msg_sender` FOREIGN KEY (`sender_id`) REFERENCES `im_user` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_msg_receiver` FOREIGN KEY (`receiver_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_msg_group` FOREIGN KEY (`group_id`) REFERENCES `im_group` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_msg_quote` FOREIGN KEY (`quote_msg_id`) REFERENCES `im_message` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息主表（字符串ID）';

CREATE TABLE IF NOT EXISTS `im_talk_session` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '会话归属用户ID（谁在看这个会话）',
  `talk_id` BIGINT UNSIGNED NOT NULL COMMENT '对应 im_talk.id',
  `talk_mode` TINYINT NOT NULL COMMENT '会话类型: 1=单聊 2=群聊',
  `to_from_id` BIGINT UNSIGNED NOT NULL COMMENT '单聊: 对端用户ID；群聊: 群ID（用于 index_name 直达）',
  `is_disturb` TINYINT NOT NULL DEFAULT 2 COMMENT '免打扰: 1=开启 2=关闭',
  `is_top` TINYINT NOT NULL DEFAULT 2 COMMENT '置顶: 1=置顶 2=未置顶（前端最多18个）',
  `is_robot` TINYINT NOT NULL DEFAULT 2 COMMENT '是否机器人: 1=是 2=否',
  `unread_num` INT NOT NULL DEFAULT 0 COMMENT '未读消息数（本地缓存数，可通过 last_ack_seq 校准）',
  `last_ack_seq` BIGINT NOT NULL DEFAULT 0 COMMENT '上次確認已读的消息序号（会话内 seq）',
  `last_msg_id` CHAR(32) NULL COMMENT '最后一条消息ID（im_message.id，字符串）',
  `last_msg_type` SMALLINT NULL COMMENT '最后一条消息类型（对应 ChatMsgTypeXxx）',
  `last_msg_digest` VARCHAR(255) NULL COMMENT '最后一条消息预览文案（如[图片消息]、文本摘要）',
  `last_sender_id` BIGINT UNSIGNED NULL COMMENT '最后一条消息发送者ID',
  `draft_text` VARCHAR(1000) NULL COMMENT '草稿内容（富文本/纯文本皆可按需）',
  `name` VARCHAR(64) NULL COMMENT '名称快照（对端用户/群 名称）',
  `avatar` VARCHAR(255) NULL COMMENT '头像快照（对端用户/群 头像）',
  `remark` VARCHAR(64) NULL COMMENT '备注快照',
  `updated_at` DATETIME NOT NULL COMMENT '最后刷新时间（通常为最后消息时间）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间（用户从会话列表移除）',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_user_talk` (`user_id`,`talk_id`) COMMENT '每个用户对应的会话唯一',
  KEY `idx_user_updated` (`user_id`,`is_top`,`updated_at` DESC) COMMENT '会话列表排序：置顶优先、时间倒序',
  KEY `idx_user_unread` (`user_id`,`unread_num`) COMMENT '按未读筛选/统计',
  KEY `idx_user_indexname` (`user_id`,`talk_mode`,`to_from_id`) COMMENT 'index_name 定位：${talk_mode}_${to_from_id}',
  CONSTRAINT `fk_session_talk` FOREIGN KEY (`talk_id`) REFERENCES `im_talk` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_talk_session_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_session_last_msg` FOREIGN KEY (`last_msg_id`) REFERENCES `im_message` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_session_last_sender` FOREIGN KEY (`last_sender_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户-会话视图/设置（置顶/免打扰/未读/草稿/最后消息）';

CREATE TABLE IF NOT EXISTS `im_talk_sequence` (
  `talk_id` BIGINT UNSIGNED NOT NULL COMMENT 'im_talk.id',
  `last_seq` BIGINT NOT NULL DEFAULT 0 COMMENT '当前会话内最大序号',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`talk_id`),
  CONSTRAINT `fk_seq_talk` FOREIGN KEY (`talk_id`) REFERENCES `im_talk` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='会话序列（分配消息 sequence 用）';

CREATE TABLE IF NOT EXISTS `im_message_forward_map` (
  `forward_msg_id` CHAR(32) NOT NULL COMMENT '转发生成的新消息ID（im_message.id）',
  `src_msg_id` CHAR(32) NOT NULL COMMENT '被转发的原消息ID（im_message.id）',
  `src_talk_id` BIGINT UNSIGNED NOT NULL COMMENT '原消息所属会话ID（im_talk.id）',
  `src_sender_id` BIGINT UNSIGNED NOT NULL COMMENT '原消息发送者用户ID',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`forward_msg_id`,`src_msg_id`),
  KEY `idx_src` (`src_msg_id`),
  CONSTRAINT `fk_forward_msg` FOREIGN KEY (`forward_msg_id`) REFERENCES `im_message` (`id`) ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT `fk_forward_src_msg` FOREIGN KEY (`src_msg_id`) REFERENCES `im_message` (`id`) ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT `fk_forward_src_talk` FOREIGN KEY (`src_talk_id`) REFERENCES `im_talk` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息转发映射（字符串ID）';

CREATE TABLE IF NOT EXISTS `im_message_read` (
  `msg_id` CHAR(32) NOT NULL COMMENT '消息ID（im_message.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '阅读用户ID',
  `read_at` DATETIME NOT NULL COMMENT '阅读时间',
  PRIMARY KEY (`msg_id`,`user_id`),
  KEY `idx_user_time` (`user_id`,`read_at`),
  CONSTRAINT `fk_read_msg` FOREIGN KEY (`msg_id`) REFERENCES `im_message` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_read_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息已读表（字符串ID）';

CREATE TABLE IF NOT EXISTS `im_message_user_delete` (
  `msg_id` CHAR(32) NOT NULL COMMENT '消息ID（im_message.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '删除动作的用户ID',
  `deleted_at` DATETIME NOT NULL COMMENT '删除时间',
  PRIMARY KEY (`msg_id`,`user_id`),
  KEY `idx_user_deleted` (`user_id`,`deleted_at`),
  CONSTRAINT `fk_delete_msg` FOREIGN KEY (`msg_id`) REFERENCES `im_message` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_delete_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户侧消息删除记录（字符串ID）';

CREATE TABLE IF NOT EXISTS `im_message_mention` (
  `msg_id` CHAR(32) NOT NULL COMMENT '消息ID（im_message.id）',
  `mentioned_user_id` BIGINT UNSIGNED NOT NULL COMMENT '@到的用户ID',
  PRIMARY KEY (`msg_id`,`mentioned_user_id`),
  KEY `idx_mentioned` (`mentioned_user_id`,`msg_id`),
  CONSTRAINT `fk_mention_msg` FOREIGN KEY (`msg_id`) REFERENCES `im_message` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_mention_user` FOREIGN KEY (`mentioned_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='消息@提及映射（字符串ID）';

CREATE TABLE IF NOT EXISTS `im_emoticon` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '表情ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '归属用户ID',
  `url` VARCHAR(500) NOT NULL COMMENT '表情图片URL（或存储路径）',
  `name` VARCHAR(64) NULL COMMENT '表情名称/别名（可选）',
  `category` VARCHAR(64) NULL COMMENT '类别/分组名（可选，简单场景用文本分组）',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序（越小越靠前）',
  `is_enabled` TINYINT NOT NULL DEFAULT 1 COMMENT '是否启用：1=启用 0=禁用',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间',
  PRIMARY KEY (`id`),
  KEY `idx_user_sort` (`user_id`,`sort`),
  KEY `idx_user_deleted` (`user_id`,`deleted_at`),
  CONSTRAINT `fk_emoticon_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户自定义表情';

CREATE TABLE IF NOT EXISTS `im_article_classify` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '分类ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '归属用户ID',
  `class_name` VARCHAR(64) NOT NULL COMMENT '分类名称',
  `is_default` TINYINT NOT NULL DEFAULT 2 COMMENT '是否默认分类: 1=是 2=否（默认不能删除）',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序值（升序）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_user_classname` (`user_id`,`class_name`),
  KEY `idx_user_sort` (`user_id`,`sort`),
  CONSTRAINT `fk_aclass_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章分类';

CREATE TABLE IF NOT EXISTS `im_article_tag` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '标签ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '标签归属用户ID（个人标签域）',
  `tag_name` VARCHAR(64) NOT NULL COMMENT '标签名称',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_user_tagname` (`user_id`,`tag_name`),
  CONSTRAINT `fk_atag_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章标签维表（按用户隔离）';

CREATE TABLE IF NOT EXISTS `im_article` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '文章ID（由数据库自增）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '作者用户ID',
  `classify_id` BIGINT UNSIGNED NULL COMMENT '分类ID（im_article_classify.id，可为空表示未分类或默认）',
  `title` VARCHAR(200) NOT NULL COMMENT '标题',
  `abstract` VARCHAR(500) NULL COMMENT '摘要（可选，冗余加速列表展示）',
  `md_content` LONGTEXT NOT NULL COMMENT 'Markdown 内容（原文）',
  `image` VARCHAR(255) NULL COMMENT '封面图片URL',
  `is_asterisk` TINYINT NOT NULL DEFAULT 2 COMMENT '是否收藏: 1=收藏 2=未收藏（冗余，真实收藏看表 im_article_asterisk）',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态: 1=正常 2=草稿（可扩展）',
  `deleted_at` DATETIME NULL COMMENT '软删除时间（进入回收站）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '最后修改时间',
  PRIMARY KEY (`id`),
  KEY `idx_user_classify` (`user_id`,`classify_id`),
  KEY `idx_user_deleted` (`user_id`,`deleted_at`),
  KEY `idx_user_updated` (`user_id`,`updated_at` DESC),
  FULLTEXT KEY `ft_title_content` (`title`,`md_content`) WITH PARSER ngram COMMENT '全文索引用于关键字搜索（需MySQL8并配置ngram）',
  CONSTRAINT `fk_article_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_article_classify` FOREIGN KEY (`classify_id`) REFERENCES `im_article_classify` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章主表';

CREATE TABLE IF NOT EXISTS `im_article_tag_map` (
  `article_id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID',
  `tag_id` BIGINT UNSIGNED NOT NULL COMMENT '标签ID（im_article_tag.id）',
  PRIMARY KEY (`article_id`,`tag_id`),
  KEY `idx_tag_article` (`tag_id`,`article_id`),
  CONSTRAINT `fk_atag_article` FOREIGN KEY (`article_id`) REFERENCES `im_article` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_atag_tag` FOREIGN KEY (`tag_id`) REFERENCES `im_article_tag` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章与标签关系';

CREATE TABLE IF NOT EXISTS `im_article_asterisk` (
  `article_id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '收藏用户ID',
  `created_at` DATETIME NOT NULL COMMENT '收藏时间',
  PRIMARY KEY (`article_id`,`user_id`),
  KEY `idx_user_article` (`user_id`,`article_id`),
  CONSTRAINT `fk_ast_article` FOREIGN KEY (`article_id`) REFERENCES `im_article` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ast_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章收藏关系';

CREATE TABLE IF NOT EXISTS `im_article_annex` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '附件ID',
  `article_id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID（im_article.id）',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '上传者用户ID（通常等于文章作者）',
  `annex_name` VARCHAR(255) NOT NULL COMMENT '附件原始名称',
  `annex_size` BIGINT NOT NULL COMMENT '附件大小字节',
  `annex_path` VARCHAR(500) NOT NULL COMMENT '存储路径/URL',
  `mime_type` VARCHAR(128) NULL COMMENT 'MIME类型（可选）',
  `deleted_at` DATETIME NULL COMMENT '软删除时间（附件回收站）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间（上传时间）',
  PRIMARY KEY (`id`),
  KEY `idx_article_deleted` (`article_id`,`deleted_at`),
  KEY `idx_user_article` (`user_id`,`article_id`),
  CONSTRAINT `fk_aannex_article` FOREIGN KEY (`article_id`) REFERENCES `im_article` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_aannex_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章附件（软删除进入回收站）';

CREATE TABLE IF NOT EXISTS `im_org_department` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '部门ID',
  `parent_id` BIGINT UNSIGNED NULL COMMENT '上级部门ID（NULL 为顶级）',
  `dept_name` VARCHAR(64) NOT NULL COMMENT '部门名称',
  `ancestors` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '祖先链（如 /1/3/ ）便于快速查询子树',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序（同级内）',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间',
  PRIMARY KEY (`id`),
  KEY `idx_parent_sort` (`parent_id`,`sort`),
  KEY `idx_ancestors` (`ancestors`),
  CONSTRAINT `fk_org_dept_parent` FOREIGN KEY (`parent_id`) REFERENCES `im_org_department` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='组织部门（树形结构）';

CREATE TABLE IF NOT EXISTS `im_org_position` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '职位ID',
  `code` VARCHAR(64) NOT NULL COMMENT '职位编码（唯一）',
  `name` VARCHAR(64) NOT NULL COMMENT '职位名称',
  `sort` INT NOT NULL DEFAULT 0 COMMENT '排序',
  `created_at` DATETIME NOT NULL COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  `deleted_at` DATETIME NULL COMMENT '软删除时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_code` (`code`),
  KEY `idx_sort` (`sort`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='职位字典';

CREATE TABLE IF NOT EXISTS `im_org_user_department` (
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
  `dept_id` BIGINT UNSIGNED NOT NULL COMMENT '部门ID（im_org_department.id）',
  `is_primary` TINYINT NOT NULL DEFAULT 1 COMMENT '是否主属部门：1=是 0=否',
  `joined_at` DATETIME NOT NULL COMMENT '加入时间',
  PRIMARY KEY (`user_id`,`dept_id`),
  KEY `idx_dept_user` (`dept_id`,`user_id`),
  CONSTRAINT `fk_ud_dept` FOREIGN KEY (`dept_id`) REFERENCES `im_org_department` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `fk_ud_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户-部门归属';

CREATE TABLE IF NOT EXISTS `im_org_user_position` (
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
  `position_id` BIGINT UNSIGNED NOT NULL COMMENT '职位ID（im_org_position.id）',
  `assigned_at` DATETIME NOT NULL COMMENT '授予时间',
  PRIMARY KEY (`user_id`,`position_id`),
  KEY `idx_position_user` (`position_id`,`user_id`),
  CONSTRAINT `fk_up_pos` FOREIGN KEY (`position_id`) REFERENCES `im_org_position` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `fk_up_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户-职位映射';

-- End of merged tables
