import time
import subprocess   
import os

def get_cpu_temp():
    """讀取樹莓派 SoC 內建的溫度感測器檔案"""
    # Exception Handling
    try:
        # 讀linux 的標準溫度介面
        with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
            temp_raw = f.read()
        # 毫攝氏轉攝氏（/1000.0）
        return float(temp_raw) / 1000.0
    except Exception as e:
        print(f"讀取溫度失敗: {e}")
        return 0.0

def update_oled(temp):
    """呼叫編譯好的 C 工具將溫度印在 OLED 上"""
    display_text = f"TEMP: {temp:.1f} C"
    
    # 執行指令: sudo ./tools/ssd1306_writer "TEMP: 45.2 C"
    # display_text -> 將字串作為 argv[1] 參數傳進去
    cmd = ["sudo", "../tools/ssd1306_writer", display_text]
    
    try:
        # 執行外部指令 （Python 與 C 溝通的橋樑），check 嚴格模式，程式失敗報錯（CalledProcessError）
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"呼叫 C 工具失敗: {e}")

# 檢查直接執行 or 被 import 進去（執行進腳本才進監控的無窮迴圈）
if __name__ == "__main__":
    print("OLED CPU 溫度監控中... ")
    try:
        while True:
            current_temp = get_cpu_temp()
            print(f"目前核心溫度: {current_temp:.1f} °C")
            
            update_oled(current_temp)
            
            time.sleep(5)
    # ctrl + c 的終止訊號
    except KeyboardInterrupt:
        print("\n已停止監控。")
