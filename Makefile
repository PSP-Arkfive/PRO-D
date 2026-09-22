REBOOTEXBIN = Rebootex_bin
REBOOTEX = Rebootex
VSHCONTROL = Vshctrl
SYSTEMCONTROL = SystemControl
GALAXYDRIVER = ISODrivers/Galaxy
M33DRIVER = ISODrivers/March33
INFERNO = ISODrivers/Inferno
STARGATE = Stargate
ISOLAUNCHER = testsuite/ISOLauncher
FASTRECOVERY = FastRecovery
SATELITE = Satelite
POPCORN = Popcorn
RECOVERY = Recovery
CROSSFW = CrossFW
DISTRIBUTE = dist
OPT_FLAGS=-j4

ifeq ($(CONFIG_635), 1)
OPT_FLAGS+=CONFIG_635=1
endif

ifeq ($(CONFIG_620), 1)
OPT_FLAGS+=CONFIG_620=1
endif

ifeq ($(CONFIG_639), 1)
OPT_FLAGS+=CONFIG_639=1
endif

ifeq ($(CONFIG_660), 1)
OPT_FLAGS+=CONFIG_660=1
endif

ifeq ($(CONFIG_661), 1)
OPT_FLAGS+=CONFIG_661=1
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

all:
# Preparing Distribution Folders
	@mkdir $(DISTRIBUTE) || true
	@mkdir $(DISTRIBUTE)/seplugins/ || true
	@cp -r contrib/fonts $(DISTRIBUTE)/seplugins/fonts || true
	@cp Translated/* $(DISTRIBUTE)/seplugins || true
	@mkdir $(DISTRIBUTE)/PSP || true
	@mkdir $(DISTRIBUTE)/PSP/GAME || true
	@mkdir $(DISTRIBUTE)/PSP/GAME/PROUPDATE || true
	@mkdir $(DISTRIBUTE)/PSP/GAME/FastRecovery || true
ifeq ($(CONFIG_620), 1)
	@mkdir $(DISTRIBUTE)/PSP/GAME/620PRO_Permanent || true
endif
ifeq ($(CONFIG_639), 1)
	@mkdir $(DISTRIBUTE)/PSP/GAME/CIPL_Flasher || true
endif
ifeq ($(CONFIG_660), 1)
	@mkdir $(DISTRIBUTE)/PSP/GAME/CIPL_Flasher || true
endif
ifeq ($(CONFIG_661), 1)
	@mkdir $(DISTRIBUTE)/PSP/GAME/CIPL_Flasher || true
endif
	@rm -f ./Common/*.o

# Creating CrossFW library
	@cd $(CROSSFW); make $(OPT_FLAGS) $(DEBUG_OPTION)

# Creating Live-System Reboot Buffer
	@cd $(REBOOTEXBIN); make $(OPT_FLAGS)
	@cd $(REBOOTEX); make $(OPT_FLAGS) $(DEBUG_OPTION)

# Creating Live-System Components
	@cd $(RECOVERY); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@rm -f ./Common/*.o
	@cd $(VSHCONTROL); make $(OPT_FLAGS) $(DEBUG_OPTION) $(NIGHTLY_OPTION)
	@cd $(USBDEVICE); make $(OPT_FLAGS) $(DEBUG_OPTION) $(NIGHTLY_OPTION)
	@cd $(SYSTEMCONTROL); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(GALAXYDRIVER); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(INFERNO); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(STARGATE); make $(OPT_FLAGS) $(DEBUG_OPTION) $(RELEASE_OPTION)
	@cd $(SATELITE); make $(OPT_FLAGS) $(DEBUG_OPTION)
	@cd $(POPCORN); make $(OPT_FLAGS) $(DEBUG_OPTION)

clean:
	@cd $(REBOOTEXBIN); make clean $(DEBUG_OPTION)
	@cd $(CROSSFW); make clean $(DEBUG_OPTION)
	@cd $(REBOOTEX); make clean $(DEBUG_OPTION)
	@cd $(INSTALLER); make clean $(DEBUG_OPTION)
	@cd $(VSHCONTROL); make clean $(DEBUG_OPTION)
	@cd $(USBDEVICE); make clean $(DEBUG_OPTION)
	@cd $(SYSTEMCONTROL); make clean $(DEBUG_OPTION)
	@cd $(GALAXYDRIVER); make clean $(DEBUG_OPTION)
	@cd $(INFERNO); make clean $(DEBUG_OPTION)
	@cd $(STARGATE); make clean $(DEBUG_OPTION) $(RELEASE_OPTION)
	@cd $(SATELITE); make clean $(DEBUG_OPTION)
	@cd $(LAUNCHER); make clean $(DEBUG_OPTION)
	@cd $(REBOOTEXPXE); make clean $(DEBUG_OPTION)
	@cd $(SYSTEMCONTROLPXE); make clean $(DEBUG_OPTION)
	@cd $(POPCORN); make clean $(DEBUG_OPTION)
	@cd $(RECOVERY); make clean $(DEBUG_OPTION)
	@rm -rf $(DISTRIBUTE)

deps:
	make clean_lib
	make build_lib

build_lib:
	@cd $(SYSTEMCONTROL)/libs; make $(OPT_FLAGS) $(DEBUG_OPTION)
	
clean_lib:
	@cd $(SYSTEMCONTROL)/libs; make clean $(DEBUG_OPTION)
