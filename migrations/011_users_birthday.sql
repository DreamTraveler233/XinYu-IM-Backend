-- 011_users_birthday.sql
-- 为 users 表添加 birthday 字段（日期），支持幂等执行

-- 使用 information_schema 做存在性检查以兼容更多 MySQL 版本
SET @db_name = DATABASE();
SET @tbl = 'users';
SET @col = 'birthday';

SELECT COUNT(1) INTO @exists FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = @db_name AND TABLE_NAME = @tbl AND COLUMN_NAME = @col;

-- 如果不存在则添加
SET @sql = IF(@exists = 0,
  CONCAT('ALTER TABLE `', @tbl, '` ADD COLUMN `', @col, '` DATE NULL COMMENT "生日" AFTER `gender`;'),
  'SELECT "column exists";'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- 可选：记录操作（如果需要）

/*
注意：
- 该脚本在执行时依赖当前连接的默认数据库（DATABASE()）。
- 若运行环境没有默认 DB，请在执行前使用 USE <dbname> 或通过迁移脚本工具指定数据库。
*/
