#include "opencv2/core/base.hpp"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <json/value.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>

#define let const auto

namespace fs = std::filesystem;

//KIF: Krkr Image File
//创建一种结构体，用来存储每一个图片文件的详细数据
struct kif{
    std::string name;
    int width,left,height,top,layer_id,group_layer_id;
    //判断是group还是file
    int type;
    bool is_layer_id(int lid){
        return lid == layer_id;
    }
};

//用来存储info.txt的数据
struct FaceAlias {
    std::string name;      // 表情名称
    std::string eyebrow;   // 眉毛
    std::string eye;       // 眼睛
    std::string mouth;     // 嘴巴
    std::string cheek;     // 脸颊
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

//分割字符串
inline std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

//用于将txt格式转换为json
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

//从info中读取数据
inline std::vector<FaceAlias> parseFgAlias(const std::string& filename) {
    std::vector<FaceAlias> aliases;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.find("fgalias") == 0) {
            auto tokens = split(line,'\t');
            if (tokens.size() >= 6) {
                FaceAlias alias {
                    tokens[1],  // 表情名称
                    tokens[2],  // 眉毛
                    tokens[3],  // 眼睛
                    tokens[4],  // 嘴巴
                    tokens[5]   // 脸颊
                };
                aliases.push_back(alias);
            }
        }
    }
    return aliases;
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
inline int findkif(std::vector<kif> &img, int find_layer_id){
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
    let filepath = fs::path(file);
    if(filepath.extension().compare(".txt") == 0){
        root = convertToJson(file);
    }
    else{
        root = readJson(file);
    }
    
    return root;

}

//保存到img中
inline std::vector<kif> parseKIF(const Json::Value root){
    std::vector<kif> img;
    int faceid = 0;
    for(int i = 1; i < root.size(); i++){
        
        kif temp;
        temp.name = root[i]["name"].asString();
        temp.left = root[i]["left"].asInt();
        temp.top = root[i]["top"].asInt();
        temp.layer_id = root[i]["layer_id"].asInt();
        
        //判断group_layer_id是否存在
        if(root[i].isMember("group_layer_id")){
            temp.type = 0;
            temp.group_layer_id = root[i]["group_layer_id"].asInt();
            if(temp.group_layer_id == faceid) temp.type = 2;
        }
        else temp.type = 1;

        if(temp.name == "表情"){
            faceid = temp.layer_id;
            continue ;
        }

        //将temp添加到vector里面
        img.push_back(temp);

        //std::cout << temp.name << " " << temp.left << " " << temp.top << " " << temp.layer_id << " " << temp.type;
        //std::cout << std::endl;
    }
    return img;
}

inline std::vector<kif> sortImg(const std::vector<kif>& img,const int& type){
    std::vector<kif> item;
    for(int i = 0; i < img.size(); i++){
        if(img[i].type == type) item.push_back(img[i]);
    }
    return item;
}

inline int getImgItemId(const std::vector<kif>& img,const int& type,const std::string& name){
    std::vector<kif> item = sortImg(img,type);
    for (int i = 0; i < item.size(); i++){
        if(item[i].name == name) return item[i].layer_id;
    }
    return -1;
}

inline std::vector<kif> sortImgById(const std::vector<kif>& img,const int& id,const std::string& perfix){
    std::vector<kif> item;
    for(int i = 0; i < img.size(); i++){
        if(img[i].group_layer_id == id) {
            kif temp = img[i];
            temp.name = perfix + img[i].name;
            item.push_back(temp);
        }
    }
    return item;
}

inline kif getItem(const std::vector<kif>& item,const std::string name){
    kif temp;
    const std::string cmp = name;
    for(int i = 0; i < item.size(); i++){
        if(item[i].name == cmp) temp = item[i];
    }
    return temp;
}

inline std::vector<kif> sortImgForBase(const std::vector<kif>& img){
    std::vector<kif> item;
    for(int i = 0; i < img.size(); i++){
        kif temp = img[i];
        int id = img[i].layer_id;
        if(img[i].type == 1) {
            for(int j = 0; j < img.size(); j++){
                if(img[j].group_layer_id == id){
                    temp.top = img[j].top;
                    temp.left = img[j].left;
                    temp.width = img[j].width;
                    temp.height = img[j].height;
                    temp.layer_id = img[j].layer_id;
                    item.push_back(temp);
                    break;
                }
            }
        }
    }
    return item;
}

inline std::string getPath(const std::string& perfix,const kif& item){
    return perfix + "_" + std::to_string(item.layer_id)+ ".png";
}

inline void work(const kif& base,const kif& eye,const kif& eyebrow,const kif& mouth,const kif& cheek,const std::string name,const std::string perfix){
    let basepath = getPath(perfix, base);
    let eyepath = getPath(perfix, eye);
    let eyebrowpath = getPath(perfix, eyebrow);
    let mouthpath = getPath(perfix, mouth);
    let cheekpath = getPath(perfix, cheek);
    let eyex = getxpos(base, eye);
    let eyey = getypos(base, eye);
    let eyebrowx = getxpos(base, eyebrow);
    let eyebrowy = getypos(base, eyebrow);
    let mouthx = getxpos(base, mouth);
    let mouthy = getypos(base, mouth);
    let cheekx = getxpos(base, cheek);
    let cheeky = getypos(base, cheek);

    let baseimg = cv::imread(basepath, cv::IMREAD_UNCHANGED);
    let eyeimg = cv::imread(eyepath, cv::IMREAD_UNCHANGED);
    let eyebrowimg = cv::imread(eyebrowpath, cv::IMREAD_UNCHANGED);
    let mouthimg = cv::imread(mouthpath, cv::IMREAD_UNCHANGED);
    let cheekimg = cv::imread(cheekpath, cv::IMREAD_UNCHANGED);

    auto output = baseimg.clone();

    overlayImages(baseimg, eyeimg,output, eyex, eyey);
    overlayImages(output, eyebrowimg,output, eyebrowx, eyebrowy);
    overlayImages(output, mouthimg,output, mouthx, mouthy);
    overlayImages(output, cheekimg,output, cheekx, cheeky);

    fs::path outputpath = fs::path("output") / fs::path(perfix + "_" + name + "_"+ base.name + ".png");
    cv::imwrite(outputpath.string(), output);
    std::cout<<"saving..... "<<outputpath<<std::endl;
}