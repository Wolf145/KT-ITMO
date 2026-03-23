#include "entrypoint.h"

#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/fs_types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/string.h>
#include <linux/types.h>

#include "http.h"
#include "util.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yaroshenko Danila");
MODULE_VERSION("0.01");

struct file_system_type networkfs_fs_type = {
    .name = "networkfs",
    .init_fs_context = networkfs_init_fs_context,
    .kill_sb = networkfs_kill_sb,
};
struct fs_context_operations networkfs_context_ops = {
    .get_tree = networkfs_get_tree,
};
struct inode_operations networkfs_inode_ops = {
    .lookup = networkfs_lookup,
    .create = networkfs_create,
    .unlink = networkfs_unlink,
    .mkdir = networkfs_mkdir,
    .rmdir = networkfs_rmdir,
};
struct file_operations networkfs_dir_ops = {
    .iterate_shared = networkfs_iterate,
};

int networkfs_iterate(struct file *filp, struct dir_context *ctx) {
  struct dentry *dentry = filp->f_path.dentry;
  struct inode *inode = dentry->d_inode;

  struct entries *response_buffer = kzalloc(sizeof(struct entries), GFP_KERNEL);
  if (response_buffer == NULL) {
    printk(KERN_ERR "networkfs_iterate: response_buffer allocation error");
    return -ENOMEM;
  }

  ino_to_string(ino_str, (ino_t)inode->i_ino);

  int64_t err = networkfs_http_call(
      (char *)inode->i_sb->s_fs_info, "list", (char *)response_buffer,
      sizeof(struct entries), 1, "inode", wstr(ino_str));
  if (err) {
    printk(KERN_ERR "networkfs_iterate: networkfs_http_call.list error: 0x%llx",
           (long long)-err);
    goto clean_networkfs_iterate;
  }

  loff_t record_counter = 0;
  for (; (size_t)ctx->pos < response_buffer->entries_count; ++ctx->pos) {
    struct entry current_entry = response_buffer->entries[(size_t)ctx->pos];
    dir_emit(ctx, current_entry.name, strlen(current_entry.name),
             current_entry.ino, current_entry.entry_type);
    ++record_counter;
  }
clean_networkfs_iterate:
  kfree(response_buffer);
  return (err ? 0 : record_counter);
}

struct dentry *networkfs_lookup(struct inode *parent, struct dentry *child,
                                unsigned int flag) {
  const char *name = child->d_name.name;

  struct entry_info *response_buffer =
      kzalloc(sizeof(struct entry_info), GFP_KERNEL);
  if (response_buffer == NULL) {
    printk(KERN_ERR "networkfs_lookup: response_buffer allocation error");
    return NULL;
  }

  ino_to_string(parent_ino_str, (ino_t)parent->i_ino);

  int64_t err =
      networkfs_http_call((char *)parent->i_sb->s_fs_info, "lookup",
                          (char *)response_buffer, sizeof(struct entry_info), 2,
                          "parent", wstr(parent_ino_str), "name", wstr(name));
  if (err) {
    printk(KERN_ERR
           "networkfs_lookup: networkfs_http_call.lookup error: 0x%llx",
           (long long)-err);
    goto clean_networkfs_lookup;
  }

  struct inode *inode = networkfs_get_inode(
      parent->i_sb, NULL,
      (response_buffer->entry_type == DT_DIR ? S_IFDIR : S_IFREG),
      response_buffer->ino);
  d_add(child, inode);
clean_networkfs_lookup:
  kfree(response_buffer);
  return (err ? NULL : child);
}

int networkfs_unlink(struct inode *parent, struct dentry *child) {
  const char *name = child->d_name.name;

  ino_to_string(parent_ino_str, (ino_t)parent->i_ino);

  int64_t err =
      networkfs_http_call((char *)parent->i_sb->s_fs_info, "unlink", NULL, 0, 2,
                          "parent", wstr(parent_ino_str), "name", wstr(name));
  if (err)
    printk(KERN_ERR
           "networkfs_unlink: networkfs_http_call.unlink error: 0x%llx",
           (long long)-err);

  return (int)err;
}

int networkfs_create(struct mnt_idmap *idmap, struct inode *parent,
                     struct dentry *child, umode_t mode, bool b) {
  const char *name = child->d_name.name;

  if (strlen(name) > 255) {
    printk(KERN_ERR "networkfs_create: name too long");
    return -ENAMETOOLONG;
  }

  ino_to_string(parent_ino_str, (ino_t)parent->i_ino);

  ino_t new_ino;
  int64_t err = networkfs_http_call((char *)parent->i_sb->s_fs_info, "create",
                                    (char *)&new_ino, sizeof(ino_t), 3,
                                    "parent", wstr(parent_ino_str), "name",
                                    wstr(name), "type", wstr("file"));
  if (err) {
    printk(KERN_ERR
           "networkfs_create: networkfs_http_call.create error: 0x%llx",
           (long long)-err);
    goto return_networkfs_create;
  }
  struct inode *inode =
      networkfs_get_inode(parent->i_sb, parent, S_IFREG, new_ino);
  if (inode != NULL) d_add(child, inode);
return_networkfs_create:
  return err;
}

