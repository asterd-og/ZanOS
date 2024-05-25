# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

override IMAGE_NAME := ZanOS

# Change those in your machine!
# QEMU := ./../build/qemu-system-x86_64
# QARGS := -L /usr/share/seabios/ -L /usr/share/qemu -L /usr/lib/ipxe/qemu/
QEMU := qemu-system-x86_64

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

# Toolchain for building the 'limine' executable for the host.
override DEFAULT_HOST_CC := cc
$(eval $(call DEFAULT_VAR,HOST_CC,$(DEFAULT_HOST_CC)))
override DEFAULT_HOST_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,HOST_CFLAGS,$(DEFAULT_HOST_CFLAGS)))
override DEFAULT_HOST_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,HOST_CPPFLAGS,$(DEFAULT_HOST_CPPFLAGS)))
override DEFAULT_HOST_LDFLAGS :=
$(eval $(call DEFAULT_VAR,HOST_LDFLAGS,$(DEFAULT_HOST_LDFLAGS)))
override DEFAULT_HOST_LIBS :=
$(eval $(call DEFAULT_VAR,HOST_LIBS,$(DEFAULT_HOST_LIBS)))

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: run-kvm

all-vbox: $(IMAGE_NAME).iso
	cp ZanOS.iso /mnt/c/Users/astrido/Documents/

.PHONY: run-normal
run-normal: $(IMAGE_NAME).iso
	$(QEMU) $(QARGS) -debugcon stdio -m 2G -cdrom $(IMAGE_NAME).iso -boot d -smp 2 -drive file="disk.img",format=raw -no-reboot -no-shutdown

.PHONY: run-kvm
run-kvm: $(IMAGE_NAME).iso
	qemu-system-x86_64 -debugcon stdio \
	-m 2G \
	-cdrom $(IMAGE_NAME).iso \
	-boot d \
	-enable-kvm \
	-smp 2 \
	-drive id=disk,file=disk.img,if=none \
	-device ahci,id=ahci \
	-device ide-hd,drive=disk,bus=ahci.0 \
	-no-reboot -no-shutdown

run-ata: $(IMAGE_NAME).iso
	qemu-system-x86_64 -debugcon stdio \
	-m 2G \
	-cdrom $(IMAGE_NAME).iso \
	-boot d \
	-enable-kvm \
	-smp 2 \
	-drive file="disk.img",format=raw \
	-no-reboot -no-shutdown

.PHONY: run-kvm-rtl8139
run-kvm-rtl8139: $(IMAGE_NAME).iso
	$(QEMU) $(QARGS) -M q35 -m 2G -cdrom $(IMAGE_NAME).iso -boot d -serial stdio -accel kvm -smp $(shell nproc) -drive file="fat.img",format=raw,index=0,media=disk -netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=net0,mac=52:54:00:12:34:55

.PHONY: run-uefi
run-uefi: ovmf $(IMAGE_NAME).iso
	$(QEMU) $(QARGS) -M q35 -m 2G -bios /usr/share/ovmf/OVMF.fd -boot d -serial stdio -enable-kvm -smp $(shell nproc) -device piix3-ide,id=ide -drive id=disk,file="fat.img",format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 \
        -drive file=$(IMAGE_NAME).iso,format=raw,if=none,id=cdrom -device ide-cd,bus=ide.1,drive=cdrom -M q35

.PHONY: run-hdd
run-hdd: $(IMAGE_NAME).hdd
	$(QEMU) $(QARGS) -M q35 -m 2G -hda $(IMAGE_NAME).hdd

.PHONY: run-hdd-uefi
run-hdd-uefi: ovmf $(IMAGE_NAME).hdd
	$(QEMU) $(QARGS) -M q35 -m 2G -bios ovmf/OVMF.fd -hda $(IMAGE_NAME).hdd

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v6.x-branch-binary --depth=1
	$(MAKE) -C limine \
		CC="$(HOST_CC)" \
		CFLAGS="$(HOST_CFLAGS)" \
		CPPFLAGS="$(HOST_CPPFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		LIBS="$(HOST_LIBS)"

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

fs:
	sudo mount disk.img /mnt
	-sudo rm /mnt/bin/shell
	sudo cp ../appsZan/shell/shell /mnt/bin/
	sudo umount disk.img

$(IMAGE_NAME).iso: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp -v kernel/bin/kernel \
		limine.cfg tmpfs limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	./limine/limine bios-install $(IMAGE_NAME).iso
	rm -rf iso_root

$(IMAGE_NAME).hdd: limine kernel
	rm -f $(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	sgdisk $(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
	./limine/limine bios-install $(IMAGE_NAME).hdd
	mformat -i $(IMAGE_NAME).hdd@@1M
	mmd -i $(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M kernel/bin/kernel limine.cfg limine/limine-bios.sys ::/
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd
	$(MAKE) -C kernel clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf
	$(MAKE) -C kernel distclean
