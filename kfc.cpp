#include "kfc.h"

int main(int argc, char *argv[]){

    //初始化
    let cfroot = readJsonFromFile("kfc_config.json");
    let configFile = cfroot["config"].asString();
    let infoFile = cfroot["info"].asString();
    let root = readJsonFromFile(configFile);
    let aliases = parseFgAlias(infoFile);

    //创建一个kif类型的vector用来存储后面的对象
    let img = parseKIF(root);

    //创建base，eye，eyebrow，mouth，cheek
    let base = sortImgForBase(img);
    let eye_id = getImgItemId(img, 2, "目");
    let eyebrow_id = getImgItemId(img, 2, "眉");
    let mouth_id = getImgItemId(img, 2, "口");
    let cheek_id = getImgItemId(img, 2, "頬");
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
            let cheekbox = getItem(cheek, alias.cheek);
            work(base[i], eyebox, eyebrowbox, mouthbox, cheekbox, alias.name, perfix);
        }
    }

    return 0;

}