#include "tap_chunk.h"

#include "util/tap_arena.h"
#include "util/tap_guard.h"
#include "util/tap_vec.h"
#include "util/tap_list.h"

#define R_DISTANCE 1
TAP_ASSERT_STATIC(R_DISTANCE % 2 == 1, render_distance_odd);

typedef enum {
    BLOCKTYPE_AIR,
    BLOCKTYPE_RED,
    BLOCKTYPE_BLUE,
    BLOCKTYPE_WHITE,
    BLOCKTYPE_SIZE
} BlockType;

typedef struct {
    BlockType type;
} Block;

typedef struct {
    TapVec3I coords;
    Block blocks[CHUNK_SZ][CHUNK_SZ][CHUNK_SZ];
    TapList free_node;
} Chunk;

typedef struct {
    Chunk *chunks;
    TapChunkMesh *meshes;
    size_t size;
    TapList free_anchor;
} ChunkPool;
    
static struct {
    int initialized;
    TapArena arena;
    ChunkPool chunk_pool;
} state = {0};

static void chunk_load(Chunk *chunk, const char *mapfile)
{
    size_t i_x, i_y, i_z;
    for (i_x = 0; i_x < CHUNK_SZ; ++i_x)
        for (i_y = 0; i_y < CHUNK_SZ; ++i_y)
            for (i_z = 0; i_z < CHUNK_SZ; ++i_z)
                chunk->blocks[i_x][i_y][i_z].type = (((i_x+i_y+i_z) % 2) == 0) ? BLOCKTYPE_WHITE : BLOCKTYPE_RED;

    (void)mapfile; /* for now, we're just randomly generating chunks, we would be using chunk->coords to load the chunk from disk. */
}

