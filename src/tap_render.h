#ifndef TAP_RENDER_H
#define TAP_RENDER_H

#include "util/tap_def.h"
#include "util/tap_result.h"
#include "util/tap_face.h"

typedef struct {
    unsigned int viewport_width;
    unsigned int viewport_height;
    float rad_fovy;
    float z_near;
    float z_far;
} TapRenderInitOpts;

TapResult tap_render_init(TapRenderInitOpts meta);
TapResult tap_render_frame_setup(TapVec3 eye, TapVec3 center, TapVec3 up);
TapResult tap_render_frame_draw(const TapFace *faces, size_t num_faces);
TapResult tap_render_frame_get(const unsigned int **framebuffer);
TapResult tap_render_deinit(void);


#endif /* TAP_RENDER_H */
