#include <libavutil/adler32.h>
#include <libavutil/aes.h>
#include <libavutil/aes_ctr.h>
#include <libavutil/ambient_viewing_environment.h>
#include <libavutil/attributes.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avassert.h>
#include <libavutil/avstring.h>
#include <libavutil/avutil.h>
#include <libavutil/base64.h>
#include <libavutil/blowfish.h>
#include <libavutil/bprint.h>
#include <libavutil/bswap.h>
#include <libavutil/buffer.h>
#include <libavutil/cast5.h>
#include <libavutil/camellia.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/container_fifo.h>
#include <libavutil/cpu.h>
#include <libavutil/crc.h>
#include <libavutil/csp.h>
#include <libavutil/des.h>
#include <libavutil/detection_bbox.h>
#include <libavutil/dict.h>
#include <libavutil/display.h>
#include <libavutil/dovi_meta.h>
#include <libavutil/downmix_info.h>
#include <libavutil/encryption_info.h>
#include <libavutil/error.h>
#include <libavutil/eval.h>
#include <libavutil/executor.h>
#include <libavutil/fifo.h>
#include <libavutil/file.h>
#include <libavutil/film_grain_params.h>
#include <libavutil/frame.h>
#include <libavutil/hash.h>
#include <libavutil/hdr_dynamic_metadata.h>
#include <libavutil/hdr_dynamic_vivid_metadata.h>
#include <libavutil/hmac.h>
#include <libavutil/hwcontext.h>
// #include <libavutil/hwcontext_cuda.h>
// #include <libavutil/hwcontext_d3d11va.h>
// #include <libavutil/hwcontext_d3d12va.h>
// #include <libavutil/hwcontext_drm.h>
// #include <libavutil/hwcontext_dxva2.h>
// #include <libavutil/hwcontext_amf.h>
// #include <libavutil/hwcontext_qsv.h>
// #include <libavutil/hwcontext_mediacodec.h>
// #include <libavutil/hwcontext_opencl.h>
// #include <libavutil/hwcontext_vaapi.h>
// #include <libavutil/hwcontext_videotoolbox.h>
// #include <libavutil/hwcontext_vdpau.h>
// #include <libavutil/hwcontext_vulkan.h>
#include <libavutil/iamf.h>
#include <libavutil/imgutils.h>
#include <libavutil/intfloat.h>
#include <libavutil/intreadwrite.h>
#include <libavutil/lfg.h>
#include <libavutil/log.h>
#include <libavutil/lzo.h>
#include <libavutil/macros.h>
#include <libavutil/mathematics.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavutil/md5.h>
#include <libavutil/mem.h>
#include <libavutil/motion_vector.h>
#include <libavutil/murmur3.h>
#include <libavutil/opt.h>
#include <libavutil/parseutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixelutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/random_seed.h>
#include <libavutil/rc4.h>
#include <libavutil/rational.h>
#include <libavutil/refstruct.h>
#include <libavutil/replaygain.h>
#include <libavutil/ripemd.h>
#include <libavutil/samplefmt.h>
#include <libavutil/sha.h>
#include <libavutil/sha512.h>
#include <libavutil/spherical.h>
#include <libavutil/stereo3d.h>
#include <libavutil/threadmessage.h>
#include <libavutil/time.h>
#include <libavutil/timecode.h>
#include <libavutil/timestamp.h>
#include <libavutil/tree.h>
#include <libavutil/twofish.h>
#include <libavutil/uuid.h>
#include <libavutil/video_enc_params.h>
#include <libavutil/xtea.h>
#include <libavutil/tea.h>
#include <libavutil/tx.h>
#include <libavutil/video_hint.h>

#include <lorelei/loreshared.h>
#include <lorelei/loreuser.h>

#include <lorelib_common/manifest-predef.h>



//
// Option macros
//
#define LORELIB_GCB_AUTO_DEPTH 1
#define LORELIB_HCB_AUTO_DEPTH 1



//
// Annotations
//
#ifdef LORELIB_VISUAL
#endif



//
// Custom(Guest)
//
#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)
#endif

#ifdef LORELIB_BUILD
#  include "lorelib_impl.c"
#endif