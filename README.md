# kfc

Krkr Fgimage Covertool

# 如何使用

首先你需要写`kfc_config.json`文件，然后把这个文件放在程序运行目录
接着把解包的立绘文件和立绘的配置文件（形如`tkan01_ll.pbd.json`文件和`tkan01.sinfo`）放到程序运行目录下（注意不要嵌套其他文件夹）
然后运行
如果文件目录下只有 pbd 文件，请使用 pbd2json

```shell
./kfc
```

接着你就会在 output 文件夹下看到合成的立绘

# 如何写 kfc_config

一个叫做 sinfo，为 sinfo 的文件名
一个叫做 config，为配置文件的文件名
一个叫做 base，为立绘差分图片的前缀
示例

```json
{
	"sinfo": "tkan01.sinfo",
	"config": "tkan01_ll.pbd.json",
	"base": "tkan01_ll_"
}
```

# 编译

```shell
g++ kfc.cpp -o kfc $(pkg-config --cflags --libs opencv jsoncpp)
```

# 特别注意

请确保所有文件的编码为 utf-8,并且换行为 LF！！！
