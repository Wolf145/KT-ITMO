#include "include/my_ffmpeg.h"
#include "include/my_fftw.h"
#include "return_codes.h"

#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t align(double** arr1, size_t sz1, size_t sz2)
{
	double* temp = NULL;
	(*arr1) = (double*)realloc(temp = *arr1, sz2 * sizeof(double));
	if (!*arr1)
	{
		fprintf(stderr, "Failed to realloc memory\n");
		free(temp);
		return ERROR_NOTENOUGH_MEMORY;
	}
	memset((*arr1) + sz1, 0, (sz2 - sz1) * sizeof(double));
	return SUCCESS;
}
int main(int argc, char** argv)
{
	double* samples1 = NULL;
	double* samples2 = NULL;
	double* res = NULL;
	uint8_t err1 = SUCCESS, err2 = SUCCESS, err = SUCCESS;
	int32_t sr1 = 0, sr2 = 0;
	size_t sz1 = 0, sz2 = 0, sz;

	switch (argc)
	{
	case 2:
		err1 = decode_audio(argv[1], 0, 2, &samples1, &sr1, &sz1);
		err2 = decode_audio(argv[1], 1, 2, &samples2, &sr2, &sz2);
		break;
	case 3:
		err1 = decode_audio(argv[1], 0, 1, &samples1, &sr1, &sz1);
		err2 = decode_audio(argv[2], 0, 1, &samples2, &sr2, &sz2);
		break;
	default:
		fprintf(stderr, "./main <audio_file1> [<audio_file2>]\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (sr1 != sr2)
	{
		// TODO: resample
		err = ERROR_UNSUPPORTED;
		fprintf(stderr, "Unsupported functionality\n");
		goto cleaning;
	}
	if (err1 || err2)
	{
		err = (err1 ? err1 : err2);
		goto cleaning;
	}
	sz = sz1;
	if (sz1 > sz2)
	{
		err = align(&samples2, sz2, sz1);
		if (err)
			goto cleaning;
	}
	else if (sz1 < sz2)
	{
		sz = sz2;
		err = align(&samples1, sz1, sz2);
		if (err)
			goto cleaning;
	}
	res = malloc(sz * sizeof(double));
	if (!res)
	{
		err = ERROR_NOTENOUGH_MEMORY;
		fprintf(stderr, "Failed to allocate memory for result array\n");
		goto cleaning;
	}
	err = cross_correlation(samples1, samples2, sz, res);
	if (err)
		goto cleaning;
	size_t count = 0;
	int32_t delta_samples;
	double max = -DBL_MAX;
	for (size_t i = 0; i < sz; ++i)
	{
		if (res[i] > max)
		{
			max = res[i];
			count = i;
		}
	}
	delta_samples = (2 * count <= sz ? (int32_t)count : (int32_t)count - (int32_t)sz);

	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", delta_samples, sr1, (int32_t)((double)(delta_samples * 1000) / sr1));

cleaning:
	free(samples1);
	free(samples2);
	free(res);
	return err;
}
