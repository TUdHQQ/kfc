#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace fs = std::filesystem;

//KIF: Krkr Image File
//创建一种结构体，用来存储每一个图片文件的详细数据
struct kif{
    std::string name;
    int width,left,height,top,layer_id,group_layer_id;
    //判断是group还是file
    bool isfile;
    bool is_layer_id(int lid){
        return lid == layer_id;
    }
};


//来源: listder.h
inline void overlayImages(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, int x, int y) {
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
            const cv::Vec4b& pixel_foreground = roi_foreground.at<cv::Vec4b>(i, j);
            cv::Vec4b& pixel_output = roi_output.at<cv::Vec4b>(i, j);

            float alpha_foreground = pixel_foreground[3] / 255.0f;
            float alpha_background = 1.0f - alpha_foreground;

            for (int k = 0; k < 3; ++k) {
                pixel_output[k] = cv::saturate_cast<uchar>(
                    alpha_foreground * pixel_foreground[k] + alpha_background * pixel_output[k]
                );
            }
            pixel_output[3] = cv::saturate_cast<uchar>(
                alpha_foreground * 255 + alpha_background * pixel_output[3]
            );
        }
    }
}

//用于将txt格式转换为json
inline std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}
inline Json::Value convertToJson(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    Json::Value root(Json::arrayValue);

    std::getline(file, line);
    
    std::getline(file, line);
    std::vector<std::string> size = split(line, '\t');
    Json::Value canvas;
    canvas["width"] = 3000;
    canvas["height"] = 5000;
    root.append(canvas);

    while (std::getline(file, line)) {
        if(line.empty()) continue;
        
        std::vector<std::string> values = split(line, '\t');
        Json::Value layer;
        
        try {
            layer["layer_type"] = std::stoi(values[0]);
            layer["name"] = values[1];
            layer["left"] = std::stoi(values[2]);
            layer["top"] = std::stoi(values[3]);
            layer["width"] = std::stoi(values[4]);
            layer["height"] = std::stoi(values[5]);
            layer["type"] = std::stoi(values[6]);
            layer["opacity"] = std::stoi(values[7]);
            layer["visible"] = std::stoi(values[8]);
            layer["layer_id"] = std::stoi(values[9]);
            
            if(values.size() > 10 && !values[10].empty()) {
                layer["group_layer_id"] = std::stoi(values[10]);
            }
            
            root.append(layer);
        } catch(...) {
            std::cerr << "Error parsing line: " << line << std::endl;
            continue;
        }
    }

    return root;
}


//用来读取json文件
inline Json::Value readJson(const std::string &file) {
    std::ifstream jsonfile(file, std::ifstream::binary);
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, jsonfile, &root, &errs)) {
        throw std::runtime_error("Failed to parse JSON file: " + file + ", error: " + errs);
    }

    return root;
}

//用来查找 layer_id
inline int findkif(std::vector<kif> img, int find_layer_id){
    for(int i = 0; i < img.size(); i++){
        if(img[i].is_layer_id(find_layer_id)) return i;
    }
    return -1;
}

//用来确定x和y
inline int getxpos(kif base,kif face){
    return abs(base.left - face.left);
}
inline int getypos(kif base,kif face){
    return abs(base.top - face.top);
}

//读取配置文件
inline Json::Value readJsonFromFile(const std::string &file){

    Json::Value root;
    fs::path filepath = fs::path(file);
    if(filepath.extension().compare(".txt") == 0){
        root = convertToJson(file);
    }
    else{
        root = readJson(file);
    }
    
    return root;

}