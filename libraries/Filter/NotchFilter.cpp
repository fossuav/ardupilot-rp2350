/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HAL_DEBUG_BUILD
#define AP_INLINE_VECTOR_OPS
#pragma GCC optimize("O2")
#endif

#include "NotchFilter.h"
#include <AP_Logger/AP_Logger.h>

const static float NOTCH_MAX_SLEW       = 0.05f;
const static float NOTCH_MAX_SLEW_LOWER = 1.0f - NOTCH_MAX_SLEW;
const static float NOTCH_MAX_SLEW_UPPER = 1.0f / NOTCH_MAX_SLEW_LOWER;

/*
   calculate the sine and cosine of an angle from 0 to pi, the range of a
   notch's omega, to within 1e-7. A dynamic notch recalculates its
   coefficients for every notch at up to half the gyro rate, where calling
   sinf() and cosf() separately, each with its general range reduction, is
   a large part of the cost. Forced inline so that it runs from wherever
   the caller is placed in memory.
 */
__attribute__((always_inline)) static inline void sin_cos_0_to_pi(float angle_rad, float &sin_angle, float &cos_angle)
{
    static constexpr float PI_4 = 0.785398163f;
    static constexpr float THREE_PI_4 = 2.35619449f;
    // pi/2 and pi as a float and its remainder, so the reduction keeps full precision
    static constexpr float PI_2_HI = 1.57079637f;
    static constexpr float PI_2_LO = -4.37113900e-08f;
    static constexpr float PI_HI = 3.14159274f;
    static constexpr float PI_LO = -8.74227801e-08f;
    // minimax polynomials for sin(x) and cos(x) over +-pi/4
    static constexpr float S3 = -1.666665077e-01f;
    static constexpr float S5 = 8.331978694e-03f;
    static constexpr float S7 = -1.949563593e-04f;
    static constexpr float C2 = -5.000000000e-01f;
    static constexpr float C4 = 4.166662320e-02f;
    static constexpr float C6 = -1.388676348e-03f;
    static constexpr float C8 = 2.439045056e-05f;

    const bool upper_quarter = angle_rad >= THREE_PI_4;
    const bool middle_half = !upper_quarter && angle_rad > PI_4;

    float x = angle_rad;
    if (middle_half) {
        x = (angle_rad - PI_2_HI) - PI_2_LO;
    } else if (upper_quarter) {
        x = (PI_HI - angle_rad) + PI_LO;
    }

    const float x2 = x * x;
    const float sin_x = x + x * x2 * (S3 + x2 * (S5 + x2 * S7));
    const float cos_x = 1.0f + x2 * (C2 + x2 * (C4 + x2 * (C6 + x2 * C8)));

    if (middle_half) {
        sin_angle = cos_x;
        cos_angle = -sin_x;
    } else if (upper_quarter) {
        sin_angle = sin_x;
        cos_angle = -cos_x;
    } else {
        sin_angle = sin_x;
        cos_angle = cos_x;
    }
}

/*
   calculate the attenuation and quality factors of the filter
 */
template <class T>
void NotchFilter<T>::calculate_A_and_Q(float center_freq_hz, float bandwidth_hz, float attenuation_dB, float& A, float& Q) {
    A = powf(10, -attenuation_dB / 40.0f);
    if (center_freq_hz > 0.5 * bandwidth_hz) {
        const float octaves = log2f(center_freq_hz / (center_freq_hz - bandwidth_hz / 2.0f)) * 2.0f;
        Q = sqrtf(powf(2, octaves)) / (powf(2, octaves) - 1.0f);
    } else {
        Q = 0.0;
    }
}

/*
  initialise filter
 */
template <class T>
void NotchFilter<T>::init(float sample_freq_hz, float center_freq_hz, float bandwidth_hz, float attenuation_dB)
{
    // check center frequency is in the allowable range
    initialised = false;
    if ((center_freq_hz > 0.5 * bandwidth_hz) && (center_freq_hz < 0.5 * sample_freq_hz)) {
        float A, Q;
        calculate_A_and_Q(center_freq_hz, bandwidth_hz, attenuation_dB, A, Q);
        init_with_A_and_Q(sample_freq_hz, center_freq_hz, A, Q);
    }
}

template <class T>
void NotchFilter<T>::init_with_A_and_Q(float sample_freq_hz, float center_freq_hz, float A, float Q)
{
    // don't update if no updates required
    if (initialised &&
        is_equal(center_freq_hz, _center_freq_hz) &&
        is_equal(sample_freq_hz, _sample_freq_hz) &&
        is_equal(A, _A)) {
        return;
    }

    float new_center_freq = center_freq_hz;

    // constrain the new center frequency by a percentage of the old frequency
    if (initialised && !need_reset && !is_zero(_center_freq_hz)) {
        new_center_freq = constrain_float(new_center_freq, _center_freq_hz * NOTCH_MAX_SLEW_LOWER,
                                          _center_freq_hz * NOTCH_MAX_SLEW_UPPER);
    }

    if (is_positive(new_center_freq) && (new_center_freq < 0.5 * sample_freq_hz) && (Q > 0.0)) {
        float omega = 2.0 * M_PI * new_center_freq / sample_freq_hz;
        float sin_omega, cos_omega;
        sin_cos_0_to_pi(omega, sin_omega, cos_omega);
        float alpha = sin_omega / (2 * Q);
        b0 =  1.0 + alpha*sq(A);
        b1 = -2.0 * cos_omega;
        b2 =  1.0 - alpha*sq(A);
        a1 = b1;
        a2 =  1.0 - alpha;

        const float a0_inv =  1.0/(1.0 + alpha);

        // Pre-multiply to save runtime calc
        b0 *= a0_inv;
        b1 *= a0_inv;
        b2 *= a0_inv;
        a1 *= a0_inv;
        a2 *= a0_inv;

        _center_freq_hz = new_center_freq;
        _sample_freq_hz = sample_freq_hz;
        _A = A;
        initialised = true;
    } else {
        // leave center_freq_hz at last value
        initialised = false;
    }
}

/*
  apply a new input sample, returning new output
 */
template <class T>
T NotchFilter<T>::apply(const T &sample)
{
    if (!initialised || need_reset) {
        // if we have not been initialised when return the input
        // sample as output and update delayed samples
        signal1 = sample;
        signal2 = sample;
        ntchsig1 = sample;
        ntchsig2 = sample;
        need_reset = false;
        return sample;
    }

    T output = sample*b0 + ntchsig1*b1 + ntchsig2*b2 - signal1*a1 - signal2*a2;

    ntchsig2 = ntchsig1;
    ntchsig1 = sample;

    signal2 = signal1;
    signal1 = output;
    return output;
}

template <class T>
void NotchFilter<T>::reset()
{
    need_reset = true;
}

#if HAL_LOGGING_ENABLED
// return the frequency to log for the notch
template <class T>
float NotchFilter<T>::logging_frequency() const
{
    return initialised ? _center_freq_hz : AP_Logger::quiet_nanf();
}
#endif

/*
   instantiate template classes
 */
template class NotchFilter<float>;
template class NotchFilter<Vector2f>;
template class NotchFilter<Vector3f>;
