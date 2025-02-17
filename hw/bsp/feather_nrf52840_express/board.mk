CFLAGS += \
	-mthumb \
	-mabi=aapcs \
	-mcpu=cortex-m4 \
	-mfloat-abi=hard \
	-mfpu=fpv4-sp-d16 \
	-DCFG_TUSB_MCU=OPT_MCU_NRF5X \
	-DNRF52840_XXAA \
  -DCONFIG_GPIO_AS_PINRESET

# nrfx issue undef _ARMCC_VERSION usage https://github.com/NordicSemiconductor/nrfx/issues/49
CFLAGS += -Wno-error=undef 

# All source paths should be relative to the top level.
LD_FILE = hw/bsp/feather_nrf52840_express/nrf52840_s140_v6.ld

LDFLAGS += -L$(TOP)/hw/mcu/nordic/nrfx/mdk

SRC_C += \
	hw/mcu/nordic/nrfx/drivers/src/nrfx_power.c \
	hw/mcu/nordic/nrfx/mdk/system_nrf52840.c \

# TODO remove later
SRC_C += src/portable/$(VENDOR)/$(CHIP_FAMILY)/hal_$(CHIP_FAMILY).c

INC += \
	$(TOP)/hw/cmsis/Include \
	$(TOP)/hw/mcu/nordic \
	$(TOP)/hw/mcu/nordic/nrfx \
	$(TOP)/hw/mcu/nordic/nrfx/mdk \
	$(TOP)/hw/mcu/nordic/nrfx/hal \
	$(TOP)/hw/mcu/nordic/nrfx/drivers/include \
	$(TOP)/hw/mcu/nordic/nrfx/drivers/src \

SRC_S += hw/mcu/nordic/nrfx/mdk/gcc_startup_nrf52840.S

ASFLAGS += -D__HEAP_SIZE=0

# For TinyUSB port source
VENDOR = nordic
CHIP_FAMILY = nrf5x

# For freeRTOS port source
FREERTOS_PORT = ARM_CM4F

# For flash-jlink target
JLINK_DEVICE = nRF52840_xxAA
JLINK_IF = swd

# For uf2 conversion
UF2_FAMILY = 0xADA52840

# flash using jlink
flash: flash-jlink
