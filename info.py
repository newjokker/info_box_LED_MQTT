import paho.mqtt.client as mqtt

# MQTT配置
broker = "8.153.160.138"  # MQTT服务器地址
port = 1883               # MQTT端口
topic = "info_box/lvdi/box_001"  # 要发布的主题

# 创建MQTT客户端
client = mqtt.Client()

# 连接到MQTT服务器
client.connect(broker, port, 60)

# 发送消息到LCD屏幕
# 格式: "第一行内容|第二行内容"
message = "Hello World! | just do it"

# 发布消息
client.publish(topic, message)

print(f"已发送消息到 {topic}:")
print(f"  '{message}'")

# 断开连接
client.disconnect()