#include <lyos/types.h>
#include <lyos/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <lyos/const.h>
#include <string.h>
#include <lyos/proc.h>
#include <lyos/global.h>
#include <lyos/proto.h>
#include <lyos/driver.h>
#include <errno.h>
#include <asm/ioctl.h>
#include <drm/drm.h>
#include <lyos/service.h>
#include <lyos/sysutils.h>

#include <libdevman/libdevman.h>

#include "libdrmdriver.h"
#include "proto.h"

int drm_plane_init(struct drm_device* dev, struct drm_plane* plane,
                   uint32_t possible_crtcs, enum drm_plane_type type)
{
    int retval;

    retval = drm_mode_object_add(dev, &plane->base, DRM_MODE_OBJECT_PLANE);
    if (retval) return retval;

    plane->possible_crtcs = possible_crtcs;
    plane->type = type;

    list_add_tail(&plane->head, &dev->mode_config.plane_list);
    plane->index = dev->mode_config.num_total_plane++;

    return 0;
}

void drm_helper_commit_planes(struct drm_device* dev,
                              struct drm_atomic_state* old_state)
{
    struct drm_plane* plane;
    int i;

    for (i = 0; i < dev->mode_config.num_total_plane; i++) {
        struct drm_plane_helper_funcs* funcs;

        plane = old_state->planes[i].ptr;
        funcs = plane->helper_funcs;

        funcs->update(plane, old_state->planes[i].old_state);
    }
}

int drm_mode_page_flip_ioctl(struct drm_device* dev, endpoint_t endpoint,
                             void* data)
{
    struct drm_mode_crtc_page_flip_target* page_flip = data;
    struct drm_crtc* crtc;
    struct drm_framebuffer* fb;

    if (page_flip->flags & ~DRM_MODE_PAGE_FLIP_FLAGS) return EINVAL;

    crtc = drm_crtc_lookup(dev, page_flip->crtc_id);
    if (!crtc) return ENOENT;

    fb = drm_framebuffer_lookup(dev, page_flip->fb_id);
    if (!fb) return ENOENT;

    return drm_atomic_page_flip(crtc, fb);
}