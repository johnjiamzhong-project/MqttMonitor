# 开发进度

最后更新：2026-04-13

---

## Phase 1 — 核心连通 ✅ 完成

- [x] CMakeLists 集成 Paho MQTT C++
- [x] MqttClient 封装（连接、订阅、发布）
- [x] MessageBuffer 线程安全消息队列
- [x] CMakePresets.json 替代 configure.bat / build.bat（VS Code CMake Tools 集成）
- [x] 编译验证通过（Ninja + MSVC + Qt MOC/UIC 正常运行）

**说明**
- 构建工具链：CMake → Ninja → Qt MOC/UIC + MSVC cl.exe
- vcpkg 管理依赖：Qt5 Widgets、PahoMqttCpp（x64-windows triplet）
- 已加 `PAHO_MQTTPP_IMPORTS` 解决 MSVC dllimport 链接问题

---

## Phase 2 — 基础 UI ✅ 完成

- [x] MqttBridge（Paho 回调 → Qt 信号跨线程转发）
- [x] ConfigPanel（连接配置面板，含输入校验，基于 .ui 文件）
- [x] ConfigPanel 内部布局：QSplitter（左 300px 配置表单 + 右 QPlainTextEdit 消息列表）
- [x] ConfigPanel 新增 `appendMessage(topic, payload)` public slot
- [x] 消息格式：`[hh:mm:ss.zzz]  topic` + 换行 + payload（simplified 压缩为单行）
- [x] MainWindow 主框架：navBar（配置/设备按钮，72px）+ QStackedWidget
- [x] MqttBridge 信号直连 ConfigPanel::appendMessage（移除 MainWindow 中转）
- [x] DeviceView 占位类（QStackedWidget index 1）
- [x] MSVC `/utf-8 /wd4819` 编译选项

**涉及文件**
```
src/bridge/MqttBridge.h / .cpp        ✅
src/ui/ConfigPanel.h / .cpp / .ui     ✅ QSplitter 重构
src/ui/DeviceView.h / .cpp            ✅ 占位
mainwindow.h / .cpp                   ✅ navBar + QStackedWidget
CMakeLists.txt                        ✅
```

---

## Phase 3 — 设备管理 ✅ 完成

### 消息 JSON 结构（已确定）

```json
{
  "device_id":  "device001",
  "name":       "温控器-1号",
  "status":     "online",
  "timestamp":  1712750000,
  "data": {
    "temperature": 25.3,
    "humidity":    60
  }
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `device_id` | string | 设备唯一标识 |
| `name` | string | 可读名称（卡片标题） |
| `status` | string | `online` / `offline` / `error` |
| `timestamp` | number | Unix 时间戳（秒） |
| `data` | object | 业务 KV，value 为 number 或 string |

- Topic 格式：`devices/<device_id>/status`
- 离线方式：设备主动发 `status: offline`，不依赖心跳超时

### 实施计划

- [x] **Step 1** 新建 `DeviceCard`（`src/ui/DeviceCard.h / .cpp`）
  - QFrame 固定尺寸
  - 顶部：设备名（粗体）+ 状态色块 Label（在线绿 / 离线红 / 未知黄）
  - 下方：`data` KV 用 QFormLayout 逐行展示
  - 公开方法：`void update(const QJsonObject& obj)`

- [x] **Step 2** 改造 `DeviceView`（`src/ui/DeviceView.h / .cpp`）
  - 替换占位 Label，改为 QScrollArea + QGridLayout（每行 3 列）
  - 新增 `updateDevice(const QString& topic, const QString& payload)` public slot
    - 解析 JSON（QJsonDocument）
    - 按 `device_id` 查找已有 DeviceCard → 调用 update()，或新建并插入网格

- [x] **Step 3** `mainwindow.cpp` 加一条 connect
  - `onConnectRequested` 中，mqttBridge_ 信号额外连接到 `deviceView_->updateDevice`

- [x] **Step 4** `CMakeLists.txt` 添加 `DeviceCard.h / .cpp`

- [x] **Step 5** 手动添加/删除设备
  - 新建 `AddDeviceDialog`（`src/ui/AddDeviceDialog.h / .cpp`）：填写设备 ID + 名称
  - DeviceView 顶部工具栏：`+ 添加设备` 按钮
  - DeviceCard 右键菜单：`删除设备`，触发 `removeRequested` 信号

- [x] **Step 6** 自动发现
  - 收到新 `device_id` 的消息时自动生成卡片（配合通配符订阅如 `devices/#`）

- [x] **Step 7** 多选设备 + 批量指令群发
  - DeviceCard 左键点击切换选中状态（蓝色边框高亮），发出 `selectionChanged` 信号
  - DeviceView 底部指令面板（选中后显示）：Topic / Payload / QoS 输入 + 发送按钮
  - Topic 支持 `{device_id}` 占位符，批量发送时自动替换为各设备 ID
  - MainWindow 新增 `onPublishRequested` 槽，调用 `MqttClient::publish()`

