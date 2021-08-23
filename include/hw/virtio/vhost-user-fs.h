/*
 * Vhost-user filesystem virtio device
 *
 * Copyright 2018-2019 Red Hat, Inc.
 *
 * Authors:
 *  Stefan Hajnoczi <stefanha@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or
 * (at your option) any later version.  See the COPYING file in the
 * top-level directory.
 */

#ifndef _QEMU_VHOST_USER_FS_H
#define _QEMU_VHOST_USER_FS_H

#include "hw/virtio/virtio.h"
#include "hw/virtio/vhost.h"
#include "hw/virtio/vhost-user.h"
#include "chardev/char-fe.h"
#include "qom/object.h"

#define TYPE_VHOST_USER_FS "vhost-user-fs-device"
OBJECT_DECLARE_SIMPLE_TYPE(VHostUserFS, VHOST_USER_FS)

/* Structures carried over the slave channel back to QEMU */
#define VHOST_USER_FS_SLAVE_MAX_ENTRIES 32

/* For the flags field of VhostUserFSSlaveMsg */
#define VHOST_USER_FS_FLAG_MAP_R (1u << 0)
#define VHOST_USER_FS_FLAG_MAP_W (1u << 1)

typedef struct {
    /* Offsets within the file being mapped */
    uint64_t fd_offset;
    /* Offsets within the cache */
    uint64_t c_offset;
    /* Lengths of sections */
    uint64_t len;
    /* Flags, from VHOST_USER_FS_FLAG_* */
    uint64_t flags;
} VhostUserFSSlaveMsgEntry;

typedef struct {
    /* Spare */
    uint32_t align32;
    /* Number of entries */
    uint16_t count;
    /* Spare */
    uint16_t align16;
} VhostUserFSSlaveMsgHdr;

/*
 * This is really a structure with a variable number of entries,
 * but we want to avoid a variable length array in the union,
 * so have one version with the variable length array
 * for places where we have the partial allocation.
 */
typedef struct {
    VhostUserFSSlaveMsgHdr hdr;
    VhostUserFSSlaveMsgEntry entries[];
} VhostUserFSSlaveMsg;

typedef struct {
    VhostUserFSSlaveMsgHdr hdr;
    VhostUserFSSlaveMsgEntry entries[VHOST_USER_FS_SLAVE_MAX_ENTRIES];
} VhostUserFSSlaveMsgMax;

typedef struct {
    CharBackend chardev;
    char *tag;
    uint16_t num_request_queues;
    uint16_t queue_size;
    uint64_t cache_size;
} VHostUserFSConf;

struct VHostUserFS {
    /*< private >*/
    VirtIODevice parent;
    VHostUserFSConf conf;
    struct vhost_virtqueue *vhost_vqs;
    struct vhost_dev vhost_dev;
    VhostUserState vhost_user;
    VirtQueue **req_vqs;
    VirtQueue *hiprio_vq;
    int32_t bootindex;

    /*< public >*/
    MemoryRegion cache;
};

/* Callbacks from the vhost-user code for slave commands */
uint64_t vhost_user_fs_slave_map(struct vhost_dev *dev, size_t message_size,
                                 VhostUserFSSlaveMsg *sm, int fd);
uint64_t vhost_user_fs_slave_unmap(struct vhost_dev *dev, size_t message_size,
                                   VhostUserFSSlaveMsg *sm);
uint64_t vhost_user_fs_slave_io(struct vhost_dev *dev, size_t message_size,
                                VhostUserFSSlaveMsg *sm, int fd);

#endif /* _QEMU_VHOST_USER_FS_H */