static void chunk_mesh_update(const Chunk *chunk, TapChunkMesh *mesh)
{
    const TapColor blocktype_colors[] = {TAP_COLOR_WHITE, TAP_COLOR_RED, TAP_COLOR_BLUE, TAP_COLOR_WHITE};
    const TapVec3 unit_voxel_vertices[8] = {
        {0, 0, 0}, /* */
        {1, 0, 0}, /* +x */
        {0, 1, 0}, /* +y */
        {0, 0, 1}, /* +z */
        {1, 1, 0}, /* +x, +y */
        {1, 0, 1}, /* +x, +z */
        {0, 1, 1}, /* +y, +z */
        {1, 1, 1}, /* +x, +y, +z */
    };

    /* x,y,z faces wound clockwise, x+1,y+1,z+1 faces wound anticlockwise (backfacing) */
    const size_t face_indexes[12][3] = {
        {0, 3, 6}, {6, 2, 0}, /* x face */
        {0, 1, 5}, {5, 3, 0}, /* y face */
        {0, 2, 4}, {4, 1, 0}, /* z face */
        {7, 5, 1}, {1, 4, 7}, /* x+1 face */
        {7, 4, 2}, {2, 6, 7}, /* y+1 face */
        {7, 6, 3}, {3, 5, 7}, /* z+1 face */
    };

    size_t i_x, i_y, i_z, i;
    mesh->num_faces = 0;
    for (i_x = 0; i_x < CHUNK_SZ; ++i_x)
        for (i_y = 0; i_y < CHUNK_SZ; ++i_y)
            for (i_z = 0; i_z < CHUNK_SZ; ++i_z)
                if (chunk->blocks[i_x][i_y][i_z].type != BLOCKTYPE_AIR)
                {
                    TapVec3 voxel_vertices[8];

                    for (i = 0; i < TAP_ARRAY_SIZE(voxel_vertices); ++i)
                    {
                        const TapVec3 chunk_offset = {(chunk->coords.x * CHUNK_SZ)-(CHUNK_SZ/2), 
                                                      (chunk->coords.y * CHUNK_SZ)-(CHUNK_SZ/2), 
                                                      (chunk->coords.z * CHUNK_SZ)-(CHUNK_SZ/2)};
                        const TapVec3 voxel_offset = {i_x, i_y, i_z};
                        voxel_vertices[i] = tap_vec3_add(tap_vec3_add(unit_voxel_vertices[i], voxel_offset), chunk_offset);
                    }

                    if (i_x == 0 || chunk->blocks[i_x-1][i_y][i_z].type == BLOCKTYPE_AIR)
                        for (i = 0; i < 2; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                    if (i_y == 0 || chunk->blocks[i_x][i_y-1][i_z].type == BLOCKTYPE_AIR)
                        for (i = 2; i < 4; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                    if (i_z == 0 || chunk->blocks[i_x][i_y][i_z-1].type == BLOCKTYPE_AIR)
                        for (i = 4; i < 6; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                    if (i_x == CHUNK_SZ-1 || chunk->blocks[i_x+1][i_y][i_z].type == BLOCKTYPE_AIR)
                        for (i = 6; i < 8 ; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                    if (i_y == CHUNK_SZ-1 || chunk->blocks[i_x][i_y+1][i_z].type == BLOCKTYPE_AIR)
                        for (i = 8; i < 10; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                    if (i_z == CHUNK_SZ-1 || chunk->blocks[i_x][i_y][i_z+1].type == BLOCKTYPE_AIR)
                        for (i = 10; i < 12; ++i)
                        {
                            TapFace *face = &(mesh->faces[mesh->num_faces++]);
                            face->color = blocktype_colors[chunk->blocks[i_x][i_y][i_z].type];
                            face->vertices[0] = voxel_vertices[face_indexes[i][0]]; 
                            face->vertices[1] = voxel_vertices[face_indexes[i][1]]; 
                            face->vertices[2] = voxel_vertices[face_indexes[i][2]]; 
                        }
                }
}

static void chunkpool_init(TapArena *arena, ChunkPool *pool)
{
    pool->size = (R_DISTANCE) * (R_DISTANCE) * (R_DISTANCE);
    pool->chunks = tap_arena_alloc(arena, Chunk, pool->size);
    pool->meshes = tap_arena_alloc(arena, TapChunkMesh, pool->size);
    tap_list_init(&pool->free_anchor);
    for (size_t i = 0; i < pool->size; ++i)
    {
        tap_list_init(&pool->chunks[i].free_node);

        /* Handle edgecase of first chunkpool_update() call. */
        pool->chunks[i].coords.x = TAP_INT_MAX-(R_DISTANCE*2); 
        pool->chunks[i].coords.y = TAP_INT_MAX-(R_DISTANCE*2);
        pool->chunks[i].coords.z = TAP_INT_MAX-(R_DISTANCE*2);

        pool->meshes[i].faces = tap_arena_alloc(arena, TapFace, (CHUNK_SZ * CHUNK_SZ * CHUNK_SZ * 12));
    }
}

static void chunkpool_update(ChunkPool* pool, TapVec3I player_chunk_coords, const char *mapfile)
{
    const TapVec3I index_offset = {R_DISTANCE/2, R_DISTANCE/2, R_DISTANCE/2}; /* +/- 5 chunk render distance so {0,0,0} offset from player is loaded_chunks[4][4][4] */
    Chunk *loaded_chunks[R_DISTANCE][R_DISTANCE][R_DISTANCE] = {0};
    int i_x, i_y, i_z;
    size_t i;

    /* Check if any chunks in the pool are within the render distance. */
    for (i = 0; i < pool->size; ++i)
    {
        Chunk *chunk = &(pool->chunks[i]);
        TapVec3I relative = tap_vec3i_add(tap_vec3i_sub(chunk->coords, player_chunk_coords), index_offset);
        if (relative.x < 0 || relative.x >= R_DISTANCE || relative.y < 0 || relative.y >= R_DISTANCE || relative.z < 0 || relative.z >= R_DISTANCE)
            tap_list_insert(&pool->free_anchor, &chunk->free_node);
        else
            loaded_chunks[relative.x][relative.y][relative.z] = chunk;
    }

    /* Load any remaining chunks within the render distance but not already in the pool. */
    for (i_x = 0; i_x < R_DISTANCE; ++i_x)
        for (i_y = 0; i_y < R_DISTANCE; ++i_y)
            for (i_z = 0; i_z < R_DISTANCE; ++i_z)
                if (loaded_chunks[i_x][i_y][i_z] == NULL)
                {
                    TapVec3I relative = {i_x, i_y, i_z};
                    TapVec3I absolute;

                    Chunk *chunk = tap_def_containerof(pool->free_anchor.next, Chunk, free_node);
                    TapChunkMesh *mesh = &(pool->meshes[chunk - pool->chunks]);
                    tap_list_remove(pool->free_anchor.next);
                    tap_list_init(&chunk->free_node);

                    absolute = tap_vec3i_sub(tap_vec3i_add(relative, player_chunk_coords), index_offset);
                    chunk->coords = absolute;
                    mesh->world_coords.x = (float)chunk->coords.x * CHUNK_SZ;
                    mesh->world_coords.y = (float)chunk->coords.y * CHUNK_SZ;
                    mesh->world_coords.z = (float)chunk->coords.z * CHUNK_SZ;

                    chunk_load(chunk, mapfile);
                    chunk_mesh_update(chunk, mesh);
                }
}


TapResult tap_chunk_init(void)
{
    TapResult result = {0};
    if (state.initialized)
        return result;

    state.arena = tap_arena_create(4096);
    chunkpool_init(&state.arena, &state.chunk_pool);

    state.initialized = 1;
    return result;
}

TapResult tap_chunk_get_meshes(TapVec3 player_coords, const TapChunkMesh **meshes, size_t *num_meshes)
{
    TapResult result = {0};

    if (!meshes || !num_meshes)
        TAP_GUARD_BAIL(TAP_ERRNO_NULLPTR);
    if (!state.initialized)
        TAP_GUARD_BAIL(TAP_ERRNO_MODULE_UNINITIALIZED);

    TapVec3I player_chunk_coords = { (int)player_coords.x / CHUNK_SZ, (int)player_coords.y / CHUNK_SZ, (int)player_coords.z / CHUNK_SZ };
    chunkpool_update(&state.chunk_pool, player_chunk_coords, "placeholder");

    *meshes = state.chunk_pool.meshes;
    *num_meshes = state.chunk_pool.size;

    return result;
}

TapResult tap_chunk_deinit(void)
{
    TapResult result = {0};
    tap_arena_destroy(&state.arena);
    state.initialized = 0;
    return result;
}
