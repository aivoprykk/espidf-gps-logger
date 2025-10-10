Import("env")
import os

ulp_bin = "ulp_battery.bin"
ulp_src = "components/logger_adc/ulp/adc.S"

def build_ulp_0(source, target, env):
    ulp_src = os.path.join(env.subst("$PROJECT_DIR"), ulp_src)
    ulp_bin_path = os.path.join(env.subst("$BUILD_DIR"), ulp_bin)
    ulp_tool = os.path.join(env.PioPlatform().get_package_dir("framework-espidf"), "tools", "ulp", "esp32ulp_mapgen.py")
    cmd = [ "python", ulp_tool, "-s", ulp_src, "-o", ulp_bin_path ]
    print(f"Building ULP binary: {cmd}")
    env.Execute(" ".join(cmd))


# Compile ULP assembly files using ulptool
def build_ulp(source, target, env):
    ulp_src = os.path.join(env.subst("$PROJECT_DIR"), ulp_src)
    ulp_bin_path = os.path.join(env.subst("$BUILD_DIR"), ulp_bin)
    cmd = [ "ulptool", "compile", ulp_src, "-o", ulp_bin_path ]
    print(f"Building ULP binary: {cmd}")
    os.system(" ".join(cmd))

env.AddPreAction("build", build_ulp_0)