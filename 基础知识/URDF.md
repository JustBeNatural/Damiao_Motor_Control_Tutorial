# URDF

URDF可视化网站：https://viewer\.robotsfan\.com/

![image-20260608104748192](D:\桌面\实习就业\北京华晟经世\工作\机械臂应用小项目\基础知识\URDF.assets\image-20260608104748192.png)

# 1\.简要介绍

URDF的全称是**Unified Robot Description Format（统一机器人描述格式）**

机械臂由多个刚体组成，每个关节都有运动关系。

URDF 是一种 **XML （Extensible Markup Language）文件**，它有标签和嵌套关系。最基本的 URDF 结构有两个核心概念：**joint**和**link**

**link** 是机械臂中的实体部件，例如底座、手臂段、末端夹爪。

一个 link 的最小例子：

![image-20260608104804834](D:\桌面\实习就业\北京华晟经世\工作\机械臂应用小项目\基础知识\URDF.assets\image-20260608104804834.png)

![image-20260608104816396](D:\桌面\实习就业\北京华晟经世\工作\机械臂应用小项目\基础知识\URDF.assets\image-20260608104816396.png)

# 2\.代码模块

一般用URDF都不会自己编写，会用SolidWorks自动生成URDF文件，或者在网上找别人写好的开源URDF文件，修修改改成自己的文件。

虽然不用自己写URDF文件，但是URDF文件语法是需要知道的，很多时候AI也不能很准确开发者的意思（需要增强Prompt准确性），就需要开发者自己手动修改URDF来满足需求了。

在URDF中，机械臂主要靠三个东西组成，其中两个比较重要，一个是link，一个是joint，（robot也重要但是比较简单）。语法也很简单，机器人上有一个joint就写一个：

`<joint>`

`..........`

`..........`

`</joint>`

这就代表一个joint模块。

同样的：

`<link>`

`..........`

`..........`

`</link>`

就代表一个link模块。

![1280X1280](D:\桌面\实习就业\北京华晟经世\工作\机械臂应用小项目\基础知识\URDF.assets\1280X1280.PNG)

还有其他的\<inertial\>,\<visual\>,\<collision\>就是\<link\>和\<joint\>下面的子模块，这些模块的作用也就是字面意思。





