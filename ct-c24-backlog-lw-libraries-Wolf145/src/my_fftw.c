#include "../include/my_fftw.h"

#include "../return_codes.h"

#include <fftw3.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t cross_correlation(double* signal1, double* signal2, size_t size, double* result)
{
	uint8_t err = SUCCESS;
	fftw_plan plan_forward1, plan_forward2, plan_backward;
	fftw_complex* block = (fftw_complex*)fftw_malloc(3 * size * sizeof(fftw_complex));
	if (!block)
	{
		err = ERROR_NOTENOUGH_MEMORY;
		fprintf(stderr, "Failed to allocate memory in fftw_alloc_complex");
		goto cleaning;
	}
	fftw_complex* complSignal1 = block;
	fftw_complex* complSignal2 = block + size;
	fftw_complex* res = block + 2 * size;

	plan_forward1 = fftw_plan_dft_r2c_1d((int32_t)size, signal1, complSignal1, FFTW_ESTIMATE);
	plan_forward2 = fftw_plan_dft_r2c_1d((int32_t)size, signal2, complSignal2, FFTW_ESTIMATE);
	if (plan_forward1 == NULL || plan_forward2 == NULL)
	{
		err = ERROR_DATA_INVALID;
		fprintf(stderr, "Failed to create plan for performing FFT");
		goto cleaning;
	}

	fftw_execute(plan_forward1);
	fftw_execute(plan_forward2);

	for (size_t i = 0; i < size; ++i)
	{
		res[i][0] = complSignal1[i][0] * complSignal2[i][0] + complSignal1[i][1] * complSignal2[i][1];
		res[i][1] = complSignal1[i][1] * complSignal2[i][0] - complSignal1[i][0] * complSignal2[i][1];
	}

	plan_backward = fftw_plan_dft_c2r_1d((int32_t)size, res, result, FFTW_ESTIMATE);
	if (plan_backward == NULL)
	{
		err = ERROR_DATA_INVALID;
		fprintf(stderr, "Failed to create plan for performing FFT");
		goto cleaning;
	}
	fftw_execute(plan_backward);
	fftw_destroy_plan(plan_backward);

cleaning:
	if (block)
	{
		fftw_destroy_plan(plan_forward1);
		fftw_destroy_plan(plan_forward2);
		fftw_cleanup();
		fftw_free(block);
	}
	return err;
}
