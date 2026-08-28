OUTDIR := out
TITLE_ID := 4200000000000667
SD_ROOT := $(OUTDIR)/sd
TITLE_DIR := $(SD_ROOT)/atmosphere/contents/$(TITLE_ID)

LSP_BSD_MITM_VERSION := 0.1.0
NSP_OUT := lsp_bsd_mitm/out/nintendo_nx_arm64_armv8a/release/lsp_bsd_mitm.nsp
NPDM_OUT := lsp_bsd_mitm/out/nintendo_nx_arm64_armv8a/release/lsp_bsd_mitm.npdm

all: dist

build:
	$(MAKE) -C Atmosphere-libs/libstratosphere nx_release
	$(MAKE) -C lsp_bsd_mitm all

pack: build
	@mkdir -p $(TITLE_DIR)/flags
	@cp $(NSP_OUT) $(TITLE_DIR)/exefs.nsp
	@cp $(NPDM_OUT) $(TITLE_DIR)/main.npdm
	@touch $(TITLE_DIR)/flags/boot2.flag
	@rm -f $(TITLE_DIR)/mitm.lst
	@echo "bsd:u" >> $(TITLE_DIR)/mitm.lst

dist: pack
	@cd $(SD_ROOT); zip -r ../lsp_bsd_mitm-$(LSP_BSD_MITM_VERSION).zip ./* > /dev/null
	@echo "Wrote out/lsp_bsd_mitm-$(LSP_BSD_MITM_VERSION).zip"

clean:
	$(MAKE) -C Atmosphere-libs/libstratosphere clean
	$(MAKE) -C lsp_bsd_mitm clean
	@rm -rf $(OUTDIR)

.PHONY: all build pack dist clean
