#include "CudaLidarModel.h"
#include "CudaCheck.h"

#define POSSON_STDDEV 0.03464f

#ifdef FIND_CUDA

#include "lidar_simulation.h"

#undef checkCudaErrors
#undef checkCudaErrors_TF
#define checkCudaErrors(val) cuda_check((val), "LidarModel", __LINE__)
#define checkCudaErrors_TF(val)                     \
    if (!cuda_check((val), "LidarModel", __LINE__)) \
        return false;

CudaLidarModel::CudaLidarModel()
{
    sim = MakeShared<cuda_lidar::LidarSimulation>();
    sim->f_accuracy = 0.2f;
    checkCudaErrors(cudaStreamCreate(&stream));
}

CudaLidarModel::~CudaLidarModel()
{
    if (sim->depthImg)
        checkCudaErrors(cudaFree(sim->depthImg));
    if (sim->refmap)
        checkCudaErrors(cudaFree(sim->refmap));
    if (sim->tagmap)
        checkCudaErrors(cudaFree(sim->tagmap));
    if (sim->camidx)
        checkCudaErrors(cudaFree(sim->camidx));
    if (sim->camuv)
        checkCudaErrors(cudaFree(sim->camuv));
    if (sim->yawpitch)
        checkCudaErrors(cudaFree(sim->yawpitch));
    if (out)
        checkCudaErrors(cudaFree(out));
    if (stream)
        checkCudaErrors(cudaStreamDestroy(stream));
}

void CudaLidarModel::set_intensity(float i)
{
    sim->t_intensity = i;
}

void CudaLidarModel::set_rain(float w)
{
    w *= 0.041666667f;
    sim->f_rainfall = exp(-0.02f * pow(w, 0.6f));
    sim->f_noise_dev = POSSON_STDDEV + 0.0245f * w;
}

void CudaLidarModel::set_snow(float s)
{
    if (s <= 1e-5)
    {
        sim->f_snowfall_range = 1000.f;
        sim->f_snowfall_prob = 1.f;
        return;
    }
    sim->f_snowfall_range = 1000.f / s;
    sim->f_snowfall_prob = 1.f - s * 0.03f * 2;
}

void CudaLidarModel::set_fog(float f)
{
    float r = 1060.282f * pow(f, -0.91f);
    sim->f_fog = exp(-0.02f * pow(r, 0.6f));
}

