#pragma once
#include <stdint.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif	  // __cplusplus

	uint8_t decode_audio(char *, size_t, size_t, double **, int32_t *, size_t *);

#ifdef __cplusplus
}
#endif	  // __cplusplus
