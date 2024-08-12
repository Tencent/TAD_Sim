/**
 * @file onnx_test.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-07-12
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <cuda_runtime.h>
#include <boost/filesystem.hpp>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include "../tadsim_trt_engine.h"


namespace fs = boost::filesystem;
std::vector<std::string> findfile(const std::string &dir, const std::string &ext) {
  std::vector<std::string> files;
  if (!fs::is_directory(dir)) return files;
  fs::path fullpath(dir);
  fs::directory_iterator end_iter;
  for (fs::directory_iterator iter(fullpath); iter != end_iter; ++iter) {
    try {
      if (!fs::is_directory(*iter)) {
        if (iter->path().extension() == ext) files.push_back(iter->path().stem().string());
      }
    } catch (const std::exception &ex) {
      std::cout << ex.what() << std::endl;
      continue;
    }
  }
  return files;
}

int main(int argc, char **argv) {
  std::cout << "dir onnx dir wid hei\n";
  std::string src = argv[1];
  std::string dir;
  std::string outdir_orgial, outdir_style, outdir_compa, outdir_video;
  std::string optdir;
  std::vector<std::string> files;
  std::vector<std::string> optfiles;
  if (fs::is_directory(src)) {
    files = findfile(src, ".jpg");
    dir = src;
    outdir_orgial = dir + "/out/orgial/";
    outdir_style = dir + "/out/style/";
    outdir_compa = dir + "/out/compare/";
    outdir_video = dir + "/out/";
    try {
      fs::create_directory(outdir_video);
      fs::create_directory(outdir_orgial);
      fs::create_directory(outdir_style);
      fs::create_directory(outdir_compa);
    } catch (const std::exception &) {
    }
  } else {
    files.push_back(fs::path(src).stem().string());
    dir = fs::path(src).parent_path().string();
    outdir_orgial = dir + "/orgial_";
    outdir_style = dir + "/style_";
    outdir_compa = dir + "/compare_";
    outdir_video = dir + "/";
  }
  std::cout << files.size() << " jpg\n";

  std::string model = "generator.opt.onnx";
  if (argc > 2) {
    model = argv[2];
  }
  if (argc > 3) {
    optdir = argv[3];
    optfiles = findfile(optdir, ".jpg");
  }

  TADOnnxParams para;
  para.width = 1408;
  para.height = 768;

  strcpy(para.onnxmodel, model.c_str());
  strcpy(para.cachetrt, (model + ".cachetrt.0").c_str());

  if (argc > 4) {
    para.width = std::atoi(argv[4]);
  }
  if (argc > 5) {
    para.height = std::atoi(argv[5]);
  }

  TADSimTRT rt(para);

  uchar *gpu_bgra = 0;
  cudaMalloc(&gpu_bgra, para.width * para.height * 4 * sizeof(uchar));

  cv::VideoWriter writer;
  int merw = optdir.empty() ? 2 : 3;
  if (files.size() > 10) {
    if (!writer.open(outdir_video + "out.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 10,
                     cv::Size(para.width * merw, para.height), true))
      std::cout << "error video.\n";
  }
  for (size_t i = 0; i < files.size(); ++i) {
    const auto &tname = files[i];
    std::cout << "\r" << i << " / " << files.size();
    cv::Mat bgr = cv::imread(dir + "/" + tname + ".jpg");
    cv::resize(bgr, bgr, cv::Size(para.width, para.height));
    cv::imwrite(outdir_orgial + tname + ".jpg", bgr);
    cv::Mat bgra(bgr.cols, bgr.rows, CV_8UC4);
    cv::Mat merimg(para.height, para.width * merw, CV_8UC4);

    cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);
    bgra.copyTo(merimg(cv::Rect(0, 0, para.width, para.height)));

    cudaMemcpy(gpu_bgra, bgra.ptr(), para.width * para.height * 4, cudaMemcpyHostToDevice);

    rt.infer(gpu_bgra);

    cudaMemcpy(bgra.ptr(), gpu_bgra, para.width * para.height * 4, cudaMemcpyDeviceToHost);
    cv::imwrite(outdir_style + tname + ".jpg", bgra);
    bgra.copyTo(merimg(cv::Rect(para.width, 0, para.width, para.height)));
    if (i < optfiles.size()) {
      bgr = cv::imread(optdir + "/" + optfiles[i] + ".jpg");
      cv::resize(bgr, bgr, cv::Size(para.width, para.height));
      cv::cvtColor(bgr, merimg(cv::Rect(para.width * 2, 0, para.width, para.height)), cv::COLOR_BGR2BGRA);
    }

    cv::imwrite(outdir_compa + tname + ".jpg", merimg);

    if (writer.isOpened()) {
      cv::Mat bgr;
      cv::cvtColor(merimg, bgr, cv::COLOR_BGRA2BGR);
      writer.write(bgr);
    }
  }
  writer.release();
  cudaFree(gpu_bgra);
}
