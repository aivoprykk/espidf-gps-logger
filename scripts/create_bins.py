import os
import shutil
import argparse


class BuildConfig:
    def __init__(self):
        self.current_selection = ""
        self.current_flashsize = ""
        self.current_target = "esp32"
        self.project_name = "espidf-gps-logger"

        self.current_directory = os.getcwd()
        xpath = self.current_directory
        while not os.path.exists(os.path.join(xpath, "CMakeLists.txt")):
            parent = os.path.dirname(xpath)
            if parent == xpath:
                break
            if parent == "/":
                print("CMakeLists.txt not found in any parent directory.")
                exit()
            xpath = parent
        self.output_path = os.path.join(xpath, "build")


def copy_file(source_path, destination_path):
    try:
        shutil.copy(source_path, destination_path)
    except FileNotFoundError:
        print("File not found, please check the file path.")
    except PermissionError:
        print("No permission to access file, please check file permissions.")
    except Exception as e:
        print(f"An error occurred: {e}")

def show_menu():
    print("0.  all boards (build all configurations)")
    print("1.  T5 v2.13     (esp32 4M Flash) DEPG0213BN 2.13 122x250 Eink (Default)")
    print("2.  T5 v2.13     (esp32 4M Flash) GDEY0213B74 2.13 122x250 Eink")
    print("3.  T5 v2.13     (esp32 4M Flash) SSSD1681 1.54 200x200 Eink")
    print("4.  T5 v2.13     (esp32 16M Flash) DEPG0213BN 2.13 122x250 Eink")
    print("5.  T5 v2.13     (esp32 16M Flash) GDEY0213B74 2.13 122x250 Eink")
    print("6.  T5 v2.13     (esp32 16M Flash) SSSD1681 1.54 200x200 Eink")
    print("7.  T-Display-S3 (esp32s3 16M Flash) ST7789V 1.14 135x240 LCD")


def get_user_choice():
    while True:
        try:
            choice = int(
                input(
                    "Please enter the board number that needs to be compiled (1-7): "
                )
            )
            if 0 <= choice <= 7:
                return choice
            else:
                print("Invalid input, please enter a number between 0 and 7.")
        except ValueError:
            print("Invalid input, please enter a valid number.")

def perform_selection(self, choice):
    print(f"Performing selection for choice {choice}")
    self.current_target = "esp32"
    if choice >= 4 and choice < 7:
        self.current_flashsize = "16m"
    else:
        self.current_flashsize = ""
    if choice == 2:
        self.current_selection ="gdey0213b74"
    elif choice == 3:
        self.current_selection ="ssd1681"
    elif choice == 5:
        self.current_selection ="gdey0213b74"
    elif choice == 6:
        self.current_selection ="ssd1681"
    elif choice == 7:
        self.current_target ="esp32s3"
        self.current_selection ="st7789"
    else:
        self.current_selection = ""

def clean_build(self):
    print ("Clean build files")
    # idf_component_path = os.path.join(self.current_directory, "managed_components")
    # if os.path.isfile(idf_component_path):
    #     os.remove(idf_component_path)
    sdkconfig_path = os.path.join(self.current_directory, "sdkconfig")
    if os.path.isfile(sdkconfig_path):
        os.remove(sdkconfig_path)
    if os.path.exists(self.output_path):
        os.system("rm -rf " + self.output_path)

def perform_action(self):
    r = os.system("idf.py > " + os.devnull)
    if r != 0:
        print(
            "Unable to execute idf.py, please see here to learn how to use and install https://github.com/espressif/esp-idf"
        )
        exit()
    clean_build(self)
    print(f"Performing action")
    # os.system("git submodule init")
    # os.system("git submodule update")
    # os.system('idf.py add-dependency "lvgl/lvgl^8.3.11"')
    toname = "sdkconfig.defaults." + self.current_target
    if self.current_selection != "":
        toname += "-" + self.current_selection
    else:
        toname += "-default"
    if self.current_flashsize != "":
        toname += "-" + self.current_flashsize
    print(f"Using config file {toname}")
    tofile = os.path.join(self.current_directory, toname)
    if not os.path.isfile(tofile):
        print(f"Configuration file {tofile} not found!")
        exit()
    copy_file(tofile, "sdkconfig.defaults")
    print(f"Building {self.current_selection} for target {self.current_target}")
    os.system("idf.py set-target " + self.current_target)
    os.system("idf.py reconfigure")
    os.system("idf.py build size 2>&1 | tee build.log")

def cp_result(self):
    print(f"Copy build result for {self.current_selection} for target {self.current_target}")
    version_file = os.path.join(self.current_directory, "version.txt")
    if os.path.isfile(version_file):
        with open(version_file, "r") as vf:
            version = vf.read().strip()
    if version != "":
        version_packed = version.replace(".", "").replace("dev", ".dev")
        dest_dir = os.path.join(self.current_directory, "builds", version_packed)
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        frompath = os.path.join(self.output_path, self.project_name + "-" + version_packed + ".bin")
        toname =  self.project_name + "-" + version_packed;
        if self.current_selection != "":
            toname += "-" + self.current_selection
        if self.current_flashsize != "":
            toname += "-" + self.current_flashsize
        topath = os.path.join(dest_dir, toname + ".bin")
        print(f"Copying from {frompath} to {topath}")
        copy_file(frompath, topath)

def main():
    self = BuildConfig()
    user_choice = 0
    parser = argparse.ArgumentParser(description="ESP-IDF Project Setup")
    parser.add_argument("board", nargs="?", help="Board number (1-7)")
    args = parser.parse_args()

    if args.board:
        if args.board == "all":
            user_choice = 0
        else:
            try:
                board_num = int(args.board)
                if 0 <= board_num <= 7:
                    user_choice = board_num
                else:
                    show_menu()
                    user_choice = get_user_choice()
            except ValueError:
                print("Invalid input, please enter a number between 0 and 7 or 'all'.")
                show_menu()
                user_choice = get_user_choice()
    else:
        show_menu()
        user_choice = get_user_choice()
    if user_choice == 0:
        for i in range(7, 0, -1):
            perform_selection(self,i)
            perform_action(self)
            cp_result(self)
        return
    else:
        perform_selection(self, user_choice)
        perform_action(self)
        cp_result(self)

if __name__ == "__main__":
    main()