**涉及文件**
```
src/ui/DeviceCard.h / .cpp          ✅ 含选中状态、右键菜单
src/ui/DeviceView.h / .cpp          ✅ 卡片网格 + 底部指令面板
src/ui/AddDeviceDialog.h / .cpp     ✅ 新建
mainwindow.h / .cpp                 ✅ onPublishRequested 槽
CMakeLists.txt                      ✅ 新增 AddDeviceDialog 源文件
```

---

## UI 风格优化 ✅ 完成（2026-04-11）

- [x] 新建 `src/ui/dark.qss`：Catppuccin Mocha 暗黑主题，覆盖所有控件（Button / LineEdit / PlainTextEdit / ComboBox / GroupBox / ScrollBar / StatusBar 等）
- [x] 新建 `resources.qrc`，将 QSS 嵌入可执行文件
- [x] `main.cpp`：设置 Fusion 风格基底 + 从资源加载全局样式表；窗口初始尺寸改为 1100×680
- [x] `mainwindow.cpp`：navBar 设 `objectName("navBar")` 供 QSS 精确定位；导航按钮改为 `setCheckable` + `QButtonGroup` 互斥，切换页面时自动高亮当前项；调用 `DwmSetWindowAttribute(DWMWA_USE_IMMERSIVE_DARK_MODE)` 使 Win11 原生标题栏同步变黑
- [x] `DeviceCard.cpp`：设 `objectName("DeviceCard")`，选中边框改为 `#89b4fa`（蓝），非选中状态回退到全局 QSS；状态 badge 颜色与暗色背景对比度调整（在线绿 / 离线红 / 未知黄）
- [x] `ConfigPanel.ui`：错误提示 Label 颜色改为 `#f38ba8`（玫瑰红，适配暗色背景）
- [x] `CMakeLists.txt`：添加 `resources.qrc`；MSVC 分支链接 `dwmapi`

**涉及文件**
```
src/ui/dark.qss                 ✅ 新建
resources.qrc                   ✅ 新建
main.cpp                        ✅ Fusion + QSS 加载
mainwindow.cpp                  ✅ navBar / 互斥按钮 / DWM 暗色标题栏
src/ui/DeviceCard.cpp           ✅ objectName + 选中/状态样式
src/ui/ConfigPanel.ui           ✅ 错误标签颜色
CMakeLists.txt                  ✅ resources.qrc + dwmapi
```

---

## Phase 4 — 扩展模块

### 多套配置持久化 ✅ 完成（2026-04-12）

- [x] 新建 `ConfigStore`（`src/core/ConfigStore.h / .cpp`）：JSON 读写，Profile 增删改查，存储路径 `%APPDATA%\MqttMonitor\profiles.json`
- [x] `ConfigPanel.ui`：GroupBox 顶部新增配置选择下拉框（profileCombo）+ 保存/删除按钮
- [x] `ConfigPanel.h / .cpp`：集成 ConfigStore，实现配置切换/保存/删除逻辑；连接时锁定下拉框防误操作；启动时自动恢复上次使用的配置
- [x] `CMakeLists.txt`：添加 ConfigStore.h / .cpp

**涉及文件**
```
src/core/ConfigStore.h / .cpp       ✅ 新建
src/ui/ConfigPanel.ui               ✅ 顶部加配置选择行
src/ui/ConfigPanel.h / .cpp         ✅ 集成 ConfigStore
CMakeLists.txt                      ✅ 新增源文件
```

---

### 卡片规则（Card Rule）✅ 完成（2026-04-13）

- [x] 新建 `CardRuleConfig`（`src/core/CardRuleConfig.h`）：纯数据结构体，配置 `device_id`/`name`/`status`/`data` 字段名及在线/离线状态值，默认值与原硬编码一致
- [x] 新建 `CardRuleStore`（`src/core/CardRuleStore.h / .cpp`）：仿 ConfigStore 模式，JSON 持久化至 `%APPDATA%\MqttMonitor\card_rules.json`，temp-file 原子写入，load 容错
- [x] 新建 `CardRuleDialog`（`src/ui/CardRuleDialog.h / .cpp`）：模态 QDialog，QFormLayout 6 行（4 个字段名 + 在线值 + 离线值），底部提示修改仅对新消息生效
- [x] `DeviceView`：工具栏新增"卡片规则"按钮；构造时 load 配置；`updateDevice()` 加翻译层，用配置字段名读 JSON 并将状态值归一化为内部固定 key 再传给 DeviceCard；DeviceCard 本身不改动
- [x] `CMakeLists.txt`：添加 5 个新文件

**涉及文件**
```
src/core/CardRuleConfig.h               ✅ 新建
src/core/CardRuleStore.h / .cpp         ✅ 新建
src/ui/CardRuleDialog.h / .cpp          ✅ 新建
src/ui/DeviceView.h / .cpp              ✅ 翻译层 + 按钮
CMakeLists.txt                          ✅ 新增源文件
```

---

### 待开发

- [ ] 日志模块
- [ ] 数据库模块
