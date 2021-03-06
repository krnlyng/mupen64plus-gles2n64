#ifndef __gl2extimg_h_
#define __gl2extimg_h_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------*
 * IMG extension tokens
 *------------------------------------------------------------------------*/

/* GL_IMG_shader_binary */
#ifndef GL_IMG_shader_binary
#define GL_SGX_BINARY_IMG	0x8C0A
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03
#endif

/*------------------------------------------------------------------------*
 * IMG extension functions
 *------------------------------------------------------------------------*/

/* GL_IMG_shader_binary */
#ifndef GL_IMG_shader_binary
#define GL_IMG_shader_binary 1
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#endif	

#ifdef __cplusplus
}
#endif

#endif /* __gl2extimg_h_ */
