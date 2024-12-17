#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <opencv2/opencv.hpp>
#include <filesystem>

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
void overlayImages(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, int x, int y) {
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

