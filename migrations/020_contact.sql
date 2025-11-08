-- 020_contact.sql
-- 联系人相关表（联系人关系/分组/申请）

-- 联系人关系（定向：owner -> friend，存放个人视角信息）
CREATE TABLE IF NOT EXISTS `im_contact` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `owner_user_id` BIGINT UNSIGNED NOT NULL COMMENT '联系人拥有者（发起展示视角的用户）',
  `friend_user_id` BIGINT UNSIGNED NOT NULL COMMENT '好友用户ID',
  `group_id` BIGINT UNSIGNED NULL COMMENT '联系人分组ID（im_contact_group.id）',
  `remark` VARCHAR(64) NULL COMMENT '我对该联系人的备注名',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 1 COMMENT '状态：1正常 2删除',
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

-- 好友申请
CREATE TABLE IF NOT EXISTS `im_contact_apply` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
  `apply_user_id` BIGINT UNSIGNED NOT NULL COMMENT '申请人用户ID',
  `target_user_id` BIGINT UNSIGNED NOT NULL COMMENT '被申请用户ID',
  `remark` VARCHAR(128) NULL COMMENT '申请附言（打招呼）',
  `status` TINYINT NOT NULL DEFAULT 1 COMMENT '状态：1=待处理 2=已同意 3=已拒绝 4=已撤回/删除',
  `is_read` TINYINT NOT NULL DEFAULT 0 COMMENT '接收方是否已读：0=未读 1=已读（用于未读数）',
  `handler_user_id` BIGINT UNSIGNED NULL COMMENT '处理人用户ID（通常等于 target_user_id）',
  `handle_remark` VARCHAR(128) NULL COMMENT '处理备注（可选）',
  `handled_at` DATETIME NULL COMMENT '处理时间（同意/拒绝）',
  `created_at` DATETIME NOT NULL COMMENT '申请时间',
  `updated_at` DATETIME NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`),
  -- 保证“待处理”的唯一性，避免同一对重复提交（历史同意/拒绝记录不受影响）
  UNIQUE KEY `uk_pending_pair` (`apply_user_id`,`target_user_id`,`status`),
  KEY `idx_target_status_read` (`target_user_id`,`status`,`is_read`),
  KEY `idx_apply_time` (`apply_user_id`,`created_at`),
  KEY `idx_target_time` (`target_user_id`,`created_at`),
  CONSTRAINT `fk_capply_apply_user` FOREIGN KEY (`apply_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_capply_target_user` FOREIGN KEY (`target_user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_capply_handler_user` FOREIGN KEY (`handler_user_id`) REFERENCES `im_user` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='联系人申请记录（申请/同意/拒绝/未读）';
