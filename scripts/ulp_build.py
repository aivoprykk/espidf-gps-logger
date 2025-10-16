Import("env")
import os

# .platformio/packages/framework-espidf/components/ulp/esp32ulp_mapgen.py

# Output file PlatformIO expects: an assembly source file named "ulp_battery.bin.S"
ulp_bin_s = "ulp_battery.bin.S"
ulp_src = "components/logger_adc/ulp/adc.S"

def build_ulp_0(source, target, env):
    ulp_src = os.path.join(env.subst("$PROJECT_DIR"), ulp_src)
    # Ensure we produce an assembly source file in the build dir that PlatformIO's build rules expect
    ulp_s_path = os.path.join(env.subst("$BUILD_DIR"), ulp_bin_s)
    ulp_tool = os.path.join(env.PioPlatform().get_package_dir("framework-espidf"), "components", "ulp", "esp32ulp_mapgen.py")
    cmd = [ "python", ulp_tool, "-s", ulp_src, "-o", ulp_s_path ]
    print(f"Building ULP assembly source: {cmd}")
    env.Execute(" ".join(cmd))

env.AddPreAction("build", build_ulp_0)