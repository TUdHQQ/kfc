# kfc
 Krkr Fgimage Covertool

# 如何使用
首先你需要写`kfc_config.json`文件，然后把这个文件放在程序运行目录
接着把解包的立绘文件和立绘的配置文件（形如`ヤエカＡ_0.json`文件）放到程序运行目录下（注意不要嵌套其他文件夹）
然后运行 
```shell
kfc ヤエカＡ_0.json #改成你要合成的立绘的配置文件
```
接着你就会在output文件夹下看到合成的立绘

# 如何写kfc_config

创建两个数组，一个叫做base，用作基底
一个叫做face，用来合成表情
示例
```json
{
    "base": [3153, 3474, 3708, 3758],
    "face": [4154, 4176, 4191, 4216, 4241, 4266, 4339, 4371, 4403, 4428]
}
```


# 编译

```shell
g++ kfc.cpp -o kfc.exe $(pkg-config --cflags --libs opencv jsoncpp)
```