/* ************************************************************************
 * Copyright (C) 2018-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell cop-
 * ies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM-
 * PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNE-
 * CTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ************************************************************************ */
#include "blas1_gtest.hpp"

#include "testing_rot.hpp"
#include "testing_rot_batched.hpp"
#include "testing_rot_strided_batched.hpp"
#include "testing_rotg.hpp"
#include "testing_rotg_batched.hpp"
#include "testing_rotg_strided_batched.hpp"
#include "testing_rotm.hpp"
#include "testing_rotm_batched.hpp"
#include "testing_rotm_strided_batched.hpp"
#include "testing_rotmg.hpp"
#include "testing_rotmg_batched.hpp"
#include "testing_rotmg_strided_batched.hpp"

namespace
{

    // ----------------------------------------------------------------------------
    // BLAS1 testing template
    // ----------------------------------------------------------------------------
    template <template <typename...> class FILTER, blas1 BLAS1>
    struct rot_test_template : public RocBLAS_Test<rot_test_template<FILTER, BLAS1>, FILTER>
    {
        // Filter for which types apply to this suite
        static bool type_filter(const Arguments& arg)
        {
            return rocblas_blas1_dispatch<rot_test_template::template type_filter_functor>(arg);
        }

        // Filter for which functions apply to this suite
        static bool function_filter(const Arguments& arg);

        // Google Test name suffix based on parameters
        static std::string name_suffix(const Arguments& arg)
        {
            RocBLAS_TestName<rot_test_template> name(arg.name);
            name << rocblas_datatype2string(arg.a_type);

            if(strstr(arg.function, "_bad_arg") != nullptr)
            {
                name << "_bad_arg";
            }
            else
            {
                bool is_rot   = (BLAS1 == blas1::rot || BLAS1 == blas1::rot_batched
                               || BLAS1 == blas1::rot_strided_batched);
                bool is_rotg  = (BLAS1 == blas1::rotg || BLAS1 == blas1::rotg_batched
                                || BLAS1 == blas1::rotg_strided_batched);
                bool is_rotmg = (BLAS1 == blas1::rotmg || BLAS1 == blas1::rotmg_batched
                                 || BLAS1 == blas1::rotmg_strided_batched);
                bool is_batched
                    = (BLAS1 == blas1::rot_batched || BLAS1 == blas1::rotm_batched
                       || BLAS1 == blas1::rotg_batched || BLAS1 == blas1::rotmg_batched);
                bool is_strided
                    = (BLAS1 == blas1::rot_strided_batched || BLAS1 == blas1::rotm_strided_batched
                       || BLAS1 == blas1::rotg_strided_batched
                       || BLAS1 == blas1::rotmg_strided_batched);

                if((is_rotg || is_rot) && arg.a_type != arg.b_type)
                    name << '_' << rocblas_datatype2string(arg.b_type);
                if(is_rot && arg.compute_type != arg.a_type)
                    name << '_' << rocblas_datatype2string(arg.compute_type);

                if(!is_rotg && !is_rotmg)
                {
                    name << '_' << arg.N;

                    name << '_' << arg.incx;
                }

                if(is_strided && !is_rotg)
                {
                    name << '_' << arg.stride_x;
                }

                if(is_rot || BLAS1 == blas1::rotm || BLAS1 == blas1::rotm_batched
                   || BLAS1 == blas1::rotm_strided_batched)
                {
                    name << '_' << arg.incy;
                }

                if(BLAS1 == blas1::rot_strided_batched || BLAS1 == blas1::rotm_strided_batched)
                {
                    name << '_' << arg.stride_y;
                }

                if(BLAS1 == blas1::rotg_strided_batched)
                {
                    name << '_' << arg.stride_a << '_' << arg.stride_b << '_' << arg.stride_c << '_'
                         << arg.stride_d;
                }

                if(BLAS1 == blas1::rotm_strided_batched || BLAS1 == blas1::rotmg_strided_batched)
                {
                    name << '_' << arg.stride_c;
                }

                if(is_batched || is_strided)
                {
                    name << "_" << arg.batch_count;
                }

                if(arg.fortran)
                {
                    name << "_F";
                }
            }

            return std::move(name);
        }
    };

    // This tells whether the BLAS1 tests are enabled
    template <blas1 BLAS1, typename Ti, typename To, typename Tc>
    using rot_enabled = std::integral_constant<
        bool,
        ((BLAS1 == blas1::rot || BLAS1 == blas1::rot_batched || BLAS1 == blas1::rot_strided_batched)
         && ((std::is_same<Ti, float>{} && std::is_same<Ti, To>{} && std::is_same<To, Tc>{})
             || (std::is_same<Ti, double>{} && std::is_same<Ti, To>{} && std::is_same<To, Tc>{})
             || (std::is_same<Ti, rocblas_float_complex>{} && std::is_same<To, float>{}
                 && std::is_same<Tc, rocblas_float_complex>{})
             || (std::is_same<Ti, rocblas_float_complex>{} && std::is_same<To, float>{}
                 && std::is_same<Tc, float>{})
             || (std::is_same<Ti, rocblas_double_complex>{} && std::is_same<To, double>{}
                 && std::is_same<Tc, rocblas_double_complex>{})
             || (std::is_same<Ti, rocblas_double_complex>{} && std::is_same<To, double>{}
                 && std::is_same<Tc, double>{})))

