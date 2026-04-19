SUMMARY = "Project Garuda Flight Controller"
SECTION = "apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# 1. Inherit the magic class
inherit cmake externalsrc

# 2. This tells BitBake to look in the 'files' subfolder for launch-drone.sh
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# 3. Point to your local root directory
# This finds the directory two levels up from the .bb file
# (Going from garuda-fc/ to recipes-garuda/ to meta-garuda/ to UAV_System/)
EXTERNALSRC = "${THISDIR}/../../.."

# 4. Include the helper script from the recipe's 'files' directory
SRC_URI += "file://launch-drone.sh"

# 5. Build Configuration
DEPENDS += "boost"

EXTRA_OECMAKE = "-DBUILD_DASHBOARD=OFF \
                 -DBUILD_SIMULATOR=ON \
                 -DBUILD_TESTS=OFF \
                 -DBUILD_DOCS=OFF"

# 6. Installation Logic
do_install() {
    # 1. Create the /usr/bin directory in the virtual image
    install -d ${D}${bindir}

    # 2. Install the binary using the OUTPUT_NAME from your CMakeLists.txt 
    # We look in ${B} (the build directory) under the Simulator subfolder
    install -m 0755 ${B}/Simulator/drone_sim ${D}${bindir}

    # 3. Install the startup script as 'launch-drone'
    # This script should contain: drone_sim 10.0.2.2
    install -m 0755 ${WORKDIR}/launch-drone.sh ${D}${bindir}/launch-drone
}

RDEPENDS:${PN} = "libstdc++ boost-system"
