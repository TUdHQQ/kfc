#include "kfc.h"

int main(int argc, char *argv[]){

    //初始化
    let cfroot = readJsonFromFile("kfc_config.json");
    let configFile = cfroot["config"].asString();
    let infoFile = cfroot["info"].asString();
    let root = readJsonFromFile(configFile);
    let aliases = parseFgAlias(infoFile);
    
    //读取第一个对象里面的 width 和 height
    //int krkr_width = root[0]["width"].asInt();
    //int krkr_height = root[0]["height"].asInt();
    //没啥用，删掉了

    //创建一个kif类型的vector用来存储后面的对象
    let img = parseKIF(root);

    //创建base，eye，eyebrow，mouth，cheek
    let base = sortImgForBase(img);
    let eye_id = getImgItenId(img, 2, "目");
    let eyebrow_id = getImgItenId(img, 2, "眉");
    let mouth_id = getImgItenId(img, 2, "口");
    let cheek_id = getImgItenId(img, 2, "頬");
    let eye = sortImgById(img, eye_id,"目_");
    let eyebrow = sortImgById(img, eyebrow_id,"眉_");
    let mouth = sortImgById(img, mouth_id,"口_");
    let cheek = sortImgById(img, cheek_id,"頬_");

    //创建output文件夹
    let outputpath = "output";
    fs::create_directory(outputpath);

    fs::path temppath = configFile;
    let perfix = temppath.stem().string();

    //遍历所有需要合成的立绘
    for(int i = 0; i < base.size(); i++){
        for (const auto& alias : aliases){
            let eyebox = getItem(eye, alias.eye);
            let eyebrowbox = getItem(eyebrow, alias.eyebrow);
            let mouthbox = getItem(mouth, alias.mouth);
            //std::cout<<alias.eye<<" "<<alias.eyebrow<<" "<<alias.mouth<<std::endl;
            //std::cout<<eyebox.name<<" "<<eyebrowbox.name<<" "<<mouthbox.name<<std::endl;
            //std::cout<<"--------------------------------"<<std::endl;
            work(base[i], eyebox, eyebrowbox, mouthbox, alias.name,perfix);
        }
    }

    return 0;

}