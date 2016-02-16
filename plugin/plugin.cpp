
#include <new>
#include <memory>
#include <vector>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>

#include "vmem2.h"

enum {
    VMEM2_VERSION = 1,
};

static int Open( vout_display_t* );
static void Close( vout_display_t* );

static picture_pool_t* Pool( vout_display_t*, unsigned );
static void Display( vout_display_t*, picture_t*, subpicture_t* );
static int Control( vout_display_t*, int, va_list );

vlc_module_begin()
    set_shortname( "vmem2" )

    set_category( CAT_VIDEO )
    set_subcategory( SUBCAT_VIDEO_VOUT )
    set_capability( "vout display", 0 )

    set_callbacks( Open, Close )
vlc_module_end()

struct vmem2_video_format_internal_t : public vmem2_video_format_t
{
    vmem2_video_format_internal_t();
    vmem2_video_format_internal_t( const video_format_t& libvlc_fmt );

    uint32_t pitches[VOUT_MAX_PLANES];
    uint32_t lines[VOUT_MAX_PLANES];
};

vmem2_video_format_internal_t::vmem2_video_format_internal_t()
    : vmem2_video_format_t{ VMEM2_VERSION, 0, 0, 0, 0, 0, 1, 1,
                            VOUT_MAX_PLANES, pitches, lines }
{
    memset( pitches, 0, sizeof( pitches ) );
    memset( lines, 0, sizeof( lines ) );
}

vmem2_video_format_internal_t::vmem2_video_format_internal_t( const video_format_t& libvlc_fmt )
    : vmem2_video_format_t { VMEM2_VERSION, libvlc_fmt.i_chroma,
                             libvlc_fmt.i_width, libvlc_fmt.i_height,
                             libvlc_fmt.i_visible_width, libvlc_fmt.i_visible_height,
                             libvlc_fmt.i_sar_num, libvlc_fmt.i_sar_den,
                             VOUT_MAX_PLANES, pitches, lines }
{
    memset( pitches, 0, sizeof( pitches ) );
    memset( lines, 0, sizeof( lines ) );
}

struct vmem2_planes_internal_t : public vmem2_planes_t
{
    vmem2_planes_internal_t();

    void* planes[VOUT_MAX_PLANES];
};

vmem2_planes_internal_t::vmem2_planes_internal_t()
    : vmem2_planes_t { VMEM2_VERSION, VOUT_MAX_PLANES, planes, nullptr }
{
    memset( planes, 0, sizeof( planes ) );
}

struct vout_display_sys_t
{
    void* opaque;

    vmem2_setup_cb setup_cb;

    vmem2_lock_cb lock_cb;
    vmem2_unlock_cb unlock_cb;
    vmem2_display_cb display_cb;

    vmem2_cleanup_cb cleanup_cb;

    vmem2_video_format_internal_t video_format; //?

    picture_pool_t* picture_pool;
};

struct picture_sys_t
{
    vout_display_sys_t *const sys;

    vmem2_planes_internal_t planes;
};

static int Open( vout_display_t* vd )
{
    std::unique_ptr<vout_display_sys_t> sys(
        new( std::nothrow ) vout_display_sys_t {
            var_InheritAddress( vd, "vmem2-opaque" ),
            reinterpret_cast<vmem2_setup_cb>( var_InheritAddress( vd, "vmem2-setup" ) ),
            reinterpret_cast<vmem2_lock_cb>( var_InheritAddress( vd, "vmem2-lock" ) ),
            reinterpret_cast<vmem2_unlock_cb>( var_InheritAddress( vd, "vmem2-unlock" ) ),
            reinterpret_cast<vmem2_display_cb>( var_InheritAddress( vd, "vmem2-display" ) ),
            reinterpret_cast<vmem2_cleanup_cb>( var_InheritAddress( vd, "vmem2-cleanup" ) ),
            vd->source,
            nullptr
        } );
    if( !sys )
        return VLC_ENOMEM;

    if( !sys->setup_cb || !sys->lock_cb )
        return VLC_EGENERIC;

    if( !sys->setup_cb( sys->opaque, &sys->video_format ) )
        return VLC_EGENERIC;

    video_format_t libvlc_fmt{ .i_chroma = sys->video_format.chroma };
    video_format_ApplyRotation( &libvlc_fmt, &vd->fmt );

    switch( libvlc_fmt.i_chroma ) {
        case VLC_CODEC_RGB24:
        case VLC_CODEC_RGB32:
            libvlc_fmt.i_rmask = 0xff0000;
            libvlc_fmt.i_gmask = 0x00ff00;
            libvlc_fmt.i_bmask = 0x0000ff;
            break;
        case VLC_CODEC_I420:
            libvlc_fmt.i_rmask = libvlc_fmt.i_gmask = libvlc_fmt.i_bmask = 0;
            break;
        default:
            return VLC_EGENERIC;
    }

    vd->info.has_hide_mouse = true;

    vd->sys     = sys.release();
    vd->fmt     = libvlc_fmt;

    vd->pool    = Pool;
    vd->prepare = NULL;
    vd->display = Display;
    vd->control = Control;
    vd->manage  = NULL;

    return VLC_SUCCESS;
}

