#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef __PLUGIN__
#include <vlc/vlc.h>
#endif

enum {
    I420_FOURCC = '024I' // FIXME
};

struct vmem2_video_format_t
{
    const uint32_t version;

    uint32_t chroma;

    const uint32_t width;
    const uint32_t height;
    const uint32_t visible_width;
    const uint32_t visible_height;

    const uint32_t sar_num;
    const uint32_t sar_den;

    uint32_t plane_count;

    uint32_t* pitches;
    uint32_t* lines;
};
typedef struct vmem2_video_format_t vmem2_video_format_t;

struct vmem2_planes_t
{
    const uint32_t version;

    const uint32_t plane_count;
    void* *const planes;

    void* opaque;
};
typedef struct vmem2_planes_t vmem2_planes_t;

typedef bool (*vmem2_setup_cb)( void* opaque, vmem2_video_format_t* format );

typedef bool (*vmem2_lock_cb)( void* opaque, vmem2_planes_t* planes );
typedef void (*vmem2_unlock_cb)( void* opaque, const vmem2_planes_t* planes );
typedef void (*vmem2_display_cb)( void* opaque, const vmem2_planes_t* planes );

typedef void (*vmem2_cleanup_cb)( void* opaque );

#ifndef __PLUGIN__
struct libvlc_media_player_t;
extern "C" {
LIBVLC_API void vmem2_set_callbacks( libvlc_media_player_t* mp,
                                     vmem2_setup_cb setup_cb,
                                     vmem2_lock_cb lock_cb, vmem2_unlock_cb unlock_cb,
                                     vmem2_display_cb display_cb,
                                     vmem2_cleanup_cb cleanup_cb,
                                     void* opaque );
}
#endif
