# kfc
 Krkr Fgimage Covertool

# 如何使用
首先你需要写`kfc_config.json`文件，然后把这个文件放在程序运行目录
接着把解包的立绘文件和立绘的配置文件（形如`氷織Ａ_0.txt`文件和`氷織Ａ_info.txt`）放到程序运行目录下（注意不要嵌套其他文件夹）
然后运行 
```shell
./kfc
```
接着你就会在output文件夹下看到合成的立绘

# 如何写kfc_config

一个叫做info，为info的文件名
一个叫做config，为配置文件的文件名
示例
```json
{
    "info": "氷織Ａ_info.txt",
    "config": "氷織Ａ_0.txt"
}
```


# 编译

```shell
g++ kfc.cpp -o kfc $(pkg-config --cflags --libs opencv jsoncpp)
```

# 特别注意

请确保所有文件的编码为utf-8,并且换行为LF！！！