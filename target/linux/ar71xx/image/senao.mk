define Build/senao-factory-image
	$(eval board=$(word 1,$(1)))
	$(eval rootfs=$(word 2,$(1)))

	mkdir -p $@.senao

	touch $@.senao/FWINFO-OpenWrt-$(REVISION)-$(board)
	$(CP) $(IMAGE_KERNEL) $@.senao/openwrt-senao-$(board)-uImage-lzma.bin
	$(CP) $(rootfs) $@.senao/openwrt-senao-$(board)-root.squashfs

	$(TAR) -c \
		--numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-C $@.senao . | gzip -9nc > $@

	rm -rf $@.senao
endef


define Device/ens202ext
  DEVICE_TITLE := EnGenius ENS202EXT
  BOARDNAME := ENS202EXT
  DEVICE_PACKAGES := rssileds
  KERNEL_SIZE := 1536k
  IMAGE_SIZE := 13632k
  IMAGES += factory.bin
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),320k(custom)ro,1536k(kernel),12096k(rootfs),2048k(failsafe)ro,64k(art)ro,13632k@0xa0000(firmware)
  IMAGE/factory.bin/squashfs := append-rootfs | pad-rootfs | senao-factory-image ens202ext $$$$@
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ens202ext

define Device/emr3000
  DEVICE_TITLE := EnGenius EMR3000
  BOARDNAME := EMR3000
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-bluetooth kmod-ath10k ath10k-firmware-qca988x
  KERNEL_SIZE := 1408k
  IMAGE_SIZE := 32064k
  IMAGES += factory.bin
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(art)ro,1408k(kernel),30656k(rootfs),320k(config)ro,32064k@0x60000(firmware)
  IMAGE/factory.bin/squashfs := append-rootfs | pad-rootfs | senao-factory-image emr3000 $$$$@
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += emr3000
