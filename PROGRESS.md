# 开发进度

最后更新：2026-04-21（断联检测 UI 控制 + 配置持久化）

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

### 指令预设 + 群发信息保留 ✅ 完成（2026-04-13）

- [x] 新建 `CmdPreset`（`src/core/CmdPreset.h`）：纯数据结构体，存储预设的 name/topic/payload/qos
- [x] 新建 `CmdPresetStore`（`src/core/CmdPresetStore.h / .cpp`）：JSON 持久化至 `%APPDATA%\MqttMonitor\cmd_presets.json`；`addPreset()` 以 topic 为基础命名，重名自动加 `-2/-3` 后缀；额外存储 lastTopic/lastPayload/lastQos 用于跨重启恢复
- [x] `DeviceView`：指令面板顶部新增预设行（下拉框 + 保存 + 删除）；构造时恢复 lastUsed 到输入框；选择预设自动填入 topic/payload/qos；字段变化实时写入 lastUsed

**涉及文件**
```
src/core/CmdPreset.h                ✅ 新建
src/core/CmdPresetStore.h / .cpp    ✅ 新建
src/ui/DeviceView.h / .cpp          ✅ 预设行 + lastUsed 恢复
CMakeLists.txt                      ✅ 新增源文件
```

---

### 设备断联检测 ✅ 完成（2026-04-21）

**设计要点**
- 断联判断：三重条件（30秒超时 / MQTT断连 / status=offline），任意一个触发即断联
- UI 样式：卡片红色边框 + 状态 badge 显示"断联"，恢复后还原断联前的状态（online/offline）
- 检测实现：懒惰轮询（1秒检查一次，仅检查 checkQueue_ 中的设备），支持大量设备
- 恢复逻辑：新消息到达立即恢复，无延迟确认
- MQTT 断连：杀掉 Broker 进程触发 `connection_lost` 回调，所有设备立即断联，状态栏提示

**实测验证（2026-04-21）**
- ✅ 30 秒无消息后设备自动显示断联（红边框 + badge "断联"）
- ✅ Kill Mosquitto 进程后所有设备立即断联，状态栏显示 "MQTT Connection Lost"
- ✅ 重新发送消息后卡片恢复断联前状态

**实施任务列表**

- [x] **Task #1** 定义断联状态数据结构和 DeviceView 基础改造
  - [x] 定义 DeviceState 结构体（deviceId / lastMessageTime / isDisconnected）
  - [x] 在 DeviceView 添加 `QMap<QString, DeviceState> deviceStates_`
  - [x] 在 DeviceView 添加 `QSet<QString> checkQueue_`（待检查队列）
  - [x] 更新 addCard() 方法初始化 DeviceState

- [x] **Task #2** 实现懒惰轮询定时器和检测逻辑
  - [x] 添加 `QTimer disconnectCheckTimer_`
  - [x] 实现 `checkDisconnectedDevices()` 槽（检查 checkQueue 中超时的设备）
  - [x] 实现 `markDeviceDisconnected()` 和 `markDeviceConnected()` 方法

- [x] **Task #3** 更新消息处理流程，集成超时检测
  - [x] 在 `updateDevice()` 中更新 lastMessageTime
  - [x] 在 `updateDevice()` 中处理设备恢复逻辑
  - [x] 在 `updateDevice()` 中决定是否加入 checkQueue

- [x] **Task #4** 处理 MQTT 连接断开事件
  - [x] 连接 MqttBridge::connectionLost 信号
  - [x] 在 `onMqttConnectionLost()` 中标记所有设备为断联

- [x] **Task #5** DeviceCard 添加断联状态支持和样式
  - [x] 添加 Q_PROPERTY(bool disconnected)
  - [x] 实现 `setDisconnected(bool)` 方法
  - [x] 实现 `isDisconnected()` 查询方法

- [x] **Task #6** 更新 dark.qss 样式表支持断联指示
  - [x] 添加 `DeviceCard[disconnected="true"]` 选择器
  - [x] 定义红色边框或灰化背景样式

- [x] **Task #7** 启动/停止轮询定时器的生命周期管理
  - [x] 在 MainWindow 中管理定时器启动/停止
  - [x] 在连接时启动，断开时停止

- [x] **Task #8** 处理设备删除时的状态清理
  - [x] 在 `removeDevice()` 中删除 deviceStates_[deviceId]
  - [x] 从 checkQueue_ 中移除 deviceId

- [x] **Task #9** 编译验证和集成测试
  - [x] 编译验证无错误
  - [x] 测试超时检测（30 秒无消息后显示断联）
  - [x] 测试 MQTT 断连（Kill Mosquitto 进程，所有设备立即断联）
  - [x] 测试恢复逻辑（新消息到达立即恢复）

**涉及文件预期**
```
src/ui/DeviceView.h / .cpp              ✅ 数据结构 + 轮询逻辑
src/ui/DeviceCard.h / .cpp              ✅ 断联状态支持
src/ui/dark.qss                         ✅ 断联样式
src/bridge/MqttBridge.h / .cpp          ✅ connectionLost 信号
mainwindow.h / .cpp                     ✅ 定时器生命周期
CMakeLists.txt                          ✅ 验证编译
```

---

### 断联检测 UI 控制 + 配置持久化 ✅ 完成（2026-04-21）

**设计要点**
- 启用/禁用开关（QCheckBox）：默认关闭，用户手动勾选启用
- 超时时间设置（QSpinBox）：范围 1-300 秒，默认 30 秒
- 配置持久化：保存到 `%APPDATA%\MqttMonitor\disconnect_detection.json`
- 启动行为：读取上次保存的配置状态，自动恢复
- 断开连接时：自动停止检测定时器，但保留配置状态
- 重新连接时：用户根据复选框状态手动启动（如果上次启用了）

**实施要点**
- [x] 新建 `DisconnectDetectionConfig` 类：管理 enabled 状态和 timeoutSeconds，JSON 持久化
- [x] 在 DeviceView 工具栏添加两个控件：
  - QCheckBox "启用断联检测"
  - QSpinBox 超时秒数（1-300，默认 30）
- [x] 修改 `checkDisconnectedDevices()` 方法：使用 `timeoutMs_` 成员变量替代硬编码的 30000
- [x] 移除 mainwindow.cpp 中的自动启动逻辑：
  - 移除连接时自动调用 `startDisconnectDetection()`
  - 保留断开时调用 `stopDisconnectDetection()`（断连后停止检测）
- [x] 构造函数自动启动：如果上次保存的配置为启用状态，自动启动检测

**操作流程**
1. 启动应用 → 读取配置 → 根据配置决定是否显示"启用断联检测"为勾选状态
2. 用户勾选 → 启动检测定时器 → 配置立即保存
3. 用户取消勾选 → 停止检测定时器 → 配置立即保存
4. 修改超时秒数 → 新值立即生效并保存

**涉及文件**
```
src/core/DisconnectDetectionConfig.h / .cpp     ✅ 新建
src/ui/DeviceView.h / .cpp                      ✅ UI 控件 + 槽函数
mainwindow.h / .cpp                             ✅ 移除自动启动逻辑
CMakeLists.txt                                  ✅ 新增 DisconnectDetectionConfig
```

---

### 待开发

- [ ] 日志模块
- [ ] 数据库模块
