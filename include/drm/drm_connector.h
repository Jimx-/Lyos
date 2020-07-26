#ifndef _DRM_DRM_CONNECTOR_H_
#define _DRM_DRM_CONNECTOR_H_

#include <stdint.h>
#include <lyos/list.h>
#include <drm/drm_mode_object.h>

struct drm_device;
struct drm_encoder;

struct drm_connector {
    struct list_head head;
    struct drm_mode_object base;
    unsigned index;

    int connector_type;
    int connector_type_id;

    uint32_t possible_encoders;
    struct drm_encoder* encoder;

    struct list_head modes;
};

int drm_connector_init(struct drm_device* dev, struct drm_connector* connector,
                       int connector_type);
int drm_connector_attach_encoder(struct drm_connector* connector,
                                 struct drm_encoder* encoder);

#define obj_to_connector(x) list_entry(x, struct drm_connector, base)

static inline struct drm_connector* drm_connector_lookup(struct drm_device* dev,
                                                         unsigned int id)
{
    struct drm_mode_object* mo;
    mo = drm_mode_object_find(dev, id, DRM_MODE_OBJECT_CONNECTOR);
    return mo ? obj_to_connector(mo) : NULL;
}

#define drm_for_each_connector(connector, dev) \
    list_for_each_entry(connector, &(dev)->mode_config.connector_list, head)

#endif
