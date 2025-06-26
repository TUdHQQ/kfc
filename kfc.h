#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#define let const auto
#define fn inline const auto

namespace fs = std::filesystem;

// KIF: Krkr Image File
// 创建一种结构体，用来存储每一个图片文件的详细数据
struct kif {
  std::string name;
  int width, left, height, top, layer_id, group_layer_id;
  // 判断是group还是file
  int type;
  fn is_group() { return type == 0; }
};

// 创建一个结构体，用来存储base的图片数据
struct base {
  kif info;
};

// 创建一个结构体，用来存储diff的图片数据
struct diff {
  kif info;
};

// 创建一个结构体，用来存储fgname的图片数据
struct fgname {
  std::string name;
  bool is_dummy;
  kif info;
};

// 创建一个结构体，用来存储fgalias
struct fgalias {
  std::string name;
  std::shared_ptr<std::vector<fgname>> list;
};

// 来源: listder.h
fn overlayImages(const cv::Mat &background, const cv::Mat &foreground,
                 cv::Mat &output, int x, int y) {
  cv::Rect roi(x, y, std::min(foreground.cols, background.cols - x),
               std::min(foreground.rows, background.rows - y));

  if (roi.width <= 0 || roi.height <= 0) {
    output = background.clone();
    return;
  }

  output = background.clone();
  cv::Mat roi_output = output(roi);
  cv::Mat roi_foreground = foreground(cv::Rect(0, 0, roi.width, roi.height));

// 使用并行处理来加速计算
#pragma omp parallel for
  for (int i = 0; i < roi.height; ++i) {
    for (int j = 0; j < roi.width; ++j) {
      const cv::Vec4b &pixel_foreground = roi_foreground.at<cv::Vec4b>(i, j);
      cv::Vec4b &pixel_output = roi_output.at<cv::Vec4b>(i, j);

      float alpha_foreground = pixel_foreground[3] / 255.0f;
      float alpha_background = 1.0f - alpha_foreground;

      for (int k = 0; k < 3; ++k) {
        pixel_output[k] =
            cv::saturate_cast<uchar>(alpha_foreground * pixel_foreground[k] +
                                     alpha_background * pixel_output[k]);
      }
      pixel_output[3] = cv::saturate_cast<uchar>(
          alpha_foreground * 255 + alpha_background * pixel_output[3]);
    }
  }
}

// 分割字符串
fn split(const std::string &str, char delim) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
    tokens.push_back(token);
  }
  return tokens;
}

// 从sinfo中读取数据
fn readSinfo(const std::string &filename, std::map<std::string, kif> &data,
             std::vector<base> &bases, std::vector<diff> &diffs,
             std::map<std::string, fgname> &fgnames,
             std::vector<fgalias> &fgaliases) {
  std::ifstream file(filename);
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }
    auto tokens = split(line, '\t');
    if (tokens[0] == "dress") {
      if (tokens[2] == "base")
        bases.push_back({data[tokens[4]]});
      if (tokens[2] == "diff")
        diffs.push_back({data[tokens[4]]});
    }
    if (tokens[0] == "fgname") {
      fgnames[tokens[1]] = {tokens[1], tokens[2] == "dummy", data[tokens[2]]};
    }
    if (tokens[0] == "fgalias") {
      fgaliases.push_back({tokens[1], std::make_shared<std::vector<fgname>>()});
      for (int i = 2; i < tokens.size(); i++) {
        if (fgnames[tokens[i]].is_dummy)
          continue;
        fgaliases.back().list->push_back(fgnames[tokens[i]]);
      }
    }
  }
  file.close();
}

// 用来读取json文件
fn readJson(const std::string &file) {
  std::ifstream jsonfile(file, std::ifstream::binary);
  Json::Value root;
  Json::CharReaderBuilder readerBuilder;
  std::string errs;

  if (!Json::parseFromStream(readerBuilder, jsonfile, &root, &errs)) {
    throw std::runtime_error("Failed to parse JSON file: " + file +
                             ", error: " + errs);
  }

  jsonfile.close();

  return root;
}

fn readData(std::string file) {
  Json::Value root = readJson(file);
  std::vector<kif> img;
  std::map<std::string, kif> data;

  for (auto &item : root) {
    if (!item.isMember("name")) {
      continue;
    }
    img.push_back(
        {item["name"].asString(), item["width"].asInt(), item["left"].asInt(),
         item["height"].asInt(), item["top"].asInt(), item["layer_id"].asInt(),
         item.isMember("group_layer_id") ? item["group_layer_id"].asInt()
                                         : item["layer_id"].asInt(),
         item["layer_type"].asInt() == 2 ? 0 : 1});
  }
  for (int i = 0; i < img.size(); i++) {
    std::string perfix = "";
    if (img[i].is_group())
      data[img[i].name] = img[i];
    for (int j = i + 1; j < img.size(); j++)
      if (img[i].group_layer_id == img[j].layer_id)
        perfix = img[j].name + '/';
    data[perfix + img[i].name] = img[i];
    data[perfix + img[i].name].name = perfix + img[i].name;
  }
  return data;
}

fn readConfig() { return readJson(fs::current_path() / "kfc_config.json"); }

fn getxpos(kif base, kif face) { return abs(base.left - face.left); }

fn getypos(kif base, kif face) { return abs(base.top - face.top); }

fn getPath(const std::string &perfix, int &id) {
  return fs::current_path() / (perfix + std::to_string(id) + ".png");
}

fn work(const fs::path &outpath, const std::string &perfix, diff &d,
        fgalias &fg) {
  let baseimg =
      cv::imread(getPath(perfix, d.info.layer_id), cv::IMREAD_UNCHANGED);
  auto output = baseimg.clone();
  for (auto &fgname : *fg.list)
    overlayImages(
        output,
        cv::imread(getPath(perfix, fgname.info.layer_id), cv::IMREAD_UNCHANGED),
        output, getxpos(d.info, fgname.info), getypos(d.info, fgname.info));
  if (outpath.parent_path() != fs::current_path())
    fs::create_directories(outpath.parent_path());
  cv::imwrite(outpath, output);
}