static void Close( vout_display_t* vd )
{
    vout_display_sys_t *sys = vd->sys;

    if( sys->cleanup_cb )
        sys->cleanup_cb( sys->opaque );

    delete sys;
    vd->sys = nullptr;
}

static int picture_lock( picture_t* pic )
{
    picture_sys_t* pic_sys = pic->p_sys;
    vout_display_sys_t* sys = pic_sys->sys;

    if( sys->lock_cb( sys->opaque, &pic_sys->planes ) ) {
        for( int i = 0; i < VOUT_MAX_PLANES; ++i ) {
            pic->p[i].p_pixels =
                reinterpret_cast<decltype( pic->p[i].p_pixels )>( pic_sys->planes.planes[i] );
        }
        return VLC_SUCCESS;
    }

    return VLC_EGENERIC;
}

static void picture_unlock( picture_t* pic )
{
    picture_sys_t* pic_sys = pic->p_sys;
    vout_display_sys_t* sys = pic_sys->sys;

    if( sys->unlock_cb )
        sys->unlock_cb( sys->opaque, &pic_sys->planes );
}

static picture_pool_t* Pool( vout_display_t* vd, unsigned count )
{
    vout_display_sys_t* sys = vd->sys;

    if( sys->picture_pool )
        return sys->picture_pool;

    std::vector<picture_t*> pictures;
    pictures.reserve( count );

    for( unsigned i = 0; i < count; ++i ) {
        std::unique_ptr<picture_sys_t> pic_sys( new( std::nothrow ) picture_sys_t { sys } );

        picture_resource_t picture_resource;

        picture_resource.p_sys = pic_sys.get();
        picture_resource.pf_destroy =
            []( picture_t* picture ) {
                delete picture->p_sys;
                picture->p_sys = nullptr;
                vlc_free( picture );
            };

        for( unsigned p = 0; p < VOUT_MAX_PLANES; ++p ) {
            picture_resource.p[p].p_pixels = nullptr;
            picture_resource.p[p].i_lines = sys->video_format.lines[p];
            picture_resource.p[p].i_pitch = sys->video_format.pitches[p];
        }

        picture_t* picture = picture_NewFromResource( &vd->fmt, &picture_resource );
        if( picture ){
            pictures.emplace( pictures.end(), picture );
            pic_sys.release();
        } else
            break;
    }

    const picture_pool_configuration_t pool_configuration = {
        .picture_count = static_cast<decltype(picture_pool_configuration_t::picture_count)>( pictures.size() ),
        .picture = pictures.data(),
        .lock = picture_lock,
        .unlock = picture_unlock
    };

    sys->picture_pool = picture_pool_NewExtended( &pool_configuration );
    if( !sys->picture_pool )
        for( picture_t* picture : pictures ) picture_Release( picture );

    return sys->picture_pool;
}

static void Display( vout_display_t* vd, picture_t* pic, subpicture_t* /*subpic*/ )
{
    vout_display_sys_t *sys = vd->sys;
    picture_sys_t* pic_sys = pic->p_sys;

    if( sys->display_cb )
        sys->display_cb( sys->opaque, &pic_sys->planes );

    picture_Release( pic );
}

static int Control( vout_display_t* /*vd*/, int /*query*/, va_list /*args*/ )
{
    return VLC_EGENERIC;
}
