import time
import subprocess
import os

def get_cpu_temp():
    """讀取樹莓派 SoC 內建的溫度感測器檔案"""
    try:
        # 樹莓派核心溫度的虛擬檔案路徑
        with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
            temp_raw = f.read()
        # 數值為毫攝氏度 (e.g., 45123)，需除以 1000 轉為攝氏
        return float(temp_raw) / 1000.0
    except Exception as e:
        print(f"讀取溫度失敗: {e}")
        return 0.0

def update_oled(temp):
    """呼叫編譯好的 C 工具將溫度印在 OLED 上"""
    display_text = f"TEMP: {temp:.1f} C"
    
    # 執行指令: sudo ./tools/ssd1306_writer "TEMP: 45.2 C"
    cmd = ["sudo", "../tools/ssd1306_writer", display_text]
    
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"呼叫 C 工具失敗: {e}")

if __name__ == "__main__":
    print("OLED CPU 溫度監控中... (按 Ctrl+C 停止)")
    try:
        while True:
            current_temp = get_cpu_temp()
            print(f"目前核心溫度: {current_temp:.1f} °C")
            
            # 更新顯示
            update_oled(current_temp)
            
            # 每 5 秒更新一次
            time.sleep(5)
    except KeyboardInterrupt:
        print("\n已停止監控。")