            || ((BLAS1 == blas1::rotg || BLAS1 == blas1::rotg_batched
                 || BLAS1 == blas1::rotg_strided_batched)
                && std::is_same<To, Tc>{}
                && ((std::is_same<Ti, float>{} && std::is_same<Ti, To>{})
                    || (std::is_same<Ti, double>{} && std::is_same<Ti, To>{})
                    || (std::is_same<Ti, rocblas_float_complex>{} && std::is_same<To, float>{})
                    || (std::is_same<Ti, rocblas_double_complex>{} && std::is_same<To, double>{})))

            || ((BLAS1 == blas1::rotm || BLAS1 == blas1::rotm_batched
                 || BLAS1 == blas1::rotm_strided_batched)
                && std::is_same<To, Ti>{} && std::is_same<To, Tc>{}
                && (std::is_same<Ti, float>{} || std::is_same<Ti, double>{}))

            || ((BLAS1 == blas1::rotmg || BLAS1 == blas1::rotmg_batched
                 || BLAS1 == blas1::rotmg_strided_batched)
                && std::is_same<To, Ti>{} && std::is_same<To, Tc>{}
                && (std::is_same<Ti, float>{} || std::is_same<Ti, double>{}))>;

// Creates tests for one of the BLAS 1 functions
// ARG passes 1-3 template arguments to the testing_* function
#define BLAS1_TESTING(NAME, ARG)                                                             \
    struct blas1_##NAME                                                                      \
    {                                                                                        \
        template <typename Ti, typename To = Ti, typename Tc = To, typename = void>          \
        struct testing : rocblas_test_invalid                                                \
        {                                                                                    \
        };                                                                                   \
                                                                                             \
        template <typename Ti, typename To, typename Tc>                                     \
        struct testing<Ti, To, Tc, std::enable_if_t<rot_enabled<blas1::NAME, Ti, To, Tc>{}>> \
            : rocblas_test_valid                                                             \
        {                                                                                    \
            void operator()(const Arguments& arg)                                            \
            {                                                                                \
                if(!strcmp(arg.function, #NAME))                                             \
                    testing_##NAME<ARG(Ti, To, Tc)>(arg);                                    \
                else if(!strcmp(arg.function, #NAME "_bad_arg"))                             \
                    testing_##NAME##_bad_arg<ARG(Ti, To, Tc)>(arg);                          \
                else                                                                         \
                    FAIL() << "Internal error: Test called with unknown function: "          \
                           << arg.function;                                                  \
            }                                                                                \
        };                                                                                   \
    };                                                                                       \
                                                                                             \
    using NAME = rot_test_template<blas1_##NAME::template testing, blas1::NAME>;             \
                                                                                             \
    template <>                                                                              \
    inline bool NAME::function_filter(const Arguments& arg)                                  \
    {                                                                                        \
        return !strcmp(arg.function, #NAME) || !strcmp(arg.function, #NAME "_bad_arg");      \
    }                                                                                        \
                                                                                             \
    TEST_P(NAME, blas1)                                                                      \
    {                                                                                        \
        RUN_TEST_ON_THREADS_STREAMS(                                                         \
            rocblas_blas1_dispatch<blas1_##NAME::template testing>(GetParam()));             \
    }                                                                                        \
                                                                                             \
    INSTANTIATE_TEST_CATEGORIES(NAME)

#define ARG1(Ti, To, Tc) Ti
#define ARG2(Ti, To, Tc) Ti, To
#define ARG3(Ti, To, Tc) Ti, To, Tc

    BLAS1_TESTING(rot, ARG3)
    BLAS1_TESTING(rot_batched, ARG3)
    BLAS1_TESTING(rot_strided_batched, ARG3)
    BLAS1_TESTING(rotg, ARG2)
    BLAS1_TESTING(rotg_batched, ARG2)
    BLAS1_TESTING(rotg_strided_batched, ARG2)
    BLAS1_TESTING(rotm, ARG1)
    BLAS1_TESTING(rotm_batched, ARG1)
    BLAS1_TESTING(rotm_strided_batched, ARG1)
    BLAS1_TESTING(rotmg, ARG1)
    BLAS1_TESTING(rotmg_batched, ARG1)
    BLAS1_TESTING(rotmg_strided_batched, ARG1)

} // namespace
