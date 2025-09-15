include $(TOPDIR)/rules.mk

PKG_NAME:=flash-fox
PKG_RELEASE:=1
PKG_LICENSE:=MIT

# Если исходники локально рядом со SDK:
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=libusb libxml2 zlib
# или если из git/тарбола — добавь PKG_SOURCE, PKG_SOURCE_URL и т.д.

include $(INCLUDE_DIR)/package.mk

define Package/flash-fox
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=flash_fox utility
  DEPENDS:=+libusb-1.0 +libxml2 +zlib
endef

define Package/flash-fox/description
USB utility using libusb-1.0 and libxml2.
endef

# Если у тебя есть собственный Makefile (как в сообщении) — используем его.
# Важно передать правильные CC/CFLAGS/LDFLAGS и pkg-config из STAGING_DIR.

define Build/Prepare
	$(call Build/Prepare/Default)
	# Скопируем твой проект в $(PKG_BUILD_DIR)
	# Предположим, что рядом с этим Makefile лежит каталог src/
	$(CP) ./src $(PKG_BUILD_DIR)/
	$(CP) ./Makefile.app $(PKG_BUILD_DIR)/Makefile
endef

# где Makefile.app — это твой оригинальный Makefile из вопроса (можно назвать просто Makefile, если не конфликтует)

define Build/Compile
	$$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		        $$($(STAGING_DIR_HOSTPKG)/bin/pkg-config --cflags libusb-1.0 libxml-2.0) \
		        -I$(STAGING_DIR)/usr/include/libusb-1.0" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LDLIBS="$$($(STAGING_DIR_HOSTPKG)/bin/pkg-config --libs libusb-1.0 libxml-2.0)" \
		PKG_CONFIG="$(STAGING_DIR_HOSTPKG)/bin/pkg-config" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig:$(STAGING_DIR)/usr/share/pkgconfig" \
		PKG_CONFIG_SYSROOT_DIR="$(STAGING_DIR)"
endef

define Package/flash-fox/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flash_fox $(1)/usr/bin/flash_fox
endef

$(eval $(call BuildPackage,flash-fox))

