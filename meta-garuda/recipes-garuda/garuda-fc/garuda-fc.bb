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
SRC_URI += "file://launch-drone.sh \
            file://garuda.conf"

# 5. Build Configuration
DEPENDS += "boost"

EXTRA_OECMAKE = "-DBUILD_DASHBOARD=OFF \
                 -DBUILD_SIMULATOR=ON \
                 -DBUILD_TESTS=OFF \
                 -DBUILD_DOCS=OFF"

# 6. Installation Logic
do_install() {
    # Install the binary
    install -d ${D}${bindir}
    install -m 0755 ${B}/Simulator/drone_sim ${D}${bindir}

    # Install the startup script
    install -m 0755 ${WORKDIR}/launch-drone.sh ${D}${bindir}/launch-drone

    # Install the config file to /etc/garuda/
    install -d ${D}${sysconfdir}/garuda
    install -m 0644 ${WORKDIR}/garuda.conf ${D}${sysconfdir}/garuda/garuda.conf
}

RDEPENDS:${PN} = "libstdc++ boost-system"

CONFFILES:${PN} = "${sysconfdir}/garuda/garuda.conf"
