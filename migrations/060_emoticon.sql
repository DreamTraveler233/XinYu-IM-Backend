-- 060_emoticon.sql
-- 自定义表情

-- 自定义表情主表（每个用户维护自己的表情）
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
