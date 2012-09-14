/*
   drbd_sysfs.c

   This file is part of DRBD.

   Copyright (C) 2001-2008, LINBIT Information Technologies GmbH.
   Copyright (C) 1999-2008, Philipp Reisner <philipp.reisner@linbit.com>.
   Copyright (C) 2002-2008, Lars Ellenberg <lars.ellenberg@linbit.com>.

   drbd is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   drbd is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with drbd; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include "linux/kobject.h"
#include <linux/drbd.h>
#include "drbd_int.h"

struct drbd_md_attribute {
	struct attribute attr;
	ssize_t (*show)(struct drbd_backing_dev *bdev, char *buf);
	/* ssize_t (*store)(struct drbd_backing_dev *bdev, const char *buf, size_t count); */
};

STATIC ssize_t drbd_md_attr_show(struct kobject *, struct attribute *, char *);
static ssize_t current_show(struct drbd_backing_dev *, char *);
static void backing_dev_release(struct kobject *kobj);

struct kobj_type drbd_bdev_kobj_type = {
	.release = backing_dev_release,
	.sysfs_ops = &(struct sysfs_ops) {
		.show = drbd_md_attr_show,
		.store = NULL,
	},
};

#define DRBD_MD_ATTR(_name) struct drbd_md_attribute drbd_md_attr_##_name = __ATTR_RO(_name)

/* since "current" is a macro, the expansion of DRBD_MD_ATTR(current) does not work: */
static struct drbd_md_attribute drbd_md_attr_current = {
	.attr = { .name = "current", .mode = 0444 },
	.show = current_show,
};

static struct attribute *drbd_md_attrs[] = {
	&drbd_md_attr_current.attr,
	NULL,
};

struct attribute_group drbd_md_attr_group = {
	.attrs = drbd_md_attrs,
	.name = "data_gen_id",
};

STATIC ssize_t drbd_md_attr_show(struct kobject *kobj, struct attribute *attr, char *buffer)
{
	struct drbd_backing_dev *bdev = container_of(kobj, struct drbd_backing_dev, kobject);
	struct drbd_md_attribute *drbd_md_attr = container_of(attr, struct drbd_md_attribute, attr);

	return drbd_md_attr->show(bdev, buffer);
}

static ssize_t current_show(struct drbd_backing_dev *bdev, char *buf)
{
	ssize_t size = 0;

	size = sprintf(buf, "0x%016llX\n", bdev->md.current_uuid);

	return size;
}

static void backing_dev_release(struct kobject *kobj)
{
	struct drbd_backing_dev *bdev = container_of(kobj, struct drbd_backing_dev, kobject);
	kfree(bdev);
}