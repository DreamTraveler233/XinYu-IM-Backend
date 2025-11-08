-- 030_group.sql
-- 群与群相关功能表

-- 群基础信息
CREATE TABLE IF NOT EXISTS `im_group` (
  `id` BIGINT UNSIGNED NOT NULL COMMENT '群ID（雪花ID/分布式ID）',
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

-- 群公告（当前生效的公告；如需历史可改为多版本+is_current）
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

-- 群申请（入群申请：创建/同意/拒绝/未读）
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

-- 可选：群邀请审计（邀请入群历史）
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
  `id` BIGINT UNSIGNED NOT NULL COMMENT '投票ID（雪花ID）',
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

-- 群投票选项
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

-- 群投票回答（多选：一人多行；单选：一人一行）
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