int networkfs_rmdir(struct inode *parent, struct dentry *child) {
  const char *name = child->d_name.name;

  ino_to_string(parent_ino_str, (ino_t)parent->i_ino);

  int64_t err =
      networkfs_http_call((char *)parent->i_sb->s_fs_info, "rmdir", NULL, 0, 2,
                          "parent", wstr(parent_ino_str), "name", wstr(name));
  if (err)
    printk(KERN_ERR "networkfs_rmdir: networkfs_http_call.unlink error: 0x%llx",
           (long long)-err);

  return (int)err;
}

int networkfs_mkdir(struct mnt_idmap *idmap, struct inode *parent,
                    struct dentry *child, umode_t mode) {
  const char *name = child->d_name.name;

  if (strlen(name) > 255) {
    printk(KERN_ERR "networkfs_create: name too long");
    return -ENAMETOOLONG;
  }

  ino_to_string(parent_ino_str, (ino_t)parent->i_ino);

  ino_t new_ino;
  int64_t err = networkfs_http_call((char *)parent->i_sb->s_fs_info, "create",
                                    (char *)&new_ino, sizeof(ino_t), 3,
                                    "parent", wstr(parent_ino_str), "name",
                                    wstr(name), "type", wstr("directory"));
  if (err) {
    printk(KERN_ERR "networkfs_mkdir: networkfs_http_call.create error: 0x%llx",
           (long long)-err);
    goto return_networkfs_mkdir;
  }
  struct inode *inode =
      networkfs_get_inode(parent->i_sb, parent, S_IFDIR | 0x777, new_ino);
  if (inode != NULL) d_add(child, inode);
return_networkfs_mkdir:
  return err;
}

int networkfs_init_fs_context(struct fs_context *fc) {
  fc->ops = &networkfs_context_ops;
  return 0;
}

/**
 * @sb:     Суперблок файловой системы.
 * @parent: Родительская inode (NULL для корня ФС).
 * @mode:   Битовая маска из прав доступа и типа файла:
 * https://github.com/torvalds/linux/blob/v6.8/include/uapi/linux/stat.h#L9.
 * @i_ino:  Уникальный идентификатор inode.
 */
struct inode *networkfs_get_inode(struct super_block *sb,
                                  const struct inode *parent, umode_t mode,
                                  int i_ino) {
  struct inode *inode = new_inode(sb);

  if (inode != NULL) {
    inode->i_ino = i_ino;
    inode->i_op = &networkfs_inode_ops;
    inode->i_fop = &networkfs_dir_ops;
    inode_init_owner(&nop_mnt_idmap, inode, parent, mode);
  }

  return inode;
}

int networkfs_fill_super(struct super_block *sb, struct fs_context *fc) {
  // Создаём корневую inode
  struct inode *inode = networkfs_get_inode(sb, NULL, S_IFDIR | 0x777, 1000);
  // Создаём корень файловой системы
  sb->s_root = d_make_root(inode);
  if (sb->s_root == NULL) return -ENOMEM;

  sb->s_fs_info = kmalloc(strlen(fc->source) + 1, GFP_KERNEL);
  if (sb->s_fs_info == NULL) return -ENOMEM;
  strcpy(sb->s_fs_info, fc->source);

  return 0;
}

int networkfs_get_tree(struct fs_context *fc) {
  int ret = get_tree_nodev(fc, networkfs_fill_super);

  if (ret != 0) {
    printk(KERN_ERR "networkfs: unable to mount: error code %d", ret);
  }

  return ret;
}

void networkfs_kill_sb(struct super_block *sb) {
  printk(KERN_INFO "networkfs: superblock is destroyed, token: %s",
         (char *)sb->s_fs_info);
  kfree(sb->s_fs_info);
}

int networkfs_init(void) {
  int err = register_filesystem(&networkfs_fs_type);
  if (err) {
    printk(KERN_ERR "networkfs: Failed to register filesystem: %d", err);
    return err;
  }

  printk(KERN_INFO "Hello, World!\n");
  return 0;
}

void networkfs_exit(void) {
  int err = unregister_filesystem(&networkfs_fs_type);
  if (err != 0)
    printk(KERN_ERR "networkfs: Failed to unregister filesystem %d", err);
  printk(KERN_INFO "Goodbye!\n");
}

module_init(networkfs_init);
module_exit(networkfs_exit);
