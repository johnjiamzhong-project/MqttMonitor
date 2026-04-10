# 开发进度

最后更新：2026-04-10

## Phase 1 — 核心连通 ✅ 完成

### 已完成
- [x] CMakeLists 集成 Paho MQTT C++
- [x] MqttClient 封装（连接、订阅、发布）
- [x] MessageBuffer 线程安全消息队列
- [x] CMakeLists.txt 更新（添加 src/core/ 源文件）
- [x] 编译验证通过（Ninja + MSVC + Qt MOC/UIC 正常运行）
- [x] CMakePresets.json 替代 configure.bat / build.bat（VS Code CMake Tools 集成）

### 创建的文件
```
src/
├── core/
│   ├── MqttClient.h / .cpp       ✅ 完成
│   └── MessageBuffer.h / .cpp    ✅ 完成
├── bridge/
│   └── (待实现)
└── ui/
    └── (待迁移 mainwindow.* + main.cpp)
CMakePresets.json                 ✅ 完成（替代 build/*.bat 脚本）
```

### 说明
- 构建工具链：CMake → Ninja → Qt MOC/UIC + MSVC cl.exe
- vcpkg 管理依赖：Qt5 Widgets、PahoMqttCpp（x64-windows triplet）
- 已加 `PAHO_MQTTPP_IMPORTS` 解决 MSVC dllimport 链接问题

---

## Phase 2 — 基础 UI ✅ 完成

最后更新：2026-04-10

### 已完成
- [x] MqttBridge（Paho 回调 → Qt 信号跨线程转发）
- [x] ConfigPanel（连接配置面板，含输入校验，基于 .ui 文件）
- [x] MainWindow 接入（QSplitter 布局，整合 ConfigPanel + MqttBridge，打通连接流程）
- [x] 修复关键 Bug：MqttClient 继承 mqtt::callback 并注册 set_callback(*this)，消息回调正常触发
- [x] 抑制 Paho 头文件引起的 MSVC C4819 编码警告
- [x] 消息展示面板：右侧 QPlainTextEdit（只读，最多 500 条，自动滚动）
- [x] 消息格式：`[hh:mm:ss.zzz]  topic` + 换行 + `    payload`，payload 自动压缩为单行
- [x] 布局调整：container + QHBoxLayout 包裹 splitter，右侧留边距，splitter handle 加宽
- [x] 添加 `/utf-8` MSVC 编译选项，解决中文 Windows 下 UTF-8 源文件解析乱码问题

### 创建/修改的文件
```
src/
├── core/
│   ├── MqttClient.h / .cpp    ✅ 修复回调注册 bug
│   └── MessageBuffer.h / .cpp ✅ 完成
├── bridge/
│   └── MqttBridge.h / .cpp    ✅ 完成
└── ui/
    ├── ConfigPanel.h / .cpp   ✅ 完成
    └── ConfigPanel.ui         ✅ 完成
mainwindow.h / .cpp            ✅ 消息展示接入，QPlainTextEdit 替换占位 QWidget
CMakeLists.txt                 ✅ 添加 /utf-8 /wd4819 编译选项
```

### 待做
- [ ] DeviceGridView（设备卡片网格，替换右侧消息列表）
- [ ] JSON 解析与结构化展示

---

## Phase 3 — 设备管理 ⏳ 待开始

- [ ] 手动添加/删除设备
- [ ] 自动发现（监听通配符 Topic）
- [ ] 批量指令群发

---

## Phase 4 — 扩展模块 ⏳ 后续迭代

- [ ] 日志模块
- [ ] 数据库模块 + 多配置切换
