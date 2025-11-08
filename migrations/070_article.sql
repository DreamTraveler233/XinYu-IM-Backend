-- 070_article.sql
-- 文章与附件相关表

-- 文章主表
CREATE TABLE IF NOT EXISTS `im_article` (
  `id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID（雪花ID）',
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

-- 文章分类
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

-- 文章标签维表（如标签不预设，可省略改用纯ID或名称表）
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

-- 文章-标签关系（多对多）
CREATE TABLE IF NOT EXISTS `im_article_tag_map` (
  `article_id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID',
  `tag_id` BIGINT UNSIGNED NOT NULL COMMENT '标签ID（im_article_tag.id）',
  PRIMARY KEY (`article_id`,`tag_id`),
  KEY `idx_tag_article` (`tag_id`,`article_id`),
  CONSTRAINT `fk_atag_article` FOREIGN KEY (`article_id`) REFERENCES `im_article` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_atag_tag` FOREIGN KEY (`tag_id`) REFERENCES `im_article_tag` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章与标签关系';

-- 文章收藏（冗余 is_asterisk 用于快速列表展示与去重）
CREATE TABLE IF NOT EXISTS `im_article_asterisk` (
  `article_id` BIGINT UNSIGNED NOT NULL COMMENT '文章ID',
  `user_id` BIGINT UNSIGNED NOT NULL COMMENT '收藏用户ID',
  `created_at` DATETIME NOT NULL COMMENT '收藏时间',
  PRIMARY KEY (`article_id`,`user_id`),
  KEY `idx_user_article` (`user_id`,`article_id`),
  CONSTRAINT `fk_ast_article` FOREIGN KEY (`article_id`) REFERENCES `im_article` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ast_user` FOREIGN KEY (`user_id`) REFERENCES `im_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章收藏关系';

-- 文章附件
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
