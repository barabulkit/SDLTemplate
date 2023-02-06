/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*

Built with:

gcc -o genfilter build-scripts/gen_audio_resampler_filter.c -lm && ./genfilter > src/audio/SDL_audio_resampler_filter.h

 */

/*
   SDL's resampler uses a "bandlimited interpolation" algorithm:
     https://ccrma.stanford.edu/~jos/resample/

   This code pre-generates the kaiser tables so we don't have to do this at
   run time, at a cost of about 20 kilobytes of static data in SDL. This code
   used to be part of SDL itself and generated the tables on the first use,
   but that was expensive to produce on platforms without floating point
   hardware.
*/

#include <stdio.h>
#include <math.h>

#define RESAMPLER_ZERO_CROSSINGS 5
#define RESAMPLER_BITS_PER_SAMPLE 16
#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING  (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))
#define RESAMPLER_FILTER_SIZE ((RESAMPLER_SAMPLES_PER_ZERO_CROSSING * RESAMPLER_ZERO_CROSSINGS) + 1)

/* This is a "modified" bessel function, so you can't use POSIX j0() */
static double
bessel(const double x)
{
    const double xdiv2 = x / 2.0;
    double i0 = 1.0f;
    double f = 1.0f;
    int i = 1;

    while (1) {
        const double diff = pow(xdiv2, i * 2) / pow(f, 2);
        if (diff < 1.0e-21f) {
            break;
        }
        i0 += diff;
        i++;
        f *= (double) i;
    }

    return i0;
}

/* build kaiser table with cardinal sine applied to it, and array of differences between elements. */
static void
kaiser_and_sinc(float *table, float *diffs, const int tablelen, const double beta)
{
    const int lenm1 = tablelen - 1;
    const int lenm1div2 = lenm1 / 2;
    const double bessel_beta = bessel(beta);
    int i;

    table[0] = 1.0f;
    for (i = 1; i < tablelen; i++) {
        const double kaiser = bessel(beta * sqrt(1.0 - pow(((i - lenm1) / 2.0) / lenm1div2, 2.0))) / bessel_beta;
        table[tablelen - i] = (float) kaiser;
    }

    for (i = 1; i < tablelen; i++) {
        const float x = (((float) i) / ((float) RESAMPLER_SAMPLES_PER_ZERO_CROSSING)) * ((float) M_PI);
        table[i] *= sinf(x) / x;
        diffs[i - 1] = table[i] - table[i - 1];
    }
    diffs[lenm1] = 0.0f;
}


static float ResamplerFilter[RESAMPLER_FILTER_SIZE];
static float ResamplerFilterDifference[RESAMPLER_FILTER_SIZE];

static void
PrepareResampleFilter(void)
{
    /* if dB > 50, beta=(0.1102 * (dB - 8.7)), according to Matlab. */
    const double dB = 80.0;
    const double beta = 0.1102 * (dB - 8.7);
    kaiser_and_sinc(ResamplerFilter, ResamplerFilterDifference, RESAMPLER_FILTER_SIZE, beta);
}

int main(void)
{
    int i;

    PrepareResampleFilter();

    printf(
        "/*\n"
        "  Simple DirectMedia Layer\n"
        "  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>\n"
        "\n"
        "  This software is provided 'as-is', without any express or implied\n"
        "  warranty.  In no event will the authors be held liable for any damages\n"
        "  arising from the use of this software.\n"
        "\n"
        "  Permission is granted to anyone to use this software for any purpose,\n"
        "  including commercial applications, and to alter it and redistribute it\n"
        "  freely, subject to the following restrictions:\n"
        "\n"
        "  1. The origin of this software must not be misrepresented; you must not\n"
        "     claim that you wrote the original software. If you use this software\n"
        "     in a product, an acknowledgment in the product documentation would be\n"
        "     appreciated but is not required.\n"
        "  2. Altered source versions must be plainly marked as such, and must not be\n"
        "     misrepresented as being the original software.\n"
        "  3. This notice may not be removed or altered from any source distribution.\n"
        "*/\n"
        "\n"
        "/* DO NOT EDIT, THIS FILE WAS GENERATED BY build-scripts/gen_audio_resampler_filter.c */\n"
        "\n"
        "#define RESAMPLER_ZERO_CROSSINGS %d\n"
        "#define RESAMPLER_BITS_PER_SAMPLE %d\n"
        "#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))\n"
        "#define RESAMPLER_FILTER_SIZE ((RESAMPLER_SAMPLES_PER_ZERO_CROSSING * RESAMPLER_ZERO_CROSSINGS) + 1)\n"
        "\n", RESAMPLER_ZERO_CROSSINGS, RESAMPLER_BITS_PER_SAMPLE
    );

    printf("static const float ResamplerFilter[RESAMPLER_FILTER_SIZE] = {\n");
    printf("    %.9ff", ResamplerFilter[0]);
    for (i = 0; i < RESAMPLER_FILTER_SIZE-1; i++) {
        printf("%s%.9ff", ((i % 5) == 4) ? ",\n    " : ", ", ResamplerFilter[i+1]);
    }
    printf("\n};\n\n");

    printf("static const float ResamplerFilterDifference[RESAMPLER_FILTER_SIZE] = {\n");
    printf("    %.9ff", ResamplerFilterDifference[0]);
    for (i = 0; i < RESAMPLER_FILTER_SIZE-1; i++) {
        printf("%s%.9ff", ((i % 5) == 4) ? ",\n    " : ", ", ResamplerFilterDifference[i+1]);
    }
    printf("\n};\n\n");
    printf("/* vi: set ts=4 sw=4 expandtab: */\n\n");

    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */

