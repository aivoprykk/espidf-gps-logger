Import("env")
import os

def build_ulp(source, target, env):
    # Get ULP source path
    ulp_source = "components/logger_adc/ulp/adc.S"

    # Build ULP using esp-idf tools
    ulp_tool = os.path.join(env.PioPlatform().get_package_dir("framework-espidf"), "tools", "ulp")
    
    cmd = [
        "python",
        os.path.join(ulp_tool, "esp32ulp_mapgen.py"),
        "-s", ulp_source,
        "-o", os.path.join(env.subst("$BUILD_DIR"))
    ]
    
    env.Execute(" ".join(cmd))