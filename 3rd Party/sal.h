/*
 * minimalist <sal.h> needed to get XAudio2 compiling with MinGW
 * If your local compiler package includes a system <sal.h>, just use that.
 */

#ifndef _SAL_H_
#define _SAL_H_

/*
 * 2015.05.06 cxd4
 * taken straight from Visual Studio's <sal.h>, required for upcoming defs
 */
#ifdef _USE_SAL2_ONLY
#define _SAL2_STRICT
#define _SAL_VERSION_CHECK(_A) _SAL_VERSION_SAL2(_A)
#else
#define _SAL_VERSION_CHECK(_A)
#endif

/*
 * 2015.05.06 cxd4
 *
 * <xma2defs.h> needs these from Visual Studio 2013's <specstrings_strict.h>.
 *
 * These macros are not in the same order as presented in official <sal.h>; I
 * just added them one-by-one in the order that the pre-processor complained.
 */
#define __in_bcount(size)               _SAL_VERSION_CHECK(__in_bcount)
#define __in_ecount(size)               _SAL_VERSION_CHECK(__in_ecount)
#define __out                           _SAL_VERSION_CHECK(__out)
#define __in                            _SAL_VERSION_CHECK(__in)
#define __inout                         _SAL_VERSION_CHECK(__inout)

/*
 * 2015.05.06 cxd4
 *
 * <xaudio2.h> needs these from Visual Studio 2013's <specstrings_strict.h>.
 * Again, I've not added these macros in the same order as given in <sal.h>.
 */
#define __deref_out                     _SAL_VERSION_CHECK(__deref_out)
#define __in_opt                        _SAL_VERSION_CHECK(__in_opt)
#define __reserved                      _SAL_VERSION_CHECK(__reserved)
#define __out_bcount(size)              _SAL_VERSION_CHECK(__out_bcount)
#define __out_ecount(size)              _SAL_VERSION_CHECK(__out_ecount)

#endif

