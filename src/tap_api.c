#include "tap_api.h"
#include "tap_chunk.h"
#include "tap_render.h"

#include "util/tap_guard.h"

static struct {
    int initialized;
    float theta;
} state = {0};

TapResult tap_api_init(const TapApiInitOpt opt)
{
    TapResult result = {0};
    TapRenderInitOpts render_opts = {0};

    if (state.initialized)
        return result;
    if (!opt.allocator.free || !opt.allocator.malloc)
        TAP_GUARD_BAIL(TAP_ERRNO_INVALID_ALLOCATOR);
    if (!opt.logger.printf)
        TAP_GUARD_BAIL(TAP_ERRNO_INVALID_LOGGER);
    tap_alloc_set_allocator(opt.allocator);
    tap_log_set_logger(opt.logger);

    TAP_GUARD(tap_chunk_init());

    render_opts.viewport_width = opt.viewport_width;
    render_opts.viewport_height = opt.viewport_height;
    render_opts.rad_fovy = 90 * TAP_MATH_ANG2RAD;
    render_opts.z_near = 1.0F;
    render_opts.z_far = 100.0F;
    TAP_GUARD(tap_render_init(render_opts));

    state.theta = 0.0F;
    state.initialized = 1;

    return result;
}

TapResult tap_api_frame(const TapApiFramebuffer **out_framebuffer)
{
    TapResult result = {0};

    if (!out_framebuffer)
        TAP_GUARD_BAIL(TAP_ERRNO_NULLPTR);
    if (!state.initialized)
        TAP_GUARD_BAIL(TAP_ERRNO_MODULE_UNINITIALIZED);

    const TapVec3 up = {0, 1, 0};
    const TapVec3 eye = tap_vec3_rotate(tap_vec3_rotate((TapVec3){0, 5, 10}, (TapVec3){0, 1, 0}, state.theta), (TapVec3){1, 0, 0}, state.theta);

    TapVec3 center = {0,0,0}; 

    TAP_GUARD(tap_render_frame_setup(eye, center, up));

    const TapChunkMesh *meshes = NULL;
    size_t num_chunks = 0;
    TAP_GUARD(tap_chunk_get_meshes(center, &meshes, &num_chunks));

    for (size_t i = 0; i < num_chunks; ++i)
        TAP_GUARD(tap_render_frame_draw(meshes[i].faces, meshes[i].num_faces));

    TAP_GUARD(tap_render_frame_get(out_framebuffer));

    state.theta += 0.01F;

    return result;
}

TapResult tap_api_deinit(void)
{
    TapResult result = {0};
    TAP_GUARD(tap_render_deinit());
    TAP_GUARD(tap_chunk_deinit());
    state.initialized = 0;
    return result;
}
