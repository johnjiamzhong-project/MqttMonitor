# MqttMonitor

基于 Qt + Paho MQTT C++ 的通用 MQTT 设备监控客户端。

---

## 项目定位

MqttMonitor 是一个运行在 Windows 桌面端的 MQTT 客户端上位机，面向需要监控多台 IoT 设备的调试/运维人员。

它不绑定任何特定设备类型或 Topic 格式，用户可以自由配置订阅规则，程序负责收集消息、展示状态、下发指令。

---

## 核心功能

### 配置管理
- 填写 Broker 地址、端口、用户名/密码、ClientID、KeepAlive 等连接参数
- 配置全局订阅 Topic（支持通配符，如 `devices/#`）
- 配置持久化，支持多套环境切换（如测试环境 / 生产环境）

### 设备管理
设备有两种来源：

| 来源 | 说明 |
|------|------|
| 手动添加 | 用户填写设备名称及其关注的 Topic 列表 |
| 自动发现 | 程序监听全局订阅 Topic，有新消息来源时自动创建设备卡片 |

自动发现的设备以消息来源 Topic 作为初始名称，用户可后续重命名。

### 设备卡片视图
右侧主区域以卡片网格展示所有设备，每张卡片显示：
- 设备名称
- 在线状态（在线 🟢 / 离线 🔴 / 未知 🟡）
- 最后一条消息的首个字段预览
- 最后消息时间戳

### 消息查看与通信
- 点击单个设备卡片 → 查看该设备的消息详情（原始 Payload + JSON 自动解析展开）
- 多选设备 → 向所选设备批量下发指令（群发）
- 指令下发：选择目标 Topic、填写 Payload、选择 QoS，发送

### 消息展示策略
- **原始层**：始终保留原始 Payload，便于调试
- **解析层**：若 Payload 为合法 JSON，自动展开为键值对
- 非 JSON 格式（纯文本、十六进制）同样支持展示，不强制约定格式

---

## 界面结构

```
┌──────────┬──────────────────────────────────────────┐
│          │                                          │
│  配置     │   [设备A 🟢]  [设备B 🔴]  [设备C 🟢]     │
│          │                                          │
│          │   [设备D 🟢]  [设备E 🟡]  [未知设备 🟢]   │
│  设备     │                                          │
│          │   [设备G 🔴]  [设备H 🟢]  [...]           │
│          │                                          │
└──────────┴──────────────────────────────────────────┘
```

左侧导航：配置 / 设备两个页面入口  
右侧内容区：当前页面的主体内容

---

## 技术栈

| 层 | 技术 |
|----|------|
| UI 框架 | Qt 5（Widgets，Fusion 风格 + 自定义 QSS 暗黑主题） |
| MQTT 通信 | Eclipse Paho MQTT C++ |
| 构建系统 | CMake + vcpkg |
| 平台 | Windows 11 |

---

## 架构分层

```
UI 层
  MainWindow / ConfigPanel / DeviceGridView / DeviceDetailView
       ↕ Qt 信号槽（QueuedConnection）
Bridge 层
  MqttBridge（QObject，负责线程安全的消息转发）
       ↕ 回调注册
Core 层
  MqttClient（封装 Paho，管理连接/订阅/发布）
  MessageBuffer（线程安全消息队列，批量推送给 UI）
```

Paho 的消息回调运行在子线程，Bridge 层通过 `Qt::QueuedConnection` 将消息安全投递至主线程，避免直接操作 UI。

---

## 开发阶段规划

### Phase 1 — 核心连通 ✅ 完成
- [x] CMakeLists.txt 集成 Paho MQTT C++
- [x] MqttClient 封装：连接、订阅、发布
- [x] MessageBuffer 线程安全消息队列
- [x] CMakePresets.json（VS Code CMake Tools 集成）

