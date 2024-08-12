/* Copyright 2010 The MathWorks, Inc. */
#ifndef RTW_HEADER_rt_math_h_
#define RTW_HEADER_rt_math_h_

#define rt_MAXf_snf(a, b) ((((a) >= (b)) || rtIsNaNF(b)) ? (a) : (b))
#define rt_MAXd_snf(a, b) ((((a) >= (b)) || rtIsNaN(b)) ? (a) : (b))
#define rt_MAXf(a, b) (((a) >= (b)) ? (a) : (b))
#define rt_MAXd(a, b) (((a) >= (b)) ? (a) : (b))
#define rt_MAX(a, b) (((a) >= (b)) ? (a) : (b))

#define rt_MINf_snf(a, b) ((((a) <= (b)) || rtIsNaNF(b)) ? (a) : (b))
#define rt_MINd_snf(a, b) ((((a) <= (b)) || rtIsNaN(b)) ? (a) : (b))
#define rt_MINf(a, b) (((a) <= (b)) ? (a) : (b))
#define rt_MINd(a, b) (((a) <= (b)) ? (a) : (b))
#define rt_MIN(a, b) (((a) <= (b)) ? (a) : (b))

#define rt_SIGNf_snf(a) (((a) < 0.0F) ? -1.0F : ((a) > 0.0F) ? 1.0F : ((a) == 0.0F) ? 0.0F : (rtNaNF))
#define rt_SIGNd_snf(a) (((a) < 0.0) ? -1.0 : ((a) > 0.0) ? 1.0 : ((a) == 0.0) ? 0.0 : (rtNaN))
#define rt_SIGNf(a) (((a) < 0.0F) ? -1.0F : ((a) > 0.0F) ? 1.0F : 0.0F)
#define rt_SIGNd(a) (((a) < 0.0) ? -1.0 : ((a) > 0.0) ? 1.0 : 0.0)
#define rt_SGN(a) (((a) < 0) ? -1 : ((a) > 0) ? 1 : 0)
#define rt_UNSGN(a) (((a) > 0U) ? 1U : 0U)

#define rt_SATURATE(sig, ll, ul) (((sig) >= (ul)) ? (ul) : (((sig) <= (ll)) ? (ll) : (sig)))

#define rt_ABS(a) (((a) >= 0) ? (a) : -(a))

#endif /* RTW_HEADER_rt_math_h_ */