bool CudaLidarModel::set_img(const TArray<uint8_t*>& img)
{
    if (sim->depthImg)
        checkCudaErrors_TF(cudaFree(sim->depthImg));

    checkCudaErrors_TF(cudaMalloc((void**) &sim->depthImg, img.Num() * sizeof(uint8_t*)));
    checkCudaErrors_TF(cudaMemcpy(sim->depthImg, img.GetData(), img.Num() * sizeof(uint8_t*), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_refmap(const TArray<float>& ref)
{
    if (sim->refmap)
        checkCudaErrors_TF(cudaFree(sim->refmap));
    checkCudaErrors_TF(cudaMalloc((void**) &sim->refmap, ref.Num() * sizeof(float)));
    checkCudaErrors_TF(cudaMemcpy(sim->refmap, ref.GetData(), ref.Num() * sizeof(float), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_tagmap(const TArray<uint32_t>& ref)
{
    if (sim->tagmap)
        checkCudaErrors_TF(cudaFree(sim->tagmap));
    checkCudaErrors_TF(cudaMalloc((void**) &sim->tagmap, ref.Num() * sizeof(uint32_t)));
    checkCudaErrors_TF(cudaMemcpy(sim->tagmap, ref.GetData(), ref.Num() * sizeof(uint32_t), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_camidx(const TArray<uint8_t>& idx)
{
    if (sim->camidx)
        checkCudaErrors_TF(cudaFree(sim->camidx));
    checkCudaErrors_TF(cudaMalloc((void**) &sim->camidx, idx.Num() * sizeof(uint8_t)));
    checkCudaErrors_TF(cudaMemcpy(sim->camidx, idx.GetData(), idx.Num() * sizeof(uint8_t), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_camuv(const TArray<uint32_t>& uv)
{
    if (sim->camuv)
        checkCudaErrors_TF(cudaFree(sim->camuv));
    checkCudaErrors_TF(cudaMalloc((void**) &sim->camuv, uv.Num() * sizeof(uint32_t)));
    checkCudaErrors_TF(cudaMemcpy(sim->camuv, uv.GetData(), uv.Num() * sizeof(uint32_t), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_yawpitch(const TArray<float>& yp)
{
    if (sim->yawpitch)
        checkCudaErrors_TF(cudaFree(sim->yawpitch));
    checkCudaErrors_TF(cudaMalloc((void**) &sim->yawpitch, yp.Num() * sizeof(float)));
    checkCudaErrors_TF(cudaMemcpy(sim->yawpitch, yp.GetData(), yp.Num() * sizeof(float), cudaMemcpyHostToDevice));
    return true;
}

bool CudaLidarModel::set_rtmat(const FTransform& mat)
{
    FMatrix mmat = mat.ToMatrixWithScale();
    if (!sim->rtmat)
    {
        checkCudaErrors_TF(cudaMalloc((void**) &sim->rtmat, 16 * sizeof(float)));
    }
    checkCudaErrors_TF(cudaMemcpy(sim->rtmat, mmat.M, 16 * sizeof(float), cudaMemcpyHostToDevice));
    return false;
}

bool CudaLidarModel::set_rn_hn(uint32_t rn, uint32_t hn)
{
    if (out)
        checkCudaErrors_TF(cudaFree(out));
    checkCudaErrors_TF(cudaMalloc((void**) &out, rn * hn * sizeof(cuda_lidar::lidarsim_point) + 1024));
    checkCudaErrors_TF(cudaGetLastError());
    sim->rhn = rn * hn;
    return true;
}

bool CudaLidarModel::simulation(uint32_t bg_idx, uint32_t N, void* _out)
{
    // return false;
    checkCudaErrors_TF(cudaMemsetAsync((void*) out, 0, N * sizeof(cuda_lidar::lidarsim_point), stream));
    if (!sim->simulation(bg_idx, N, out, stream))
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar sim faild"));
        return false;
    }
    checkCudaErrors_TF(cudaMemcpyAsync(
        (void*) _out, (void*) out, N * sizeof(cuda_lidar::lidarsim_point), cudaMemcpyDeviceToHost, stream));
    checkCudaErrors_TF(cudaStreamSynchronize(stream));
    return true;
}

#else
CudaLidarModel::CudaLidarModel()
{
}
CudaLidarModel::~CudaLidarModel()
{
}
void CudaLidarModel::set_intensity(float i)
{
}
void CudaLidarModel::set_rain(float r)
{
}
void CudaLidarModel::set_snow(float s)
{
}
void CudaLidarModel::set_fog(float f)
{
}
bool CudaLidarModel::set_img(const TArray<uint8_t*>& img)
{
    return false;
}
bool CudaLidarModel::set_refmap(const TArray<float>& ref)
{
    return false;
}
bool CudaLidarModel::set_tagmap(const TArray<uint32_t>& ref)
{
    return false;
}
bool CudaLidarModel::set_camidx(const TArray<uint8_t>& idx)
{
    return false;
}
bool CudaLidarModel::set_camuv(const TArray<uint32_t>& uv)
{
    return false;
}
bool CudaLidarModel::set_yawpitch(const TArray<float>& yp)
{
    return false;
}
bool CudaLidarModel::set_rn_hn(uint32_t rn, uint32_t hn)
{
    return false;
}
bool CudaLidarModel::set_rtmat(const FTransform& mat)
{
    return false;
}
bool CudaLidarModel::simulation(uint32_t bg_idx, uint32_t N, void* out)
{
    return false;
}

#endif
