# FaceSwap4UE
用于虚幻引擎的换脸服务

## 项目需求
1. 用户上传一张自己的照片
2. 场景截图，将截图出来的模特模型的脸换成用户上传的

## 运行平台
 - 引擎版本：5.3
 - 操作系统：Windows 10+
 - 显卡：N卡

## 文档
[项目文档](https://github.com/Circle1688/FaceSwap4UE/blob/main/Documents/FaceSwap4UE.md)


## TODO

### 后端
基于FaceFusion 项目地址：https://github.com/facefusion/facefusion
  - [ ] FaceFusion后端API服务改造（GPU服务器处理速度很快，不考虑并发问题）
  - [ ] 本地部署（虚拟环境 py3.10）未来可改服务器部署

### UE插件
- 以暴露`蓝图节点`形式交付
- 蓝图异步节点（Http）
  - [x] 发送处理请求
  - [x] 回调结果

- Utils 蓝图函数库
  - [x] 运行时截图（异步）
  - [x] 获取本地图片
  - [x] 保存图片到本地

- 后期服务器部署`未来`
   - [ ] 从账户获取图片（异步）`未来`
   - [ ] 保存图片到账户（异步）`未来`
   - [ ] 服务器部署（虚拟环境 py3.10）`未来`
