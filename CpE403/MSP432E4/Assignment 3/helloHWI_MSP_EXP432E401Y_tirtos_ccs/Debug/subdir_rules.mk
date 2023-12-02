################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs" --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/Debug" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/ti/posix/ccs" --include_path="C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=none -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/Debug/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-591086753:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-591086753-inproc

build-591086753-inproc: ../helloHWI.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/ccs1240/xdctools_3_61_02_27_core/xs" --xdcpath="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source;C:/ti/simplelink_msp432e4_sdk_4_20_00_12/kernel/tirtos/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.msp432:MSP432E401Y -r release -c "C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path=\"C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs\" --include_path=\"C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/Debug\" --include_path=\"C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source\" --include_path=\"C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/third_party/CMSIS/Include\" --include_path=\"C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/ti/posix/ccs\" --include_path=\"C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include\" --advice:power=none -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-591086753 ../helloHWI.cfg
configPkg/compiler.opt: build-591086753
configPkg/: build-591086753

build-704118417: ../helloHWI.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.4.0/sysconfig_cli.bat" -s "C:/ti/simplelink_msp432e4_sdk_4_20_00_12/.metadata/product.json" -o "syscfg" "C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/helloHWI.syscfg" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_drivers_config.c: build-704118417 ../helloHWI.syscfg
syscfg/ti_drivers_config.h: build-704118417
syscfg/ti_utils_build_linker.cmd.exp: build-704118417
syscfg/syscfg_c.rov.xs: build-704118417
syscfg/: build-704118417

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs" --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/Debug" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432e4_sdk_4_20_00_12/source/ti/posix/ccs" --include_path="C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=none -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/Users/samue/ccs_workspace_v12/helloHWI_MSP_EXP432E401Y_tirtos_ccs/Debug/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


