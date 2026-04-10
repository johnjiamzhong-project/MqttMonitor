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
| UI 框架 | Qt 5（Widgets） |
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

### Phase 1 — 核心连通
- [ ] CMakeLists.txt 集成 Paho MQTT C++
- [ ] MqttClient 封装：连接、订阅、发布
- [ ] 控制台验证消息收发

### Phase 2 — 基础 UI
- [ ] 配置页：Broker 参数填写与保存
- [ ] 设备页：卡片网格布局
- [ ] Bridge 层：Paho 回调 → Qt 信号跨线程安全传递
- [ ] 消息展示：原始 Payload + JSON 解析

### Phase 3 — 设备管理
- [ ] 手动添加/删除设备
- [ ] 自动发现：监听通配符 Topic，自动生成卡片
- [ ] 多选设备 + 指令群发

### Phase 4 — 扩展模块（后续迭代）
- [ ] 日志模块：消息历史记录到本地文件
- [ ] 数据库模块：SQLite 持久化 + 历史消息查询
- [ ] 多套 Broker 配置切换

---

## 目录结构（规划）

```
MqttMonitor/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── core/
│   │   ├── MqttClient.h / .cpp        # Paho 封装
│   │   └── MessageBuffer.h / .cpp     # 线程安全消息队列
│   ├── bridge/
│   │   └── MqttBridge.h / .cpp        # Paho 回调 → Qt 信号
│   └── ui/
│       ├── MainWindow.h / .cpp
│       ├── ConfigPanel.h / .cpp
│       ├── DeviceGridView.h / .cpp
│       ├── DeviceCard.h / .cpp
│       └── DeviceDetailView.h / .cpp
└── resources/
    └── icons/
```

---

## 关于 Topic 格式

MqttMonitor **不强制约定 Topic 结构**。

用户可以自由配置订阅规则，程序展示原始消息。若需要自动发现设备，建议（非强制）采用如下格式，便于从 Topic 中解析出设备标识：

```
{任意前缀}/{device_id}/{任意后缀}
```

例如：`factory/line1/temp`、`devices/sensor_01/data` 均可正常工作。
