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

## Phase 2 — 基础 UI 🔄 进行中

最后更新：2026-04-10

### 已完成
- [x] MqttBridge（Paho 回调 → Qt 信号跨线程转发）
- [x] ConfigPanel（连接配置面板，含输入校验）

### 创建的文件
```
src/
├── bridge/
│   ├── MqttBridge.h / .cpp    ✅ 完成
└── ui/
    ├── ConfigPanel.h / .cpp   ✅ 完成
    └── ConfigPanel.ui         ✅ 完成
```

### 待做
- [ ] MainWindow 接入（组装 ConfigPanel + MqttBridge，打通连接流程）
- [ ] DeviceGridView（设备卡片网格）
- [ ] 消息展示与 JSON 解析

---

## Phase 3 — 设备管理 ⏳ 待开始

- [ ] 手动添加/删除设备
- [ ] 自动发现（监听通配符 Topic）
- [ ] 批量指令群发

---

## Phase 4 — 扩展模块 ⏳ 后续迭代

- [ ] 日志模块
- [ ] 数据库模块 + 多配置切换
