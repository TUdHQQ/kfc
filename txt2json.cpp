#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>

std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

Json::Value convertToJson(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    Json::Value root(Json::arrayValue);

    std::getline(file, line);
    
    std::getline(file, line);
    std::vector<std::string> size = split(line, '\t');
    Json::Value canvas;
    canvas["width"] = 3660;
    canvas["height"] = 6000;
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

void writeJson(Json::Value &root, const std::string &file) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true; 

    std::ofstream output(file);
    if (!output.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &output);

    output.close();
}

int main(int argc, char* argv[]) {
    std::string filename = argv[1];
    Json::Value root = convertToJson(filename);
    
    writeJson(root, "output.json");

    return 0;
}