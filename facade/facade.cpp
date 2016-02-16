#include <vlc_common.h>
#include <vlc_variables.h>

#include "vmem2.h"

struct libvlc_media_player_t
{
    VLC_COMMON_MEMBERS
};

void vmem2_set_callbacks( libvlc_media_player_t* mp,
                          vmem2_setup_cb setup_cb,
                          vmem2_lock_cb lock_cb, vmem2_unlock_cb unlock_cb,
                          vmem2_display_cb display_cb,
                          vmem2_cleanup_cb cleanup_cb,
                          void* opaque )
{
    var_Create( mp, "vmem2-setup", VLC_VAR_ADDRESS );
    var_Create( mp, "vmem2-lock", VLC_VAR_ADDRESS );
    var_Create( mp, "vmem2-unlock", VLC_VAR_ADDRESS );
    var_Create( mp, "vmem2-display", VLC_VAR_ADDRESS );
    var_Create( mp, "vmem2-cleanup", VLC_VAR_ADDRESS );
    var_Create( mp, "vmem2-opaque", VLC_VAR_ADDRESS );

    var_SetAddress( mp, "vmem2-setup", reinterpret_cast<void*>( setup_cb ) );
    var_SetAddress( mp, "vmem2-lock", reinterpret_cast<void*>( lock_cb ) );
    var_SetAddress( mp, "vmem2-unlock", reinterpret_cast<void*>( unlock_cb ) );
    var_SetAddress( mp, "vmem2-display", reinterpret_cast<void*>( display_cb ) );
    var_SetAddress( mp, "vmem2-cleanup", reinterpret_cast<void*>( cleanup_cb ) );
    var_SetAddress( mp, "vmem2-opaque", opaque );

    var_SetString( mp, "vout", "vmem2" );
    var_SetString( mp, "window", "none" );
}
