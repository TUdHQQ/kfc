#include "kfc.h"

//用来查找 layer_id
int findkif(std::vector<kif> img, int find_layer_id){
    for(int i = 0; i < img.size(); i++){
        if(img[i].is_layer_id(find_layer_id)) return i;
    }
    return -1;
}

//用来确定x和y
int getxpos(kif base,kif face){
    return abs(base.left - face.left);
}
int getypos(kif base,kif face){
    return abs(base.top - face.top);
}


int main(int argc, char *argv[]){

    //读取json文件
    std::ifstream jsonfile(argv[1], std::ifstream::binary);
    std::ifstream cfjsonfile("kfc_config.json", std::ifstream::binary);
    Json::Value root, cfroot;
    Json::CharReaderBuilder readerBuilder, cfreaderBuilder;
    std::string errs, cferrs;

    //解析json文件
    if (!Json::parseFromStream(readerBuilder, jsonfile, &root, &errs)) {
        std::cerr << "parse fail!" << errs << std::endl;
        return 1;
    }
    if (!Json::parseFromStream(cfreaderBuilder, cfjsonfile, &cfroot, &cferrs)) {
        std::cerr << "parse fail!" << cferrs << std::endl;
        return 1;
    }

    //读取第一个对象里面的 width 和 height
    int krkr_width = root[0]["width"].asInt();
    int krkr_height = root[0]["height"].asInt();

    //创建一个kif类型的vector用来存储后面的对象
    std::vector<kif> img;

    //遍历对象
    for(int i = 1; i < root.size(); i++){
        
        //创建temp，用来临时存储对象中的数据
        kif temp;
        temp.name = root[i]["name"].asString();
        temp.left = root[i]["left"].asInt();
        temp.top = root[i]["top"].asInt();
        temp.layer_id = root[i]["layer_id"].asInt();
        
        //判断group_layer_id是否存在
        if(root[i].isMember("group_layer_id")){
            temp.isfile = 1;
            temp.group_layer_id = root[i]["group_layer_id"].asInt();
        }
        else temp.isfile = 0;

        //将temp添加到vector里面
        img.push_back(temp);

        //std::cout << temp.name << " " << temp.left << " " << temp.top << " " << temp.layer_id << " " << temp.isfile;
        //std::cout << std::endl;
    }

    //将congfig里面的数据存储起来
    std::string cf_perfix, temp = argv[1];
    cf_perfix = temp.substr(0, temp.size() - 5);
    const Json::Value base = cfroot["base"];
    const Json::Value face = cfroot["face"];
    //std::cout << cf_perfix;

    //创建output文件夹
    std::string outputpath = "output";
    _mkdir(outputpath.c_str());

    //遍历配置文件中的base和face数组
    for(int i = 0; i < base.size(); i++){
        for(int j = 0; j < face.size(); j++){
            int base_pos = findkif(img, base[i].asInt());
            int face_pos = findkif(img, face[j].asInt());

            if(base_pos == -1 || face_pos == -1) {
                std::cerr << "layer_id not found" <<  std::endl;
                return 1;
            }

            int x = getxpos(img[base_pos], img[face_pos]);
            int y = getypos(img[base_pos], img[face_pos]);

            std::string outputname = ".\\" + outputpath + '\\' + cf_perfix + '_' + std::to_string(base[i].asInt()) + '_' + std::to_string(face[j].asInt()) + ".png";
            std::string basestring = cf_perfix + '_' + std::to_string(base[i].asInt()) + ".png";
            std::string facestring = cf_perfix + '_' + std::to_string(face[j].asInt()) + ".png";
            cv::Mat basepng = cv::imread(basestring, cv::IMREAD_UNCHANGED);
            cv::Mat facepng = cv::imread(facestring, cv::IMREAD_UNCHANGED);
            cv::Mat output;
            overlayImages(basepng, facepng, output, x, y);
            imwrite(outputname, output);
            printf("saving...   %d_%d.png\n", base[i].asInt(),face[j].asInt());
        }
    }

    return 0;

}