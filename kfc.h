#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <opencv2/opencv.hpp>
#include <io.h>
#include <direct.h>

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
void overlayImages(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, int x, int y){
    if (background.cols < x + foreground.cols || background.rows < y + foreground.rows){
        output = foreground.clone();
        cv::Rect roi(x, y, background.cols, background.rows);
        roi &= cv::Rect(0, 0, foreground.cols, foreground.rows);
        cv::Mat roi_output = output(roi);
        cv::Mat roi_background = background(cv::Rect(0, 0, roi.width, roi.height));
        for (int i = 0; i < roi.height; ++i){
            for (int j = 0; j < roi.width; ++j){
                cv::Vec4b pixel_background = roi_background.at<cv::Vec4b>(i, j);
                cv::Vec4b &pixel_output = roi_output.at<cv::Vec4b>(i, j);
                double alpha_background = pixel_background[3] / 255.0;
                double alpha_foreground = 1.0 - alpha_background;
                for (int k = 0; k < 3; ++k){
                    pixel_output[k] = static_cast<uchar>(alpha_background * pixel_background[k] + alpha_foreground * pixel_output[k]);
                }
                pixel_output[3] = static_cast<uchar>((alpha_background * 255) + (alpha_foreground * pixel_output[3]));
            }
        }
        cv::Mat temp=output;
        for (int y = 0; y < foreground.rows; ++y) {
            for (int x = 0; x < foreground.cols; ++x) {
                cv::Vec4b pixel1 = foreground.at<cv::Vec4b>(y, x);
                cv::Vec4b pixel2 = temp.at<cv::Vec4b>(y, x);
                if (pixel1[3] > 0) {
                    output.at<cv::Vec4b>(y, x) = pixel1;
                } else {
                    output.at<cv::Vec4b>(y, x) = pixel2;
                }
            }
        }
    }
    else{
        output = background.clone();
        cv::Rect roi(x, y, foreground.cols, foreground.rows);
        roi &= cv::Rect(0, 0, background.cols, background.rows);
        cv::Mat roi_output = output(roi);
        cv::Mat roi_foreground = foreground(cv::Rect(0, 0, roi.width, roi.height));
        for (int i = 0; i < roi.height; ++i){
            for (int j = 0; j < roi.width; ++j){
                cv::Vec4b pixel_foreground = roi_foreground.at<cv::Vec4b>(i, j);
                cv::Vec4b &pixel_output = roi_output.at<cv::Vec4b>(i, j);
                double alpha_foreground = pixel_foreground[3] / 255.0;
                double alpha_background = 1.0 - alpha_foreground;
                for (int k = 0; k < 3; ++k){
                    pixel_output[k] = static_cast<uchar>(alpha_foreground * pixel_foreground[k] + alpha_background * pixel_output[k]);
                }
                pixel_output[3] = static_cast<uchar>((alpha_foreground * 255) + (alpha_background * pixel_output[3]));
            }
        }
    }
}

//来源: listder.h
void getFileNames(std::string path, std::vector<std::string>& files){
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1){
		do{
			if ((fileinfo.attrib & _A_SUBDIR)){
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}