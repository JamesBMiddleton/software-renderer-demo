#include "tap_render.h"

#include "util/tap_assert.h"
#include "util/tap_guard.h"
#include "util/tap_mat.h"
#include "util/tap_math.h"
#include "util/tap_str.h"
#include "util/tap_vec.h"
#include "util/tap_arena.h"

typedef struct {
    TapVec3 points[6];
    TapVec3 normals[6];
} Frustum; 

static struct {
    int initialized;
    TapArena arena;
    unsigned int *framebuffer;
    float *zbuffer;
    Frustum frustum;
    struct {
        unsigned int viewport_width;
        unsigned int viewport_height;
        float rad_fovy;
        float z_near;
        float z_far;
        TapVec3 eye;
        TapVec3 center;
        TapVec3 up;
    } metadata;
} state = {0};

TapResult tap_render_init(TapRenderInitOpts meta)
{
    TapResult result = {0};
    if (state.initialized)
        return result;
    if (meta.rad_fovy < 0 || meta.rad_fovy > TAP_MATH_PI*2 || meta.z_near < 0.01F || meta.z_far < meta.z_near || meta.viewport_width == 0 || meta.viewport_height == 0)
        TAP_GUARD_BAIL(TAP_ERRNO_INVALID_VALUE);

    state.metadata.viewport_width = meta.viewport_width;
    state.metadata.viewport_height = meta.viewport_height;
    state.metadata.rad_fovy = meta.rad_fovy;
    state.metadata.z_near = meta.z_near;
    state.metadata.z_far = meta.z_far;

    state.arena = tap_arena_create(4096);
    state.framebuffer = tap_arena_alloc(&state.arena, unsigned int, (size_t)state.metadata.viewport_width * (size_t)state.metadata.viewport_height);
    state.zbuffer = tap_arena_alloc(&state.arena, float, (size_t)state.metadata.viewport_width * (size_t)state.metadata.viewport_height);

    state.initialized = 1;
    return result;
}


/* Perspective projection matrix. */
/* Right-handed (+z points towards camera), -1,1 NDC range. */
/* Maps to openGL's gluPerspective(), GLM's perspectiveRH_NO(). */
static TapMat4 projection_matrix_get(const float rad_fovy, const float aspect, const float z_near, const float z_far)
{
    TAP_ASSERT(rad_fovy > 0 && rad_fovy < TAP_MATH_PI*2);
    TAP_ASSERT(!TAP_MATH_EQUAL_FLOAT(aspect, 0));
    TAP_ASSERT(z_near > 0.01F);
    TAP_ASSERT(z_far > z_near);
    {
        const float a = tap_math_tan(rad_fovy / 2);
        const float b = 1 / (aspect / a);
        const float c = 1 / a;
        const float d = - (z_far + z_near) / (z_far - z_near);
        const float e = - (2 * z_far * z_near) / (z_far - z_near);
        const TapMat4 out = {
            b,0, 0,0,
            0,c, 0,0,
            0,0, d,e,
            0,0,-1,1,
        };
        return out;
    }
}

/* Affine transformation which converts our vertex from 'world frame' to 'camera frame'. */
/* Camera looks down the negative z axis. */
/* Maps to GLM's LookAtRH(), OpenGL's gluLookAt(). */
static TapMat4 view_matrix_get(const TapVec3 eye, const TapVec3 center, const TapVec3 up)
{
    const TapVec3 f = tap_vec3_normalize(tap_vec3_sub(center, eye));
    const TapVec3 s = tap_vec3_normalize(tap_vec3_cross(f, up));
    const TapVec3 u = tap_vec3_cross(s, f);
    const TapMat4 M = { 
          s.x,  s.y,  s.z, -tap_vec3_dot(s, eye),
          u.x,  u.y,  u.z, -tap_vec3_dot(u, eye),
         -f.x, -f.y, -f.z,  tap_vec3_dot(f, eye),
            0,    0,    0,                     1, 
    }; 
    return M;
}