### Phase 2 — 基础 UI ✅ 完成
- [x] ConfigPanel：Broker 参数填写，内置 QSplitter（左配置表单 + 右消息列表）
- [x] MqttBridge：Paho 回调 → Qt 信号跨线程安全传递
- [x] MainWindow：navBar（配置/设备）+ QStackedWidget 双页结构
- [x] DeviceView 占位类（待 Phase 3 实现）

### Phase 3 — 设备管理 ✅ 完成
- [x] 设备消息 JSON 结构确定（device_id / name / status / timestamp / data）
- [x] DeviceView 实现（QScrollArea + QGridLayout 卡片网格）
- [x] DeviceCard 类（展示 name / status / data KV）
- [x] JSON 解析（Qt5 QJsonDocument）
- [x] 手动添加/删除设备（对话框填写 ID + 名称；右键菜单删除）
- [x] 自动发现：收到新 device_id 消息时自动生成卡片（配合通配符订阅如 `devices/#`）
- [x] 多选设备 + 指令群发（左键点击卡片选中/取消；底部面板填写 Topic / Payload / QoS 批量发送；Topic 支持 `{device_id}` 占位符）

### UI 风格优化 ✅ 完成
- [x] Catppuccin Mocha 暗黑主题（全局 QSS）
- [x] Win11 原生标题栏暗色（DWM API）
- [x] 导航按钮互斥高亮

### Phase 4 — 扩展模块
- [x] 多套 Broker 配置切换：命名配置存档，启动自动恢复，JSON 持久化至 `%APPDATA%\MqttMonitor\profiles.json`
- [ ] 日志模块：消息历史记录到本地文件
- [ ] 数据库模块：SQLite 持久化 + 历史消息查询

---

## 目录结构

```
MqttMonitor/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── PROGRESS.md
├── main.cpp
├── mainwindow.h / .cpp / .ui
└── src/
    ├── core/
    │   ├── MqttClient.h / .cpp        # Paho 封装
    │   └── MessageBuffer.h / .cpp     # 线程安全消息队列
    ├── bridge/
    │   └── MqttBridge.h / .cpp        # Paho 回调 → Qt 信号
    └── ui/
        ├── ConfigPanel.h / .cpp / .ui  # 配置页（含消息列表）
        ├── DeviceView.h / .cpp         # 设备页（卡片网格 + 指令面板）
        ├── DeviceCard.h / .cpp         # 设备卡片（状态展示 + 多选）
        └── AddDeviceDialog.h / .cpp    # 手动添加设备对话框
```

---

## 消息格式约定

### Topic 格式

```
devices/<device_id>/status
```

`device_id` 同时冗余在 payload JSON 中，便于解析。

### Payload JSON 结构

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
| `name` | string | 可读名称，卡片标题显示 |
| `status` | string | `online` / `offline` / `error` |
| `timestamp` | number | Unix 时间戳（秒） |
| `data` | object | 业务 KV，value 为 number 或 string |

- 设备离线时主动发送 `status: offline` 消息，不依赖心跳超时
- `data` 字段结构自由，程序以 KV 列表形式展示在卡片下方

---

## 调试工具

### MQTTX 定时发送脚本

在 MQTTX 的定时发送功能中使用以下脚本，可模拟设备持续上报温湿度数据：

```javascript
function handlePayload(value) {
  let msg = typeof value === 'string' ? JSON.parse(value) : value;

  msg.timestamp = Date.now();
  msg.data = {
    temperature: +(20 + Math.random() * 15).toFixed(1), // 模拟 20-35 度
    humidity: Math.floor(40 + Math.random() * 30)       // 模拟 40-70% 湿度
  };

  return JSON.stringify(msg, null, 2);
}

execute(handlePayload);
```

**使用方式：**
1. 在 MQTTX 新建连接，Topic 填写 `<device_id>/status`
2. Payload 填写包含完整字段的初始 JSON（`device_id`、`name`、`status` 等固定字段在此填写）
3. 开启定时发送，选择上方脚本，脚本会自动覆盖 `timestamp` 和 `data` 字段
