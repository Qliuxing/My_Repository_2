Profile include path:

    Application Makefile
        include Config.mk                                           # Common configuration for the platform
            include $(PRODUCT_DIR)/config/Extra.mk                  # Common configuration for the PRODUCT
                include $(PRODUCT_DIR)/config/$(PROFILE_NAME).mk    # PROFILE specific configuration
        include Rules.mk         
