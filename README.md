# kfc
 Krkr Fgimage Covertool

# 如何使用
首先你需要写`kfc_config.json`文件，然后把这个文件放在程序运行目录
接着把解包的立绘文件和立绘的配置文件（形如`氷織Ａ_0.txt`文件和`氷織Ａ_info.txt`）放到程序运行目录下（注意不要嵌套其他文件夹）
然后运行 
```shell
kfc 氷織Ａ_0.txt #改成你要合成的立绘的配置文件
```
接着你就会在output文件夹下看到合成的立绘

# 如何写kfc_config

创建两个数组，一个叫做base，用作基底
一个叫做line，你需要手动查看`氷織Ａ_info.txt`找到第二个`fgalias`在第几行
示例
```json
{
    "base": [14, 25, 45, 58, 2934],
    "line": 67
}
```


# 编译

```shell
g++ kfc.cpp -o kfc $(pkg-config --cflags --libs opencv jsoncpp)
```