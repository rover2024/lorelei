#include <openssl/aes.h>
#include <openssl/asn1err.h>
#include <openssl/async.h>
#include <openssl/asyncerr.h>
#include <openssl/bioerr.h>
#include <openssl/blowfish.h>
#include <openssl/bn.h>
#include <openssl/bnerr.h>
#include <openssl/buffer.h>
#include <openssl/buffererr.h>
#include <openssl/byteorder.h>
#include <openssl/camellia.h>
#include <openssl/cast.h>
#include <openssl/cmac.h>
#include <openssl/cmp_util.h>
#include <openssl/cmperr.h>
#include <openssl/cmserr.h>
#include <openssl/comperr.h>
#include <openssl/conf_api.h>
#include <openssl/conferr.h>
#include <openssl/configuration.h>
#include <openssl/conftypes.h>
#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_object.h>
#include <openssl/crmferr.h>
#include <openssl/cryptoerr.h>
#include <openssl/cryptoerr_legacy.h>
#include <openssl/cterr.h>
#include <openssl/decoder.h>
#include <openssl/decodererr.h>
#include <openssl/des.h>
#include <openssl/dh.h>
#include <openssl/dherr.h>
#include <openssl/dsa.h>
#include <openssl/dsaerr.h>
#include <openssl/dtls1.h>
#include <openssl/e_os2.h>
#include <openssl/e_ostime.h>
#include <openssl/ebcdic.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/ecerr.h>
#include <openssl/encoder.h>
#include <openssl/encodererr.h>
#include <openssl/engine.h>
#include <openssl/engineerr.h>
#include <openssl/esserr.h>
#include <openssl/evp.h>
#include <openssl/evperr.h>
#include <openssl/fips_names.h>
#include <openssl/hmac.h>
#include <openssl/hpke.h>
#include <openssl/http.h>
#include <openssl/httperr.h>
#include <openssl/idea.h>
#include <openssl/indicator.h>
#include <openssl/kdf.h>
#include <openssl/kdferr.h>
#include <openssl/macros.h>
#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/mdc2.h>
#include <openssl/ml_kem.h>
#include <openssl/modes.h>
#include <openssl/obj_mac.h>
#include <openssl/objects.h>
#include <openssl/objectserr.h>
#include <openssl/ocsperr.h>
#include <openssl/opensslconf.h>
#include <openssl/ossl_typ.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include <openssl/pem2.h>
#include <openssl/pemerr.h>
#include <openssl/pkcs12err.h>
#include <openssl/pkcs7err.h>
#include <openssl/prov_ssl.h>
#include <openssl/proverr.h>
#include <openssl/provider.h>
#include <openssl/quic.h>
#include <openssl/rand.h>
#include <openssl/randerr.h>
#include <openssl/rc2.h>
#include <openssl/rc4.h>
#include <openssl/rc5.h>
#include <openssl/ripemd.h>
#include <openssl/rsa.h>
#include <openssl/rsaerr.h>
#include <openssl/seed.h>
#include <openssl/self_test.h>
#include <openssl/sha.h>
#include <openssl/srtp.h>
#include <openssl/ssl2.h>
#include <openssl/ssl3.h>
#include <openssl/sslerr.h>
#include <openssl/sslerr_legacy.h>
#include <openssl/stack.h>
#include <openssl/store.h>
#include <openssl/storeerr.h>
#include <openssl/symhacks.h>
#include <openssl/thread.h>
#include <openssl/tls1.h>
#include <openssl/trace.h>
#include <openssl/ts.h>
#include <openssl/tserr.h>
#include <openssl/txt_db.h>
#include <openssl/types.h>
#include <openssl/uierr.h>
#include <openssl/whrlpool.h>
#include <openssl/x509err.h>
#include <openssl/x509v3err.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/cmp.h>
#include <openssl/cms.h>
#include <openssl/comp.h>
#include <openssl/conf.h>
#include <openssl/core_names.h>
#include <openssl/crmf.h>
#include <openssl/crypto.h>
#include <openssl/ct.h>
#include <openssl/err.h>
#include <openssl/ess.h>
#include <openssl/fipskey.h>
#include <openssl/lhash.h>
#include <openssl/ocsp.h>
#include <openssl/opensslv.h>
#include <openssl/pkcs12.h>
#include <openssl/pkcs7.h>
#include <openssl/safestack.h>
#include <openssl/srp.h>
#include <openssl/ssl.h>
#include <openssl/ui.h>
#include <openssl/x509.h>
#include <openssl/x509_acert.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

#include <internal/o_dir.h>
#include <internal/dso.h>
#include <internal/sslconf.h>
#include <internal/err.h>
#include <internal/asn1.h>

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