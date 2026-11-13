PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

REBOOTEXBIN = Rebootex_bin
VSHCONTROL = Vshctrl
SYSTEMCONTROL = SystemControl
GALAXYDRIVER = ISODrivers/Galaxy
M33DRIVER = ISODrivers/March33
INFERNO = ISODrivers/Inferno
STARGATE = Stargate
SATELITE = Satelite
POPCORN = Popcorn
RECOVERY = Recovery
CROSSFW = CrossFW
INSTALLER = Installer
DISTRIBUTE = dist
LIBS_DIR = .usbdevice-build
LIBS_ARCHIVE = $(LIBS_DIR)/LIBS.zip
LIBS_URL = https://github.com/pspdev/psp-dynamic-libraries/releases/download/latest/LIBS.zip
OPT_FLAGS=-j4

ifeq ($(CONFIG_635), 1)
OPT_FLAGS+=CONFIG_635=1
FLASH_PACKAGE = FLASH635.PRO
M33_PRX = march33.prx
endif

ifeq ($(CONFIG_620), 1)
OPT_FLAGS+=CONFIG_620=1
FLASH_PACKAGE = FLASH620.PRO
M33_PRX = march33_620.prx
endif

ifeq ($(CONFIG_639), 1)
OPT_FLAGS+=CONFIG_639=1
FLASH_PACKAGE = FLASH639.PRO
M33_PRX = march33.prx
endif

ifeq ($(CONFIG_660), 1)
OPT_FLAGS+=CONFIG_660=1
FLASH_PACKAGE = FLASH660.PRO
M33_PRX = march33_660.prx
endif

ifeq ($(CONFIG_661), 1)
OPT_FLAGS+=CONFIG_661=1
FLASH_PACKAGE = FLASH661.PRO
M33_PRX = march33_660.prx
endif

ifeq ($(PSID_CHECK), 1)
RELEASE_OPTION=PSID_CHECK=1
endif

ifeq ($(DEBUG), 1)
DEBUG_OPTION=DEBUG=1
endif

ifeq ($(NIGHTLY), 1)
NIGHTLY_OPTION=NIGHTLY=1
endif

all: libs_latest
# Preparing Distribution Folders
	@mkdir -p $(DISTRIBUTE)/flash0/kd || true
	@mkdir -p $(DISTRIBUTE)/flash0/vsh/module || true
	@mkdir -p $(DISTRIBUTE)/seplugins/ || true
	@mkdir -p $(DISTRIBUTE)/PSP/GAME/PROINSTALLER || true
	@cp -r contrib/fonts $(DISTRIBUTE)/seplugins/fonts || true
	@cp Translated/* $(DISTRIBUTE)/seplugins || true
	@rm -f ./Common/*.o

# Creating CrossFW library
	@cd $(CROSSFW); make $(OPT_FLAGS) $(DEBUG_OPTION)

# Creating Live-System Reboot Buffer
	@cd $(REBOOTEXBIN); make $(OPT_FLAGS)

# Creating Live-System Components
	@cd $(RECOVERY); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@rm -f ./Common/*.o
	@cd $(VSHCONTROL); make $(OPT_FLAGS) $(DEBUG_OPTION) $(NIGHTLY_OPTION)
	@cd $(SYSTEMCONTROL); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(GALAXYDRIVER); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(INFERNO); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(STARGATE); make $(OPT_FLAGS) $(DEBUG_OPTION) $(RELEASE_OPTION)
	@cd $(POPCORN); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(SATELITE); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(INSTALLER); make $(OPT_FLAGS) $(DEBUG_OPTION)

# Copy binaries
	@cp $(RECOVERY)/recovery.prx dist/flash0/vsh/module/_recovery.prx
	@cp $(VSHCONTROL)/vshctrl.prx dist/flash0/kd/_vshctrl.prx
	@cp $(SYSTEMCONTROL)/systemctrl.prx dist/flash0/kd/_systemctrl.prx
	@cp $(GALAXYDRIVER)/galaxy.prx dist/flash0/kd/_galaxy.prx
	@cp $(INFERNO)/inferno.prx dist/flash0/kd/_inferno.prx
	@cp $(STARGATE)/stargate.prx dist/flash0/kd/_stargate.prx
	@cp $(POPCORN)/popcorn.prx dist/flash0/kd/_popcorn.prx
	@cp $(SATELITE)/satelite.prx dist/flash0/vsh/module/_satelite.prx
	@cp $(M33DRIVER)/$(M33_PRX) dist/flash0/kd/_march33.prx
	@rm -f $(DISTRIBUTE)/PSP/GAME/PROINSTALLER/usbdevice.prx
	cd $(LIBS_DIR) && $(PY) $(BUILDTOOLS)/gz/pspgz.py $(abspath $(DISTRIBUTE)/PSP/GAME/PROINSTALLER/usbdevice.prx) $(BUILDTOOLS)/gz/SystemControl.hdr usbdevice.prx pspUsbDev_Driver 0x1006
	@test -s $(DISTRIBUTE)/PSP/GAME/PROINSTALLER/usbdevice.prx
	$(PY) $(BUILDTOOLS)/pack/pack.py -p dist/$(FLASH_PACKAGE) flash0.txt -s
	@cp $(INSTALLER)/EBOOT.PBP $(DISTRIBUTE)/PSP/GAME/PROINSTALLER/ || true
	@cp dist/$(FLASH_PACKAGE) $(DISTRIBUTE)/PSP/GAME/PROINSTALLER/ || true

libs_latest:
	mkdir -p $(LIBS_DIR)
	curl --fail --location --silent --show-error "$(LIBS_URL)" -o "$(LIBS_ARCHIVE)"
	unzip -o "$(LIBS_ARCHIVE)" usbdevice.prx -d "$(LIBS_DIR)"

clean:
	@rm -rf $(DISTRIBUTE)
	@rm -rf $(LIBS_DIR)
	@cd $(REBOOTEXBIN); make clean $(DEBUG_OPTION)
	@cd $(CROSSFW); make clean $(DEBUG_OPTION)
	@cd $(VSHCONTROL); make clean $(DEBUG_OPTION)
	@cd $(SYSTEMCONTROL); make clean $(DEBUG_OPTION)
	@cd $(GALAXYDRIVER); make clean $(DEBUG_OPTION)
	@cd $(INFERNO); make clean $(DEBUG_OPTION)
	@cd $(STARGATE); make clean $(DEBUG_OPTION) $(RELEASE_OPTION)
	@cd $(SATELITE); make clean $(DEBUG_OPTION)
	@cd $(POPCORN); make clean $(DEBUG_OPTION)
	@cd $(RECOVERY); make clean $(DEBUG_OPTION)
	@cd $(INSTALLER); make clean $(DEBUG_OPTION)

deps:
	make clean_lib
	make build_lib

build_lib:
	@cd $(SYSTEMCONTROL)/libs; make $(OPT_FLAGS) $(DEBUG_OPTION)
	
clean_lib:
	@cd $(SYSTEMCONTROL)/libs; make clean $(DEBUG_OPTION)

.PHONY: libs_latest