static unsigned int phong_shader(const TapVec3 a, const TapVec3 b, const TapVec3 c, const TapVec3 lightsource, const TapColor base_color)
{
    const float ambient_term = 0.05F;
    const float diffuse_weight = 0.8F;
    const float specular_weight = 0.5F;
    const unsigned int e = 32; /* E. */
    const TapVec3 normal = tap_vec3_normalize(tap_vec3_cross(tap_vec3_sub(b, a), tap_vec3_sub(c, a)));
    const float diffuse_term = TAP_MATH_MAX(0.0F, tap_vec3_dot(normal, lightsource));

    const TapVec3 reflect = tap_vec3_normalize(tap_vec3_reflect(lightsource, normal));
    const float specular_term = tap_math_powf(TAP_MATH_MAX(0.0F, -reflect.z), (int)e);

    const float summed_terms = ambient_term + (diffuse_term * diffuse_weight) + (specular_term * specular_weight);

    const unsigned char red_channel = (unsigned char)tap_math_round(TAP_MATH_MIN(255.0F, (base_color & 0xFF) * summed_terms)); 
    const unsigned char green_channel = (unsigned char)tap_math_round(TAP_MATH_MIN(255.0F, (base_color >> 8U & 0xFF) * summed_terms)); 
    const unsigned char blue_channel = (unsigned char)tap_math_round(TAP_MATH_MIN(255.0F, (base_color >> 16U & 0xFF) * summed_terms)); 

    return ((unsigned int)red_channel) | ((unsigned int)green_channel << 8U)  | ((unsigned int)blue_channel << 16U)  | (255U << 24U);
}

static void rasterize(const TapVec4 a_clip, const TapVec4 b_clip, const TapVec4 c_clip, const unsigned int viewport_width, const unsigned int viewport_height, 
                      const unsigned int color, unsigned int *framebuffer, float *zbuffer)
{
    TAP_ASSERT(framebuffer);
    TAP_ASSERT(zbuffer);
    {
        /* Perspective divide */
        const TapVec3 a_ndc = tap_vec4_dehomogenize(a_clip);
        const TapVec3 b_ndc = tap_vec4_dehomogenize(b_clip);
        const TapVec3 c_ndc = tap_vec4_dehomogenize(c_clip);

        const float x_scale = (float)viewport_width/2.0F;
        const float x_translate = (float)viewport_width/2.0F;
        const float y_scale = (float)viewport_height/2.0F;
        const float y_translate = (float)viewport_height/2.0F;
        const TapVec3 a_screen = {(a_ndc.x * x_scale) + x_translate, (a_ndc.y * y_scale) + y_translate, a_ndc.z};
        const TapVec3 b_screen = {(b_ndc.x * x_scale) + x_translate, (b_ndc.y * y_scale) + y_translate, b_ndc.z};
        const TapVec3 c_screen = {(c_ndc.x * x_scale) + x_translate, (c_ndc.y * y_scale) + y_translate, c_ndc.z};

        /* The matrix we'll use for barycentric calcs. */
        const TapMat3 ABC = { 
            a_screen.x, a_screen.y, 1,
            b_screen.x, b_screen.y, 1, 
            c_screen.x, c_screen.y, 1,
        }; 

        /* backface culling; determinate gives us the area of the triangle made by points a,b,c. if the area is negative, triangle is back facing. */
        /* less than 1 pixel culling; not worth drawing. */
        if (tap_mat3_det(ABC) < 1) 
            return;

        /* Triangle rasterization using zbuffered bounding box method. */
        {
            int x = 0;
            int y = 0;
            const int bbox_max_x = (int)tap_math_round(TAP_MATH_MAX(a_screen.x, TAP_MATH_MAX(b_screen.x, c_screen.x)));
            const int bbox_max_y = (int)tap_math_round(TAP_MATH_MAX(a_screen.y, TAP_MATH_MAX(b_screen.y, c_screen.y)));
            const int bbox_min_x = (int)tap_math_round(TAP_MATH_MIN(a_screen.x, TAP_MATH_MIN(b_screen.x, c_screen.x)));
            const int bbox_min_y = (int)tap_math_round(TAP_MATH_MIN(a_screen.y, TAP_MATH_MIN(b_screen.y, c_screen.y)));

            for (x = bbox_min_x; x < bbox_max_x; ++x)
            {
                for (y = bbox_min_y; y < bbox_max_y; ++y)
                {
                    const float z_bias = 0.003F; /* Prevent Z-fighting. */
                    const unsigned int buf_index = (unsigned int)x + ((unsigned int)y*viewport_width);
                    float z = {0};
                    TapVec3 P = {0}; /* Our pixel coords. */
                    TapVec3 bary = {0}; /* x=alpha, y=beta, z=gamma */

                    P.x = (float)x; P.y = (float)y; P.z = 1.0F;
                    bary = tap_mat3_mulv(tap_mat3_transpose(tap_mat3_invert(ABC)), P); /* See bary.c for details. */

                    /* Check if we're in the triangle. */ 
                    if (bary.x < 0 || bary.y < 0 || bary.z < 0) 
                        continue;

                    /* Check whether the pixel is obscured by another we've already drawn with a lower z value. -1 = z_near, +1 = z_far */
                    z = (a_screen.z * bary.x) + (b_screen.z * bary.y) + (c_screen.z * bary.z); 
                    if (zbuffer[buf_index] < (z-z_bias))
                        continue;

                    zbuffer[buf_index] = z;

                    framebuffer[buf_index] = color;
                }
            }
        }
    }
}

