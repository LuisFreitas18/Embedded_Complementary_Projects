import os
import shutil
import platform
import time
from SCons.Script import Import

Import('env')

# Definição de Cores ANSI para o Terminal
RESET  = "\033[0m"
GREEN  = "\033[1;32m"
YELLOW = "\033[1;33m"
RED    = "\033[1;31m"
CYAN   = "\033[1;36m"

def find_pico_storage_rpirp2():
    sistema = platform.system()
    
    if sistema == "Windows":
        import ctypes
        kernel32 = ctypes.windll.kernel32
        volumeNameBuffer = ctypes.create_unicode_buffer(1024)
        
        for letra in range(65, 91): # ASCII para A-Z
            drive = f"{chr(letra)}:\\"
            if os.path.exists(drive):
                rc = kernel32.GetVolumeInformationW(
                    drive, volumeNameBuffer, ctypes.sizeof(volumeNameBuffer), 
                    None, None, None, None, 0
                )
                if rc and volumeNameBuffer.value == "RPI-RP2":
                    return drive
                    
    elif sistema == "Linux":
        base_media = f"/media/{os.getlogin()}/"
        if os.path.exists(base_media):
            for pasta in os.listdir(base_media):
                if "RPI-RP2" in pasta:
                    return os.path.join(base_media, pasta)
                    
    elif sistema == "Darwin": # macOS
        caminho_mac = "/Volumes/RPI-RP2"
        if os.path.exists(caminho_mac):
            return caminho_mac
            
    return None

def upload_to_pico(source, target, env):
    print(f"\n{CYAN}=================================================={RESET}")
    print(f"{CYAN} [SYSTEM] Deploying UF2 file... {RESET}")
    print(f"{CYAN}=================================================={RESET}")
    
    caminho_origem = os.path.join(".pio", "build", "pico", "firmware.uf2")
    
    # 1. Verifica se o ficheiro compilado existe
    if not os.path.exists(caminho_origem):
        print(f"{RED} [ERROR] Firmware not found at: {caminho_origem}{RESET}")
        print(f"{YELLOW} Ensure the build process completed successfully.{RESET}")
        return -1

    # 2. Tenta encontrar o Pi Pico USB
    print(f"{YELLOW} -> Searching for Raspberry Pi Pico (RPI-RP2)...{RESET}")
    caminho_pico = find_pico_storage_rpirp2()
    
    if not caminho_pico:
        print(f"{RED} [ERROR] Failed to find the Pi Pico mounted as USB!{RESET}")
        print(f"{YELLOW} Ensure you are in BOOTSEL mode (press the Pico's button while plugging in the cable).{RESET}")
        return -1
        
    print(f"{GREEN} -> Pico found successfully at: {caminho_pico}{RESET}")
    
    # 3. Copia o ficheiro .uf2 para o Pico
    try:
        destino_final = os.path.join(caminho_pico, "firmware.uf2")
        print(f"{YELLOW} -> Copying '{caminho_origem}' to '{destino_final}'...{RESET}")
        
        shutil.copyfile(caminho_origem, destino_final)
        
        print(f"\n{GREEN}=================================================={RESET}")
        print(f"{GREEN} UPLOAD SUCCESSFUL! {RESET}")
        print(f"{GREEN} Pi Pico will restart and run your program.{RESET}")
        print(f"{GREEN}==================================================\n{RESET}")
        return 0
        
    except Exception as e:
        print(f"{RED} [ERROR] Error copying file: {str(e)}{RESET}")
        return -1

# Configura como comando principal de upload para evitar erros com portas COM
def custom_upload_target(target, source, env):
    return upload_to_pico(source, target, env)

env.Replace(UPLOADCMD=custom_upload_target)