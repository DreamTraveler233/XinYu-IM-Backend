-- 080_org.sql
-- 组织机构相关表（部门/职位/用户归属）

-- 组织部门（树形）父级可空 + 自引用外键
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

-- 职位字典
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

-- 用户-部门归属（一个主属部门 + 可选多部门）
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

-- 用户-职位映射（可多职位）
CREATE TABLE IF NOT EXISTS `im_org_user_position` (
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
  `position_id` BIGINT UNSIGNED NOT NULL COMMENT '职位ID（im_org_position.id）',
  `assigned_at` DATETIME NOT NULL COMMENT '授予时间',
  PRIMARY KEY (`user_id`,`position_id`),
  KEY `idx_position_user` (`position_id`,`user_id`),
  CONSTRAINT `fk_up_pos` FOREIGN KEY (`position_id`) REFERENCES `im_org_position` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `fk_up_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户-职位映射';