TapResult tap_render_frame_setup(TapVec3 eye, TapVec3 center, TapVec3 up)
{
    TapResult result = {0};

    if (!state.initialized)
        TAP_GUARD_BAIL(TAP_ERRNO_MODULE_UNINITIALIZED);

    for (unsigned int i = 0; i < (state.metadata.viewport_width * state.metadata.viewport_height); ++i)
    {
        state.framebuffer[i] = 0xFF16110E; /* Initialize all pixels in the frame to github dark theme black. */
        state.zbuffer[i] = 2; /* z_far=1 in NDC, so we need to start our z buffer beyond that. */
    }

    state.metadata.eye = eye;
    state.metadata.center = center;
    state.metadata.up = up;

    return result;
}

TapResult tap_render_frame_draw(const TapFace *faces, size_t num_faces)
{
    TapResult result = {0};

    const float ratio = (float)state.metadata.viewport_width / (float)state.metadata.viewport_height;
    const TapMat4 projection_mat = projection_matrix_get(state.metadata.rad_fovy, ratio, state.metadata.z_near, state.metadata.z_far);
    const TapMat4 eye_view_mat = view_matrix_get(state.metadata.eye, state.metadata.center, state.metadata.up);

    const TapVec3 lightsource_eye = {0,0.5F,1};

    if (!faces)
        TAP_GUARD_BAIL(TAP_ERRNO_NULLPTR);
    if (!state.initialized)
        TAP_GUARD_BAIL(TAP_ERRNO_MODULE_UNINITIALIZED);

    for (size_t i = 0; i < num_faces; ++i)
    {
        const TapVec4 a_world = tap_vec3_homogenize(faces[i].vertices[0]);
        const TapVec4 b_world = tap_vec3_homogenize(faces[i].vertices[1]);
        const TapVec4 c_world = tap_vec3_homogenize(faces[i].vertices[2]);

        const TapVec4 a_eye = tap_mat4_mulv(eye_view_mat, a_world);
        const TapVec4 b_eye = tap_mat4_mulv(eye_view_mat, b_world);
        const TapVec4 c_eye = tap_mat4_mulv(eye_view_mat, c_world);

        const TapVec4 a_clip = tap_mat4_mulv(projection_mat, a_eye);
        const TapVec4 b_clip = tap_mat4_mulv(projection_mat, b_eye);
        const TapVec4 c_clip = tap_mat4_mulv(projection_mat, c_eye);

        const TapColor face_color = phong_shader(tap_vec4_truncate(a_eye), tap_vec4_truncate(b_eye), tap_vec4_truncate(c_eye), 
                                tap_vec3_normalize(lightsource_eye), faces[i].color);

        /* Currently just rejecting anything partially outside clip space, need to clip them (create new faces) */ 
        if (-a_clip.w <= a_clip.x && a_clip.x <= a_clip.w && -a_clip.w <= a_clip.y && a_clip.y <= a_clip.w && -a_clip.w <= a_clip.z && a_clip.z <= a_clip.w &&
            -b_clip.w <= b_clip.x && b_clip.x <= b_clip.w && -b_clip.w <= b_clip.y && b_clip.y <= b_clip.w && -b_clip.w <= b_clip.z && b_clip.z <= b_clip.w &&
            -c_clip.w <= c_clip.x && c_clip.x <= c_clip.w && -c_clip.w <= c_clip.y && c_clip.y <= c_clip.w && -c_clip.w <= c_clip.z && c_clip.z <= c_clip.w)
            rasterize(a_clip, b_clip, c_clip, state.metadata.viewport_width, state.metadata.viewport_height, face_color, state.framebuffer, state.zbuffer);
    }
    return result;
}

TapResult tap_render_frame_get(const unsigned int **framebuffer)
{
    TapResult result = {0};
    if (!state.initialized)
        TAP_GUARD_BAIL(TAP_ERRNO_MODULE_UNINITIALIZED);

    *framebuffer = state.framebuffer;

    return result;
}

TapResult tap_render_deinit(void)
{
    TapResult result = {0};
    tap_arena_destroy(&state.arena);
    state.initialized = 0;
    return result;
}
